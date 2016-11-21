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
#include "hal/hal_gpio.h"
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

struct hal_spi_settings g_ili9341_spi_settings = {
    .data_mode = HAL_SPI_MODE0,
    .data_order = HAL_SPI_MSB_FIRST,
    .word_size = HAL_SPI_WORD_SIZE_8BIT,
    .baudrate = 8000
};

uint8_t g_ili9341_initialised = 0;

int
ili9341_write_command(uint8_t c)
{
    int rc;
    uint8_t txbuf[1] = { c };
    uint8_t rxbuf[1] = { 0x00 };

    /* Set DC LOW, CS LOW */
    hal_gpio_write(MYNEWT_VAL(ILI9341_DC_PIN), 0);
    hal_gpio_write(MYNEWT_VAL(ILI9341_SS_PIN), 0);
    /* Send SPI data */
    rc = hal_spi_txrx(MYNEWT_VAL(ILI9341_SPI_BUS), txbuf, rxbuf, 1);
    /* Set CS HIGH */
    hal_gpio_write(MYNEWT_VAL(ILI9341_SS_PIN), 1);

#if MYNEWT_VAL(ILI9341_LOG)
    if (!rc == 0) {
        ILI9341_ERR("Error: RC = 0x%04d (%u)\n", rc, rc);
#if MYNEWT_VAL(ILI9341_STATS)
        STATS_INC(g_ili9341stats, errors);
#endif
    }
#endif

    return rc;
}

int
ili9341_write_data(uint8_t d)
{
    int rc;
    uint8_t txbuf[1] = { d };
    uint8_t rxbuf[1] = { 0x00 };

    /* Set DC HIGH, CS LOW */
    hal_gpio_write(MYNEWT_VAL(ILI9341_DC_PIN), 1);
    hal_gpio_write(MYNEWT_VAL(ILI9341_SS_PIN), 0);
    /* Send SPI data */
    rc = hal_spi_txrx(MYNEWT_VAL(ILI9341_SPI_BUS), txbuf, rxbuf, 1);
    /* Set CS HIGH */
    hal_gpio_write(MYNEWT_VAL(ILI9341_SS_PIN), 1);

#if MYNEWT_VAL(ILI9341_LOG)
    if (!rc == 0) {
        ILI9341_ERR("Error: RC = 0x%04d (%u)\n", rc, rc);
#if MYNEWT_VAL(ILI9341_STATS)
        STATS_INC(g_ili9341stats, errors);
#endif
    }
#endif

    return rc;
}

int
ili9341_read_cmd8(uint8_t reg, uint8_t *val)
{
    int rc;
    uint8_t txbuf[1];
    uint8_t rxbuf[1] = { 0x00 };

    /* Sekret handshake */
    hal_gpio_write(MYNEWT_VAL(ILI9341_DC_PIN), 0);
    hal_gpio_write(MYNEWT_VAL(ILI9341_SS_PIN), 0);
    txbuf[0] = 0xD9;
    rc = hal_spi_txrx(MYNEWT_VAL(ILI9341_SPI_BUS), txbuf, rxbuf, 1);
    hal_gpio_write(MYNEWT_VAL(ILI9341_DC_PIN), 1);
    txbuf[0] = 0x10;
    rc = hal_spi_txrx(MYNEWT_VAL(ILI9341_SPI_BUS), txbuf, rxbuf, 1);
    hal_gpio_write(MYNEWT_VAL(ILI9341_SS_PIN), 1);
    if (!rc == 0) {
        goto error;
    }

    /* Set the register to read ... */
    hal_gpio_write(MYNEWT_VAL(ILI9341_DC_PIN), 0);
    hal_gpio_write(MYNEWT_VAL(ILI9341_SS_PIN), 0);
    txbuf[0] = reg;
    rc = hal_spi_txrx(MYNEWT_VAL(ILI9341_SPI_BUS), txbuf, rxbuf, 1);

    /* ... then read the results back */
    hal_gpio_write(MYNEWT_VAL(ILI9341_DC_PIN), 1);
    txbuf[0] = 0xFF;
    rc = hal_spi_txrx(MYNEWT_VAL(ILI9341_SPI_BUS),
                      txbuf, rxbuf, 1);
    hal_gpio_write(MYNEWT_VAL(ILI9341_SS_PIN), 1);
    if (!rc == 0) {
        goto error;
    }

    /* update 'val' with the read results */
    memcpy(val, rxbuf, 1);

error:
#if MYNEWT_VAL(ILI9341_LOG)
    if (!rc == 0) {
        ILI9341_ERR("Error: RC = 0x%04d (%u)\n", rc, rc);
#if MYNEWT_VAL(ILI9341_STATS)
        STATS_INC(g_ili9341stats, errors);
#endif
    }
#endif
    return rc;
}

