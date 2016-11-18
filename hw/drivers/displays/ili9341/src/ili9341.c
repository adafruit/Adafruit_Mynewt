/*****************************************************************************/
/*!
    @file     ili9341.c
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
/*****************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include "sysinit/sysinit.h"
#include "hal/hal_spi.h"
#include "ili9341.h"
#include "ili9341_priv.h"

#if MYNEWT_VAL(ILI9341_LOG)
#include "log/log.h"
#endif

#if MYNEWT_VAL(ILI9341_STATS)
#include "stats/stats.h"
#endif

#if MYNEWT_VAL(ILI9341_STATS)
/* Define the stats section and records */
STATS_SECT_START(ili9341_stat_section)
    STATS_SECT_ENTRY(errors)
STATS_SECT_END

/* Define stat names for querying */
STATS_NAME_START(ili9341_stat_section)
    STATS_NAME(ili9341_stat_section, errors)
STATS_NAME_END(ili9341_stat_section)

/* Global variable used to hold stats data */
STATS_SECT_DECL(ili9341_stat_section) g_ili9341stats;
#endif

#if MYNEWT_VAL(ILI9341_LOG)
#define LOG_MODULE_ILI9341    (9341)
#define ILI9341_INFO(...)     LOG_INFO(&_log, LOG_MODULE_ILI9341, __VA_ARGS__)
#define ILI9341_ERR(...)      LOG_ERROR(&_log, LOG_MODULE_ILI9341, __VA_ARGS__)
static struct log _log;
#else
#define ILI9341_INFO(...)
#define ILI9341_ERR(...)
#endif

int ili9341_spi_write(uint8_t b)
{
    /* ToDo! */
    return 0;
}

int ili9341_write_command(uint8_t c)
{
    /* ToDo! */
    return 0;
}

int ili9341_write_data(uint8_t d)
{
    /* ToDo! */
    return 0;
}

void
ili9341_init(void)
{
    int rc;

#if !MYNEWT_VAL(ILI9341_TASK)
    return;
#endif

#if MYNEWT_VAL(ILI9341_LOG)
    log_register("ili9341", &_log, &log_console_handler, NULL, LOG_SYSLEVEL);
#endif

#if MYNEWT_VAL(ILI9341_CLI)
    rc = ili9341_shell_init();
    SYSINIT_PANIC_ASSERT(rc == 0);
#endif

#if MYNEWT_VAL(ILI9341_STATS)
    /* Initialise the stats entry */
    rc = stats_init(
        STATS_HDR(g_ili9341stats),
        STATS_SIZE_INIT_PARMS(g_ili9341stats, STATS_SIZE_32),
        STATS_NAME_INIT_PARMS(ili9341_stat_section));
    SYSINIT_PANIC_ASSERT(rc == 0);
    /* Register the entry with the stats registry */
    rc = stats_register("ili9341", STATS_HDR(g_ili9341stats));
    SYSINIT_PANIC_ASSERT(rc == 0);
#endif
}
