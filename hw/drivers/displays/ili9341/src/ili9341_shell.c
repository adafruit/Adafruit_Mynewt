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
#include "hal/hal_spi.h"
#include "ili9341.h"
#include "ili9341_priv.h"

#if MYNEWT_VAL(ILI9341_CLI)
static int ili9341_shell_cmd(int argc, char **argv);

static struct shell_cmd ili9341_shell_cmd_struct = {
    .sc_cmd = "ili9341",
    .sc_cmd_func = ili9341_shell_cmd
};

static int
ili9341_shell_stol(char *param_val, long min, long max, long *output, uint8_t b)
{
    char *endptr;
    long lval;

    if ((b != 10) && (b != 16)) {
        return EINVAL;
    }

    lval = strtol(param_val, &endptr, b);
    if (param_val != '\0' && *endptr == '\0' &&
        lval >= min && lval <= max) {
            *output = lval;
    } else {
        return EINVAL;
    }

    return 0;
}

static int
ili9341_shell_err_too_few_args(char *cmd_name)
{
    console_printf("Error: too few arguments for command \"%s\"\n",
                   cmd_name);
    return EINVAL;
}

static int
ili9341_shell_err_too_many_args(char *cmd_name)
{
    console_printf("Error: too many arguments for command \"%s\"\n",
                   cmd_name);
    return EINVAL;
}

static int
ili9341_shell_err_unknown_arg(char *cmd_name)
{
    console_printf("Error: unknown argument \"%s\"\n",
                   cmd_name);
    return EINVAL;
}

static int
ili9341_shell_err_invalid_arg(char *cmd_name)
{
    console_printf("Error: invalid argument \"%s\"\n",
                   cmd_name);
    return EINVAL;
}

static int
ili9341_shell_help(void)
{
    console_printf("%s cmd [flags...]\n", ili9341_shell_cmd_struct.sc_cmd);
    console_printf("cmd:\n");
    console_printf("  init\n");
    console_printf("  debug [verbose]\n");
    console_printf("  fill <color:0xFFFF>\n");

    return 0;
}

static int
ili9341_shell_cmd_init(int argc, char **argv)
{
    int rc;

    if (argc > 2) {
        return ili9341_shell_err_too_many_args(argv[1]);
    }

    /* Configure P0.30 as output and high to deassert (touch screen SPI CS!) */
    /* This is a test to avoid SPI conflicts if the CS pin is misconfigured */
    rc = hal_gpio_init_out(30, 1);
    if (!rc == 0) {
        goto error;
    }

    /* Configure P0.30 as output and high to deassert (SD card SPI CS!) */
    /* This is a test to avoid SPI conflicts if the CS pin is misconfigured */
    rc = hal_gpio_init_out(27, 1);
    if (!rc == 0) {
        goto error;
    }

    /* Configure SSEL/CS and DC pins as outputs */
    rc = hal_gpio_init_out(MYNEWT_VAL(ILI9341_SS_PIN), 1);
    rc = hal_gpio_init_out(MYNEWT_VAL(ILI9341_DC_PIN), 1);
    if (!rc == 0) {
        goto error;
    }

    /* Enable the SPI bus */
    rc = hal_spi_enable(MYNEWT_VAL(ILI9341_SPI_BUS));
    if (!rc == 0) {
        goto error;
    }

    /* Initialise the display */
    rc = ili9341_disp_init();
    if (!rc == 0) {
        goto error;
    }

    /* Clear screen */
    rc = ili9341_fill_screen(0x0000);
    if (!rc == 0) {
        goto error;
    }

    /* Disable the SPI bus to save power */
    rc = hal_spi_disable(MYNEWT_VAL(ILI9341_SPI_BUS));
    assert(rc == 0);

error:
    return rc;
}

