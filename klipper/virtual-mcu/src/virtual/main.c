// Main starting point for the virtual (simulated) micro-controller
//
// Based on src/linux/main.c - Copyright (C) 2017  Kevin O'Connor
// <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <stdio.h> // fprintf
#include <unistd.h> // getopt
#include "command.h" // DECL_CONSTANT
#include "../linux/internal.h" // console_setup
#include "sched.h" // sched_main

DECL_CONSTANT_STR("MCU", "virtual");


/****************************************************************
 * Restart
 ****************************************************************/

static char **orig_argv;

void
command_config_reset(uint32_t *args)
{
    if (! sched_is_shutdown())
        shutdown("config_reset only available when shutdown");
    int ret = execv(orig_argv[0], orig_argv);
    report_errno("execv", ret);
}
DECL_COMMAND_FLAGS(command_config_reset, HF_IN_SHUTDOWN, "config_reset");


/****************************************************************
 * Startup
 ****************************************************************/

int
main(int argc, char **argv)
{
    // Parse program args
    orig_argv = argv;
    int opt;
    char *serial = "/tmp/pseudoserial";
    while ((opt = getopt(argc, argv, "I:")) != -1) {
        switch (opt) {
        case 'I':
            serial = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s [-I path]\n", argv[0]);
            return -1;
        }
    }

    // Initial setup
    int ret = console_setup(serial);
    if (ret)
        return -1;
    fprintf(stderr, "Virtual MCU started: serial=%s\n", serial);

    // Main loop
    sched_main();
    return 0;
}