int
ili9341_disp_init(void)
{
    int rc;

#if MYNEWT_VAL(ILI9341_LOG)
    ILI9341_INFO("Initialising ILI9341\n");
#endif

    rc = ili9341_write_command(0xEF);
    /* Normally if SPI will fail, it should happen in the first command */
    if (!rc == 0) {
        goto error;
    }
    rc = ili9341_write_data(0x03);
    rc = ili9341_write_data(0x80);
    rc = ili9341_write_data(0x02);

    rc = ili9341_write_command(0xCF);
    rc = ili9341_write_data(0x00);
    rc = ili9341_write_data(0XC1);
    rc = ili9341_write_data(0X30);

    rc = ili9341_write_command(0xED);
    rc = ili9341_write_data(0x64);
    rc = ili9341_write_data(0x03);
    rc = ili9341_write_data(0X12);
    rc = ili9341_write_data(0X81);

    rc = ili9341_write_command(0xE8);
    rc = ili9341_write_data(0x85);
    rc = ili9341_write_data(0x00);
    rc = ili9341_write_data(0x78);

    rc = ili9341_write_command(0xCB);
    rc = ili9341_write_data(0x39);
    rc = ili9341_write_data(0x2C);
    rc = ili9341_write_data(0x00);
    rc = ili9341_write_data(0x34);
    rc = ili9341_write_data(0x02);

    rc = ili9341_write_command(0xF7);
    rc = ili9341_write_data(0x20);

    rc = ili9341_write_command(0xEA);
    rc = ili9341_write_data(0x00);
    rc = ili9341_write_data(0x00);

    /* Power Control */
    rc = ili9341_write_command(ILI9341_PWCTR1);
    rc = ili9341_write_data(0x23);
    rc = ili9341_write_command(ILI9341_PWCTR2);
    rc = ili9341_write_data(0x10);

    /* VCM Control */
    rc = ili9341_write_command(ILI9341_VMCTR1);
    rc = ili9341_write_data(0x3e);
    rc = ili9341_write_data(0x28);
    rc = ili9341_write_command(ILI9341_VMCTR2);
    rc = ili9341_write_data(0x86);

    /* Memory Access Control */
    rc = ili9341_write_command(ILI9341_MADCTL);
    rc = ili9341_write_data(0x48);

    rc = ili9341_write_command(ILI9341_PIXFMT);
    rc = ili9341_write_data(0x55);

    rc = ili9341_write_command(ILI9341_FRMCTR1);
    rc = ili9341_write_data(0x00);
    rc = ili9341_write_data(0x18);

    /* Display Function Control */
    rc = ili9341_write_command(ILI9341_DFUNCTR);
    rc = ili9341_write_data(0x08);
    rc = ili9341_write_data(0x82);
    rc = ili9341_write_data(0x27);

    /* 3 Gamma Function Disable */
    rc = ili9341_write_command(0xF2);
    rc = ili9341_write_data(0x00);

    /* Gamma Curve Settings */
    rc = ili9341_write_command(ILI9341_GAMMASET);
    rc = ili9341_write_data(0x01);

    /* Set Gamma Curve */
    rc = ili9341_write_command(ILI9341_GMCTRP1);
    rc = ili9341_write_data(0x0F);
    rc = ili9341_write_data(0x31);
    rc = ili9341_write_data(0x2B);
    rc = ili9341_write_data(0x0C);
    rc = ili9341_write_data(0x0E);
    rc = ili9341_write_data(0x08);
    rc = ili9341_write_data(0x4E);
    rc = ili9341_write_data(0xF1);
    rc = ili9341_write_data(0x37);
    rc = ili9341_write_data(0x07);
    rc = ili9341_write_data(0x10);
    rc = ili9341_write_data(0x03);
    rc = ili9341_write_data(0x0E);
    rc = ili9341_write_data(0x09);
    rc = ili9341_write_data(0x00);
    rc = ili9341_write_command(ILI9341_GMCTRN1);
    rc = ili9341_write_data(0x00);
    rc = ili9341_write_data(0x0E);
    rc = ili9341_write_data(0x14);
    rc = ili9341_write_data(0x03);
    rc = ili9341_write_data(0x11);
    rc = ili9341_write_data(0x07);
    rc = ili9341_write_data(0x31);
    rc = ili9341_write_data(0xC1);
    rc = ili9341_write_data(0x48);
    rc = ili9341_write_data(0x08);
    rc = ili9341_write_data(0x0F);
    rc = ili9341_write_data(0x0C);
    rc = ili9341_write_data(0x31);
    rc = ili9341_write_data(0x36);
    rc = ili9341_write_data(0x0F);

    /* Exit Sleep (requires 120ms delay)*/
    rc = ili9341_write_command(ILI9341_SLPOUT);
    os_time_delay(((OS_TICKS_PER_SEC) / 1000) * 120);

    /* Display on */
    rc = ili9341_write_command(ILI9341_DISPON);

    /* Set initialised flag */
    g_ili9341_initialised = 1;

error:
#if MYNEWT_VAL(ILI9341_LOG)
    ILI9341_INFO("Exiting ILI9341 initilisation\n");
#endif
    return rc;
}

