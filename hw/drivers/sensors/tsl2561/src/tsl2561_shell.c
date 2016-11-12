/**************************************************************************/
/*!
    @file     tsl2561_shell.c
    @author   ktown (Adafruit Industries)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2016, Adafruit Industries (adafruit.com)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/

#include <string.h>
#include "sysinit/sysinit.h"
#include "console/console.h"
#include "shell/shell.h"
#include "adafruit/tsl2561.h"
#include "tsl2561_priv.h"
#include "tsl2561_shell.h"

#if MYNEWT_VAL(TSL2561_CLI)
static int tsl2561_shell_cmd(int argc, char **argv);

static struct shell_cmd tsl2561_shell_cmd_struct = {
    .sc_cmd = "tsl2561",
    .sc_cmd_func = tsl2561_shell_cmd
};

/*
static struct kv_pair {
    char *key;
    int val;
};

static struct kv_pair tsl2561_shell_cmd_int_time[] = {
    { "13", TSL2561_INTEGRATIONTIME_13MS },
    { "101", TSL2561_INTEGRATIONTIME_101MS },
    { "402", TSL2561_INTEGRATIONTIME_402MS },
    { NULL },
};

static int
tsl2561_shell_err_too_few_args(char *cmd_name)
{
    console_printf("Error: too few arguments for command \"%s\"\n",
                   cmd_name);
    return -1;
}
*/

static int
tsl2561_shell_err_too_many_args(char *cmd_name)
{
    console_printf("Error: too many arguments for command \"%s\"\n",
                   cmd_name);
    return -1;
}

static int
tsl2561_shell_err_unknown_arg(char *cmd_name)
{
    console_printf("Error: unknown argument \"%s\"\n",
                   cmd_name);
    return -1;
}

static int
tsl2561_shell_help(void) {
    console_printf("%s [r]\n", tsl2561_shell_cmd_struct.sc_cmd);
    console_printf("%s [gain]\n", tsl2561_shell_cmd_struct.sc_cmd);
    console_printf("%s [time]\n", tsl2561_shell_cmd_struct.sc_cmd);

    return 0;
}

static int
tsl2561_shell_cmd_read(int argc, char **argv) {
    uint16_t full;
    uint16_t ir;

    int rc = tsl2561_get_data(&full, &ir);
    if (rc != 0) {
        console_printf("Read failed: %d\n", rc);
        return rc;
    }

    console_printf("Full:  %u\n", full);
    console_printf("IR:    %u\n", ir);

    return 0;
}

static int
tsl2561_shell_cmd_gain(int argc, char **argv) {
    if (argc > 2) {
        return tsl2561_shell_err_too_many_args(argv[1]);
    }

    uint8_t gain = tsl2561_get_gain();
    console_printf("0x%02X\n", gain);

    return 0;
}

static int
tsl2561_shell_cmd_time(int argc, char **argv) {
    if (argc > 2) {
        return tsl2561_shell_err_too_many_args(argv[1]);
    }

    uint8_t time = tsl2561_get_integration_time();
    console_printf("0x%02X\n", time);

    return 0;
}

static int
tsl2561_shell_cmd(int argc, char **argv) {
    if (argc == 1) {
        return tsl2561_shell_help();
    }

    /* Read command (get a new data sample) */
    if (argc > 1 && strcmp(argv[1], "r") == 0) {
      return tsl2561_shell_cmd_read(argc, argv);
    }

    /* Gain command */
    if (argc > 1 && strcmp(argv[1], "gain") == 0) {
      return tsl2561_shell_cmd_gain(argc, argv);
    }

    /* Integration time command */
    if (argc > 1 && strcmp(argv[1], "time") == 0) {
      return tsl2561_shell_cmd_time(argc, argv);
    }

    return tsl2561_shell_err_unknown_arg(argv[1]);
}

int
tsl2561_shell_init(void) {
    int rc = shell_cmd_register(&tsl2561_shell_cmd_struct);
    SYSINIT_PANIC_ASSERT(rc == 0);
    return rc;
}

#endif
