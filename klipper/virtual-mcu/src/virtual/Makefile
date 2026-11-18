# Additional build rules for the virtual (simulated) mcu
#
# Console (pty) and watchdog handling are reused unchanged from the Linux
# mcu target; timers are adapted in timer.c and the "hardware" is simulated.

dirs-y += src/linux src/virtual src/generic

src-y += virtual/main.c virtual/gpio.c virtual/adc.c virtual/hard_pwm.c
src-y += virtual/spi.c virtual/i2c.c virtual/thermal.c virtual/motion.c
src-y += virtual/timer.c linux/console.c linux/watchdog.c
src-y += generic/crc16_ccitt.c generic/alloc.c

CFLAGS_klipper.elf += -lutil -lrt -lpthread -lm