int
ili9341_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1,
                            uint16_t y1)
{
    int rc;

    if (!g_ili9341_initialised) {
        return EIO;
    }

    /* Set column address (X) */
    rc = ili9341_write_command(ILI9341_CASET);
    rc = ili9341_write_data(x0 >> 8);
    rc = ili9341_write_data(x0 & 0xFF);
    rc = ili9341_write_data(x1 >> 8);
    rc = ili9341_write_data(x1 & 0xFF);

    /* Set Row address (Y) */
    rc = ili9341_write_command(ILI9341_PASET);
    rc = ili9341_write_data(y0>>8);
    rc = ili9341_write_data(y0);
    rc = ili9341_write_data(y1>>8);
    rc = ili9341_write_data(y1);

    /* Write to RAM ... */
    rc = ili9341_write_command(ILI9341_RAMWR);

    return rc;
}

int
ili9341_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    int rc;
    uint8_t txbuf[2] = { (uint8_t)(color >> 8), (uint8_t)(color & 0xFF) };
    uint8_t rxbuf[2] = { 0x00, 0x00 };

    if((x < 0) || (x >= ILI9341_TFTWIDTH ) ||
       (y < 0) || (y >= ILI9341_TFTHEIGHT )) {
        return EINVAL;
    }

    if (!g_ili9341_initialised) {
        return EIO;
    }

    /* Set drawing position */
    rc = ili9341_set_addr_window(x, y, x, y);

    hal_gpio_write(MYNEWT_VAL(ILI9341_DC_PIN), 1);
    hal_gpio_write(MYNEWT_VAL(ILI9341_SS_PIN), 0);
    rc = hal_spi_txrx(MYNEWT_VAL(ILI9341_SPI_BUS), txbuf, rxbuf, 2);
    hal_gpio_write(MYNEWT_VAL(ILI9341_SS_PIN), 1);

#if MYNEWT_VAL(ILI9341_LOG)
    if (!rc == 0) {
        ILI9341_ERR("Error: RC = 0x%04d (%u)\n", rc, rc);
#if MYNEWT_VAL(ILI9341_STATS)
        STATS_INC(g_ili9341stats, errors);
#endif
    }
#endif

    return rc;
}

int
ili9341_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                      uint16_t color)
{
    int rc;
    uint8_t txbuf[2] = { (uint8_t)(color >> 8), (uint8_t)(color & 0xFF) };
    uint8_t rxbuf[2] = { 0x00, 0x00 };

    /* Validate the starting position */
    if((x < 0) || (x >= ILI9341_TFTWIDTH ) ||
       (y < 0) || (y >= ILI9341_TFTHEIGHT )) {
        return EINVAL;
    }

    if((x+w > ILI9341_TFTWIDTH ) ||
       (y+h > ILI9341_TFTHEIGHT )) {
        return EINVAL;
    }

    if (!g_ili9341_initialised) {
        return EIO;
    }

    /* Set drawing bounding box for speed purposes */
    rc = ili9341_set_addr_window(x, y, x+w-1, y+h-1);

    hal_gpio_write(MYNEWT_VAL(ILI9341_DC_PIN), 1);
    hal_gpio_write(MYNEWT_VAL(ILI9341_SS_PIN), 0);

    /* Send a continuous stream of colors */
    for(y=h; y>0; y--) {
        for(x=w; x>0; x--) {
            rc = hal_spi_txrx(MYNEWT_VAL(ILI9341_SPI_BUS), txbuf, rxbuf, 2);
            if (!rc == 0) {
                goto error;
            }
        }
    }

    hal_gpio_write(MYNEWT_VAL(ILI9341_SS_PIN), 1);

error:
#if MYNEWT_VAL(ILI9341_LOG)
    if (!rc == 0) {
        ILI9341_ERR("Error: RC = 0x%04d (%u)\n", rc, rc);
#if MYNEWT_VAL(ILI9341_STATS)
        STATS_INC(g_ili9341stats, errors);
#endif
    }
#endif
    return rc;
}

int
ili9341_fill_screen(uint16_t color) {
    return ili9341_fill_rect(0, 0,  ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT, color);
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

    /* Configure the SPI bus */
    rc = hal_spi_config(MYNEWT_VAL(ILI9341_SPI_BUS), &g_ili9341_spi_settings);
    SYSINIT_PANIC_ASSERT(rc == 0);

    /* ToDo: Configure and init ILI9341! */
}
