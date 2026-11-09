// Simple heater/thermistor physics for the virtual mcu
//
// Each "thermal zone" pairs a heater output pin with a thermistor adc
// pin. The heater's duty cycle is measured from its pin transitions and
// fed into a first order thermal model:
//
//   dT/dt = (T_eq - T) / tau,  T_eq = ambient + duty * (max_temp - ambient)
//
// which is advanced lazily (exact exponential solution) whenever the
// thermistor is sampled, so it costs nothing while idle.
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <math.h> // exp
#include <stdio.h> // fprintf
#include <stdlib.h> // rand_r
#include <string.h> // strchr
#include "autoconf.h" // CONFIG_CLOCK_FREQ
#include "board/misc.h" // timer_read_time
#include "virtual.h" // struct virtual_pin

#define AMBIENT_TEMP 25.
#define KELVIN_TO_CELSIUS -273.15
#define PULLUP_RESISTOR 4700.

struct thermal_preset {
    const char *name;
    double max_temp; // steady state temperature at 100% duty
    double tau;      // time constant in seconds
};

static const struct thermal_preset presets[] = {
    // ~200C after 40s at full power, ~300C maximum
    { "hotend", 300., 40. },
    // ~60C after 150s at full power, ~130C maximum
    { "bed", 130., 370. },
};

struct thermal_zone {
    struct virtual_pin *heater, *sensor;
    const struct thermal_preset *preset;
    double temp;
    uint32_t last_update, last_change, on_ticks;
    int started;
};

#define MAX_ZONES 8
static struct thermal_zone zones[MAX_ZONES];
static int zone_count;

// Default zones matching the shipped printer configs
static const char *default_zones[] = {
    "PD7:PA5:hotend",   // extruder
    "PD3:PA6:hotend",   // extruder1
    "PB4:PA7:bed",      // heater_bed
};


/****************************************************************
 * Thermistor (EPCOS 100K B57560G104F with a 4.7K pull-up)
 ****************************************************************/

static double sh_c1, sh_c2, sh_c3;

// Steinhart-Hart coefficients from three calibration points - same
// calculation as klippy/extras/thermistor.py
static void
thermistor_setup(double t1, double r1, double t2, double r2
                 , double t3, double r3)
{
    double inv_t1 = 1. / (t1 - KELVIN_TO_CELSIUS);
    double inv_t2 = 1. / (t2 - KELVIN_TO_CELSIUS);
    double inv_t3 = 1. / (t3 - KELVIN_TO_CELSIUS);
    double ln_r1 = log(r1), ln_r2 = log(r2), ln_r3 = log(r3);
    double ln3_r1 = ln_r1*ln_r1*ln_r1, ln3_r2 = ln_r2*ln_r2*ln_r2;
    double ln3_r3 = ln_r3*ln_r3*ln_r3;
    double inv_t12 = inv_t1 - inv_t2, inv_t13 = inv_t1 - inv_t3;
    double ln_r12 = ln_r1 - ln_r2, ln_r13 = ln_r1 - ln_r3;
    double ln3_r12 = ln3_r1 - ln3_r2, ln3_r13 = ln3_r1 - ln3_r3;
    sh_c3 = ((inv_t12 - inv_t13 * ln_r12 / ln_r13)
             / (ln3_r12 - ln3_r13 * ln_r12 / ln_r13));
    sh_c2 = (inv_t12 - sh_c3 * ln3_r12) / ln_r12;
    sh_c1 = inv_t1 - sh_c2 * ln_r1 - sh_c3 * ln3_r1;
}

// Fraction of full scale the adc reads at the given temperature
static double
thermistor_adc(double temp)
{
    double inv_t = 1. / (temp - KELVIN_TO_CELSIUS);
    // Solve for ln(R) using Cardano's formula
    double y = (sh_c1 - inv_t) / (2. * sh_c3);
    double x = sqrt(pow(sh_c2 / (3. * sh_c3), 3.) + y*y);
    double ln_r = cbrt(x - y) - cbrt(x + y);
    double r = exp(ln_r);
    return r / (PULLUP_RESISTOR + r);
}


