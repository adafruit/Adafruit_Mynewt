/**************************************************************************/
/*!
    @file     ili9341_shell.c
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
#include <errno.h>
#include "sysinit/sysinit.h"
#include "console/console.h"
#include "shell/shell.h"
#include "hal/hal_gpio.h"
#include "ili9341.h"
#include "ili9341_priv.h"

#if MYNEWT_VAL(ILI9341_CLI)
static int ili9341_shell_cmd(int argc, char **argv);

static struct shell_cmd ili9341_shell_cmd_struct = {
    .sc_cmd = "ili9341",
    .sc_cmd_func = ili9341_shell_cmd
};

/*
static int
ili9341_shell_stol(char *param_val, long min, long max, long *output)
{
    char *endptr;
    long lval;

    lval = strtol(param_val, &endptr, 10);
    if (param_val != '\0' && *endptr == '\0' &&
        lval >= min && lval <= max) {
            *output = lval;
    } else {
        return EINVAL;
    }

    return 0;
}
*/

/*
ili9341_shell_err_too_many_args(char *cmd_name)
{
    console_printf("Error: too many arguments for command \"%s\"\n",
                   cmd_name);
    return -1;
}
*/

static int
ili9341_shell_err_unknown_arg(char *cmd_name)
{
    console_printf("Error: unknown argument \"%s\"\n",
                   cmd_name);
    return -1;
}

/*
static int
ili9341_shell_err_invalid_arg(char *cmd_name)
{
    console_printf("Error: invalid argument \"%s\"\n",
                   cmd_name);
    return -1;
}
*/

static int
ili9341_shell_help(void)
{
    console_printf("%s cmd [flags...]\n", ili9341_shell_cmd_struct.sc_cmd);
    console_printf("cmd:\n");
    console_printf("  ToDo!\n");

    return 0;
}

static int
ili9341_shell_cmd(int argc, char **argv)
{
    if (argc == 1) {
        return ili9341_shell_help();
    }

    return ili9341_shell_err_unknown_arg(argv[1]);
}

int
ili9341_shell_init(void)
{
    int rc;

    rc = shell_cmd_register(&ili9341_shell_cmd_struct);
    SYSINIT_PANIC_ASSERT(rc == 0);

    return rc;
}

#endif
