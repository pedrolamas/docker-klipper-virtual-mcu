// Simulated (empty) hardware i2c bus for the virtual mcu
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <string.h> // memset
#include "command.h" // DECL_ENUMERATION
#include "gpio.h" // i2c_setup
#include "i2ccmds.h" // I2C_BUS_SUCCESS
#include "sched.h" // sched_shutdown

// Common bus names from other mcus, all of them the same empty bus
DECL_ENUMERATION("i2c_bus", "twi", 0);
DECL_ENUMERATION("i2c_bus", "i2c0", 0);
DECL_ENUMERATION("i2c_bus", "i2c0a", 0);
DECL_ENUMERATION("i2c_bus", "i2c0b", 0);
DECL_ENUMERATION("i2c_bus", "i2c0c", 0);
DECL_ENUMERATION("i2c_bus", "i2c0d", 0);
DECL_ENUMERATION("i2c_bus", "i2c0e", 0);
DECL_ENUMERATION("i2c_bus", "i2c0f", 0);
DECL_ENUMERATION("i2c_bus", "i2c0g", 0);
DECL_ENUMERATION("i2c_bus", "i2c0h", 0);
DECL_ENUMERATION("i2c_bus", "i2c1", 0);
DECL_ENUMERATION("i2c_bus", "i2c1a", 0);
DECL_ENUMERATION("i2c_bus", "i2c1b", 0);
DECL_ENUMERATION("i2c_bus", "i2c1c", 0);
DECL_ENUMERATION("i2c_bus", "i2c1d", 0);
DECL_ENUMERATION("i2c_bus", "i2c1e", 0);
DECL_ENUMERATION("i2c_bus", "i2c1f", 0);
DECL_ENUMERATION("i2c_bus", "i2c1g", 0);
DECL_ENUMERATION("i2c_bus", "i2c2", 0);
DECL_ENUMERATION("i2c_bus", "i2c2a", 0);
DECL_ENUMERATION("i2c_bus", "i2c3", 0);
DECL_ENUMERATION("i2c_bus", "i2c3a", 0);
DECL_ENUMERATION("i2c_bus", "i2c4", 0);

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
