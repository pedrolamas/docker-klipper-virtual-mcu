# Additional build rules for the virtual (simulated) mcu
#
# Timer, console (pty) and watchdog handling are reused unchanged from
# the Linux mcu target; only the "hardware" is simulated here.

dirs-y += src/linux src/virtual src/generic

src-y += virtual/main.c virtual/gpio.c virtual/adc.c virtual/hard_pwm.c
src-y += virtual/spi.c virtual/i2c.c
src-y += linux/timer.c linux/console.c linux/watchdog.c
src-y += generic/crc16_ccitt.c generic/alloc.c

CFLAGS_klipper.elf += -lutil -lrt -lpthread
