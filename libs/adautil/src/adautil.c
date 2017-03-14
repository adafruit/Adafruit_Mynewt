/**************************************************************************/
/*!
    @file     adautil.c
    @author   hathach

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
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/

#include "adafruit/adautil.h"
#include "bsp/bsp.h"
#include "hal/hal_system.h"
#include <shell/shell.h>

/*------------------------------------------------------------------*/
/* MACRO TYPEDEF CONSTANT ENUM
 *------------------------------------------------------------------*/
/* Magic number that bootloader will activate boot_serial when detected
 * Must match BOOT_RESET_TO_DFU_MAGIC in syscfg.yml of Bootloader */
#define BOOTLOADER_RESET_TO_DFU_MAGIC   0xDF

/*------------------------------------------------------------------*/
/* VARIABLE DECLARATION
 *------------------------------------------------------------------*/
#if MYNEWT_VAL(ADAUTIL_DFU_CLI)

static int _adautil_enter_dfu(int argc, char **argv)
{
  NRF_POWER->GPREGRET = BOOTLOADER_RESET_TO_DFU_MAGIC;
  NVIC_SystemReset();
}

static struct shell_cmd _adautil_cmd[] =
{
    { .sc_cmd = "dfu", .sc_cmd_func = _adautil_enter_dfu },
};

#endif

/*------------------------------------------------------------------*/
/* FUNCTION DECLARATION
 *------------------------------------------------------------------*/

void adautil_init(void)
{
#if MYNEWT_VAL(ADAUTIL_DFU_CLI)
  (void) shell_cmd_register(&_adautil_cmd[0]);
#endif
}