static int
ili9341_shell_cmd_debug(int argc, char **argv)
{
    int rc;
    uint8_t x;
    uint8_t verbose;

    if (argc > 3) {
        return ili9341_shell_err_too_many_args(argv[1]);
    }

    /* Check if we want verbose output or not */
    if (argc == 3 && strcmp(argv[2], "verbose") == 0) {
        verbose = 1;
    } else if (argc == 3) {
        return ili9341_shell_err_invalid_arg(argv[2]);
    } else {
        verbose = 0;
    }

    /* Enable the SPI bus */
    rc = hal_spi_enable(MYNEWT_VAL(ILI9341_SPI_BUS));
    assert(rc == 0);

    /* DISPLAY POWER MODE (0x0A)                          DEFAULT VAL: 0x08 */
    /* ==================================================================== */
    /* BIT  Val Description                                                 */
    /* ---  --- ----------------------------------------------------------- */
    /*   7    0 Booster off or has a fault                                  */
    /*        1 Booster on and working OK                                   */
    /*   6    0 Idle mode off                                               */
    /*        1 Idle mode on                                                */
    /*   5    0 Partial mode off                                            */
    /*        1 Partial mode on                                             */
    /*   4    0 Sleep mode on                                               */
    /*        1 Sleep mode off                                              */
    /*   3    0 Display normal mode off                                     */
    /*        1 Display normal mode on                                      */
    /*   2    0 Display is off                                              */
    /*        1 Display is on                                               */
    /*   1   -- Not defined                                                 */
    /*   0   -- Not defined                                                 */
    x = 0;
    rc = ili9341_read_cmd8(ILI9341_RDMODE, &x);
    if (!rc == 0) {
        goto error;
    }
    console_printf("Display Power Mode: 0x%02X\n", x);
    if (verbose) {
        console_printf("\t%s\n", x & 0x80 ? "Booster: on" :
                                            "Boost: off/faulty!");
        console_printf("\t%s\n", x & 0x40 ? "Idle mode: on" :
                                            "Idle mode: off");
        console_printf("\t%s\n", x & 0x20 ? "Partial mode: on" :
                                            "Partial mode: off");
        console_printf("\t%s\n", x & 0x10 ? "Sleep mode: off" :
                                            "Sleep mode: on");
        console_printf("\t%s\n", x & 0x08 ? "Normal display mode: on" :
                                            "Normal display mode: off");
        console_printf("\t%s\n", x & 0x04 ? "Display: on" :
                                            "Display: off");
    }


    /* MADCTL (0x0B)                                      DEFAULT VAL: 0x00 */
    /* ==================================================================== */
    /* BIT  Val Description                                                 */
    /* ---  --- ----------------------------------------------------------- */
    /*   7    0 Top to Bottom                                               */
    /*        1 Bottom to Top                                               */
    /*   6    0 Left to Right                                               */
    /*        1 Right to Left                                               */
    /*   5    0 Normal Mode                                                 */
    /*        1 Reverse Mode                                                */
    /*   4    0 LCD Refresh Top to Bottom                                   */
    /*        1 LCD Refresh Bottom to Top                                   */
    /*   3    0 RGB                                                         */
    /*        1 BGR                                                         */
    /*   2    0 LCD Refresh Left to Right                                   */
    /*        1 LCD Refresh Right to Left                                   */
    /*   1   -- Always 0                                                    */
    /*   0   -- Always 0                                                    */
    x = 0;
    rc = ili9341_read_cmd8(ILI9341_RDMADCTL, &x);
    if (!rc == 0) {
        goto error;
    }
    console_printf("MADCTL Mode: 0x%02X\n", x);
    if (verbose) {
        console_printf("\t%s\n", x & 0x80 ? "Vert: bottom to top" :
                                            "Vert: top to bottom");
        console_printf("\t%s\n", x & 0x40 ? "Horz: right to left" :
                                            "Horz: left to right");
        console_printf("\t%s\n", x & 0x20 ? "Reverse mode" :
                                            "Normal mode");
        console_printf("\t%s\n", x & 0x10 ? "Vert refresh: bottom to top" :
                                            "Vert refresh: top to bottom");
        console_printf("\t%s\n", x & 0x08 ? "Component order: BGR" :
                                            "Component order: RGB");
        console_printf("\t%s\n", x & 0x04 ? "Horz refresh: right to left" :
                                            "Horz refresh: left to right");
    }

    /* DISPLAY PIXEL FORMAT (0x0C)                          DEFAULT VAL: -- */
    /* ==================================================================== */
    /* BIT  Name Description                                                */
    /* ---  ---- ---------------------------------------------------------- */
    /*   7  RIM  RGB Interface (not relevant)                               */
    /* 6:4  DPI  RGB Interface (not relevant)                               */
    /*   3  ---  Reserved (always 0)                                        */
    /* 2:0  DBI  MCU Interface                                              */
    /*           000 = Reserved                                             */
    /*           001 = Reserved                                             */
    /*           010 = Reserved                                             */
    /*           011 = Reserved                                             */
    /*           100 = Reserved                                             */
    /*           101 = 16 bits/pixel                                        */
    /*           110 = 18 bits/pixel                                        */
    /*           111 = Reserved                                             */
    x = 0;
    rc = ili9341_read_cmd8(ILI9341_RDPIXFMT, &x);
    if (!rc == 0) {
        goto error;
    }
    console_printf("Pixel Format: 0x%02X\n", x);
    if (verbose) {
        if ((x & 0b111) == 0x05) {
            console_printf("\t16 bits per pixel\n");
        } else if ((x & 0b111) == 0x06) {
            console_printf("\t18 bits per pixel\n");
        }
        else {
            console_printf("\tUnknown bits per pixel!\n");
        }
    }

    /* DISPLAY IMAGE FORMAT (0x0D)                          DEFAULT VAL: -- */
    /* ==================================================================== */
    /* BIT  Val  Description                                                */
    /* ---  ---  ---------------------------------------------------------- */
    /* 2:0  000  Gamma Curve 1                                              */
    x = 0;
    rc = ili9341_read_cmd8(ILI9341_RDIMGFMT, &x);
    if (!rc == 0) {
        goto error;
    }
    console_printf("Image Format: 0x%02X\n", x);
    if (verbose) {
        if ((x & 0b111) == 0) {
            console_printf("\tGamma Curve 1\n");
        } else {
            console_printf("\tUnknown Gamme Curve\n");
        }
    }

    /* DISPLAY SELF DIAGNOSTIC (0x0F)                       DEFAULT VAL: -- */
    /* ==================================================================== */
    /* BIT  Description                                                     */
    /* ---  --------------------------------------------------------------- */
    /*   7  1 if register access is working properly                        */
    /*   6  1 if the display is functioning properly                        */
    x = 0;
    rc = ili9341_read_cmd8(ILI9341_RDSELFDIAG, &x);
    if (!rc == 0) {
        goto error;
    }
    console_printf("Self Diagnostic: 0x%02X\n", x);
    if (verbose) {
        if ((x & 0x80) == 0x80) {
            console_printf("\tRegister access functioning properly\n");
        } else {
            console_printf("\tRegister access error detected!\n");
        }
        if ((x & 0x40) == 0x40) {
            console_printf("\tDisplay is functioning properly\n");
        } else {
            console_printf("\tDisplay error detected!\n");
        }
    }

    /* Disable the SPI bus */
    rc = hal_spi_disable(MYNEWT_VAL(ILI9341_SPI_BUS));
    assert(rc == 0);

error:
    return rc;
}

