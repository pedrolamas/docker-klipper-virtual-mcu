// Simulated (empty) hardware spi bus for the virtual mcu
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <string.h> // memset
#include "command.h" // DECL_ENUMERATION
#include "gpio.h" // spi_setup
#include "sched.h" // sched_shutdown

DECL_ENUMERATION("spi_bus", "spi", 0);

struct spi_config
spi_setup(uint32_t bus, uint8_t mode, uint32_t rate)
{
    if (bus)
        shutdown("Invalid spi_setup parameters");
    return (struct spi_config){ .mode = mode, .rate = rate };
}

void
spi_prepare(struct spi_config config)
{
}

void
spi_transfer(struct spi_config config, uint8_t receive_data
             , uint8_t len, uint8_t *data)
{
    // No device is attached - the bus reads back as all zeros
    if (receive_data)
        memset(data, 0, len);
}