/****************************************************************
 * Thermal model
 ****************************************************************/

static void
zone_start(struct thermal_zone *z, uint32_t now)
{
    z->started = 1;
    z->last_update = z->last_change = now;
    z->on_ticks = 0;
}

// Advance the model to 'now' using the heater duty since the last update
static void
zone_update(struct thermal_zone *z, uint32_t now)
{
    if (!z->started) {
        zone_start(z, now);
        return;
    }
    uint32_t total = now - z->last_update;
    if (total < CONFIG_CLOCK_FREQ / 1000)
        return;
    uint32_t on = z->on_ticks;
    if (z->heater->state) {
        on += now - z->last_change;
        z->last_change = now;
    }
    double duty = (double)on / total;
    if (duty > 1.)
        duty = 1.;
    double dt = (double)total / CONFIG_CLOCK_FREQ;
    const struct thermal_preset *p = z->preset;
    double t_eq = AMBIENT_TEMP + duty * (p->max_temp - AMBIENT_TEMP);
    z->temp = t_eq + (z->temp - t_eq) * exp(-dt / p->tau);
    z->last_update = now;
    z->on_ticks = 0;
}

void
thermal_heater_changed(struct thermal_zone *z, uint8_t old_state
                       , uint32_t now)
{
    if (!z->started) {
        zone_start(z, now);
        return;
    }
    if (old_state)
        z->on_ticks += now - z->last_change;
    z->last_change = now;
}

uint16_t
thermal_adc_read(struct thermal_zone *z, uint16_t adc_max)
{
    zone_update(z, timer_read_time());
    // Add +/- 0.5 lsb of dither so oversampled readings resolve
    // temperatures finer than one adc step
    static unsigned int seed = 1;
    double dither = (double)rand_r(&seed) / RAND_MAX - .5;
    double val = thermistor_adc(z->temp) * adc_max + dither;
    if (val < 0.)
        return 0;
    if (val > adc_max)
        return adc_max;
    return (uint16_t)(val + .5);
}


/****************************************************************
 * Setup
 ****************************************************************/

// Parse "HEATER_PIN:SENSOR_PIN:PRESET" (eg, "PD7:PA5:hotend")
int
thermal_add(const char *spec)
{
    char buf[64], *heater = buf, *sensor, *preset;
    if (strlen(spec) >= sizeof(buf))
        goto fail;
    strcpy(buf, spec);
    sensor = strchr(heater, ':');
    if (!sensor)
        goto fail;
    *sensor++ = '\0';
    preset = strchr(sensor, ':');
    if (!preset)
        goto fail;
    *preset++ = '\0';

    uint32_t hpin, spin;
    if (virtual_pin_parse(heater, &hpin) || virtual_pin_parse(sensor, &spin))
        goto fail;
    const struct thermal_preset *p = NULL;
    int i;
    for (i = 0; i < sizeof(presets) / sizeof(presets[0]); i++)
        if (strcmp(presets[i].name, preset) == 0)
            p = &presets[i];
    if (!p || zone_count >= MAX_ZONES)
        goto fail;
    struct virtual_pin *hp = virtual_pin_get(hpin);
    struct virtual_pin *sp = virtual_pin_get(spin);
    if (hp->heats || sp->senses)
        goto fail;

    struct thermal_zone *z = &zones[zone_count++];
    z->heater = hp;
    z->sensor = sp;
    z->preset = p;
    z->temp = AMBIENT_TEMP;
    hp->heats = z;
    sp->senses = z;
    if (!sh_c1)
        thermistor_setup(25., 100000., 150., 1641.9, 250., 226.15);
    fprintf(stderr, "Thermal zone: heater=%s sensor=%s preset=%s\n"
            , heater, sensor, preset);
    return 0;
fail:
    fprintf(stderr, "Invalid thermal zone '%s' (expected"
            " HEATER_PIN:SENSOR_PIN:hotend|bed, eg PD7:PA5:hotend)\n", spec);
    return -1;
}

void
thermal_add_defaults(void)
{
    int i;
    for (i = 0; i < sizeof(default_zones) / sizeof(default_zones[0]); i++)
        thermal_add(default_zones[i]);
}