static int
ili9341_shell_cmd_fill(int argc, char **argv)
{
    int rc;
    long val;

    if (argc < 3) {
        return ili9341_shell_err_too_few_args(argv[1]);
    }

    if (argc > 3) {
        return ili9341_shell_err_too_many_args(argv[1]);
    }

    if (ili9341_shell_stol(argv[2], 0, UINT16_MAX, &val, 16)) {
        return ili9341_shell_err_invalid_arg(argv[2]);
    }

    /* Enable the SPI bus */
    rc = hal_spi_enable(MYNEWT_VAL(ILI9341_SPI_BUS));
    if (!rc == 0) {
        goto error;
    }

    rc = ili9341_fill_screen((uint16_t)val);
    if (!rc == 0) {
        goto error;
    }

    /* Disable the SPI bus */
    rc = hal_spi_disable(MYNEWT_VAL(ILI9341_SPI_BUS));
    if (!rc == 0) {
        goto error;
    }

error:
    return rc;
}

static int
ili9341_shell_cmd(int argc, char **argv)
{
    int rc;

    if (argc == 1) {
        return ili9341_shell_help();
    }

    /* Command handlers */
    rc = 0;
    if (argc > 1 && strcmp(argv[1], "init") == 0) {
        rc = ili9341_shell_cmd_init(argc, argv);
    } else if (argc > 1 && strcmp(argv[1], "debug") == 0) {
        rc = ili9341_shell_cmd_debug(argc, argv);
    } else if (argc > 1 && strcmp(argv[1], "fill") == 0) {
        rc = ili9341_shell_cmd_fill(argc, argv);
    } else {
        return ili9341_shell_err_unknown_arg(argv[1]);
    }

    /* Giving an error warning if relevant */
    if (rc == EIO) {
        console_printf("ILI9341 not initialised (\'%s init\')\n",
                    ili9341_shell_cmd_struct.sc_cmd);
    } else if (!rc == 0) {
        console_printf("RC = 0x%04X (%u)\n", rc, rc);
    }

    return rc;
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
