// Simulated (empty) hardware spi bus for the virtual mcu
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <string.h> // memset
#include "command.h" // DECL_ENUMERATION
#include "gpio.h" // spi_setup
#include "sched.h" // sched_shutdown

// Common bus names from other mcus, all of them the same empty bus
DECL_ENUMERATION("spi_bus", "spi", 0);
DECL_ENUMERATION("spi_bus", "spi0", 0);
DECL_ENUMERATION("spi_bus", "spi0a", 0);
DECL_ENUMERATION("spi_bus", "spi0b", 0);
DECL_ENUMERATION("spi_bus", "spi0c", 0);
DECL_ENUMERATION("spi_bus", "spi0d", 0);
DECL_ENUMERATION("spi_bus", "spi1", 0);
DECL_ENUMERATION("spi_bus", "spi1a", 0);
DECL_ENUMERATION("spi_bus", "spi1b", 0);
DECL_ENUMERATION("spi_bus", "spi1c", 0);
DECL_ENUMERATION("spi_bus", "spi2", 0);
DECL_ENUMERATION("spi_bus", "spi2a", 0);
DECL_ENUMERATION("spi_bus", "spi2b", 0);
DECL_ENUMERATION("spi_bus", "spi3", 0);
DECL_ENUMERATION("spi_bus", "spi3a", 0);
DECL_ENUMERATION("spi_bus", "spi4", 0);
DECL_ENUMERATION("spi_bus", "spi5", 0);
DECL_ENUMERATION("spi_bus", "spi6", 0);

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
