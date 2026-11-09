// Simulated (empty) hardware i2c bus for the virtual mcu
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <string.h> // memset
#include "command.h" // DECL_ENUMERATION
#include "gpio.h" // i2c_setup
#include "i2ccmds.h" // I2C_BUS_SUCCESS
#include "sched.h" // sched_shutdown

DECL_ENUMERATION("i2c_bus", "twi", 0);

struct i2c_config
i2c_setup(uint32_t bus, uint32_t rate, uint8_t addr)
{
    if (bus)
        shutdown("Unsupported i2c bus");
    return (struct i2c_config){ .addr = addr };
}

int
i2c_write(struct i2c_config config, uint8_t write_len, uint8_t *write)
{
    return I2C_BUS_SUCCESS;
}

int
i2c_read(struct i2c_config config, uint8_t reg_len, uint8_t *reg
         , uint8_t read_len, uint8_t *read)
{
    // No device is attached - reads return all zeros
    memset(read, 0, read_len);
    return I2C_BUS_SUCCESS;
}
