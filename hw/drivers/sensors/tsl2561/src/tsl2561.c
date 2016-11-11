/*****************************************************************************/
/*!
    @file     tsl2561.c
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

#include "sysinit/sysinit.h"
#include "adafruit/tsl2561.h"
#include "hal/hal_i2c.h"
#include "console/console.h"
#include "shell/shell.h"

#include "tsl2561_priv.h"

/* ToDo: Add timer based polling and data ready callback */
/* ToDo: Add LOG support */
/* ToDo: Add STATS support */

#define TSL2561_REGISTER_CONTROL          (0x00)
#define TSL2561_REGISTER_TIMING           (0x01)
#define TSL2561_REGISTER_THRESHHOLDL_LOW  (0x02)
#define TSL2561_REGISTER_THRESHHOLDL_HIGH (0x03)
#define TSL2561_REGISTER_THRESHHOLDH_LOW  (0x04)
#define TSL2561_REGISTER_THRESHHOLDH_HIGH (0x05)
#define TSL2561_REGISTER_INTERRUPT        (0x06)
#define TSL2561_REGISTER_CRC              (0x08)
#define TSL2561_REGISTER_ID               (0x0A)
#define TSL2561_REGISTER_CHAN0_LOW        (0x0C)
#define TSL2561_REGISTER_CHAN0_HIGH       (0x0D)
#define TSL2561_REGISTER_CHAN1_LOW        (0x0E)
#define TSL2561_REGISTER_CHAN1_HIGH       (0x0F)

#define TSL2561_VISIBLE                   (2)     /* Channel 0 - channel 1 */
#define TSL2561_INFRARED                  (1)     /* Channel 1 */
#define TSL2561_FULLSPECTRUM              (0)     /* Channel 0 */

#define TSL2561_CONTROL_POWERON           (0x03)
#define TSL2561_CONTROL_POWEROFF          (0x00)

#define TSL2561_INTEGRATIONTIME_13MS      (0x00)  /* 13.7ms */
#define TSL2561_INTEGRATIONTIME_101MS     (0x01)  /* 101ms */
#define TSL2561_INTEGRATIONTIME_402MS     (0x02)  /* 402ms */

#define TSL2561_GAIN_1X                   (0x00)  /* No gain */
#define TSL2561_GAIN_16X                  (0x10)  /* 16x gain */

#define TSL2561_COMMAND_BIT               (0x80)  /* Must be 1 */
#define TSL2561_CLEAR_BIT                 (0x40)  /* 1=Clear any pending int */
#define TSL2561_WORD_BIT                  (0x20)  /* 1=Read/write word */
#define TSL2561_BLOCK_BIT                 (0x10)  /* 1=Use block read/write */

#define TSL2561_CONTROL_POWERON           (0x03)
#define TSL2561_CONTROL_POWEROFF          (0x00)

static uint8_t _tsl2561_gain;
static uint8_t _tsl2561_integration_time;

int
tsl2561_enable(uint8_t state) {
    int rc;
    /* Enable the device by setting the control bit to 0x03 */
    rc = tsl2561_write8(TSL2561_COMMAND_BIT | TSL2561_REGISTER_CONTROL,
                        state ? TSL2561_CONTROL_POWERON : TSL2561_CONTROL_POWEROFF);
    return rc;
}

int
tsl2561_write8(uint8_t reg, uint32_t value) {
    int rc;

    uint8_t payload[2] = { reg, value & 0xFF };
    struct hal_i2c_master_data data_struct = {
      .address = MYNEWT_VAL(TSL2561_I2CADDR),
      .len = 2,
      .buffer = payload
    };

    rc = hal_i2c_master_write(0, &data_struct, OS_TICKS_PER_SEC / 10, 1);

    return rc;
}

int
tsl2561_read8(uint8_t reg, uint8_t *value) {
    int rc;
    uint8_t payload;

    struct hal_i2c_master_data data_struct = {
      .address = MYNEWT_VAL(TSL2561_I2CADDR),
      .len = 1,
      .buffer = &payload
    };

    /* Register write */
    payload = reg;
    rc = hal_i2c_master_write(0, &data_struct, OS_TICKS_PER_SEC / 10, 1);
    if (rc) {
        return rc;
    }

    /* Read one byte back */
    payload = 0;
    rc = hal_i2c_master_read(0, &data_struct, OS_TICKS_PER_SEC / 10, 1);
    *value = payload;

    return rc;
}

int
tsl2561_read16(uint8_t reg, uint16_t *value) {
    int rc;
    uint8_t payload[2] = { reg, 0 };

    struct hal_i2c_master_data data_struct = {
      .address = MYNEWT_VAL(TSL2561_I2CADDR),
      .len = 1,
      .buffer = payload
    };

    /* Register write */
    rc = hal_i2c_master_write(0, &data_struct, OS_TICKS_PER_SEC / 10, 1);
    if (rc) {
        return rc;
    }

    /* Read two bytes back */
    memset(payload, 0, 2);
    data_struct.len = 2;
    rc = hal_i2c_master_read(0, &data_struct, OS_TICKS_PER_SEC / 10, 1);
    *value = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);

    /* ToDo: Log raw reads */
    // console_printf("0x%04X\n", (uint16_t)payload[0] | ((uint16_t)payload[1] << 8));

    return rc;
}

int
tsl2561_get_data(uint16_t *broadband, uint16_t *ir) {
    int rc;
    int delay_ticks;

    tsl2561_enable(1);

    /* Wait integration time ms before getting a data sample */
    /* ToDo: Find a more efficient and accurate solution */
    switch (_tsl2561_integration_time) {
        case TSL2561_INTEGRATIONTIME_13MS:
            delay_ticks = OS_TICKS_PER_SEC / 75;
        break;
        case TSL2561_INTEGRATIONTIME_101MS:
            delay_ticks = OS_TICKS_PER_SEC / 9;
        break;
        case TSL2561_INTEGRATIONTIME_402MS:
        default:
            delay_ticks = OS_TICKS_PER_SEC / 2;
        break;
    }
    os_time_delay(delay_ticks);

    *broadband = *ir = 0;
    rc = tsl2561_read16(TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN0_LOW,
                        broadband);
    assert(rc == 0);
    rc = tsl2561_read16(TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN1_LOW,
                        ir);
    assert(rc == 0);

    tsl2561_enable(0);

    return rc;
}

int
tsl2561_set_integration_time(uint8_t int_time) {
    int rc;

    tsl2561_enable(1);
    rc = tsl2561_write8(TSL2561_COMMAND_BIT | TSL2561_REGISTER_TIMING,
                        _tsl2561_integration_time | _tsl2561_gain);
    assert(rc == 0);
    _tsl2561_integration_time = int_time;
    tsl2561_enable(0);

    return rc;
}

uint8_t
tsl2561_get_integration_time(void) {
    return _tsl2561_integration_time;
}

int
tsl2561_set_gain(uint8_t gain) {
    int rc;

    tsl2561_enable(1);
    rc = tsl2561_write8(TSL2561_COMMAND_BIT | TSL2561_REGISTER_TIMING,
                        _tsl2561_integration_time | _tsl2561_gain);
    assert(rc == 0);
    _tsl2561_gain = gain;
    tsl2561_enable(0);

    return rc;
}

uint8_t
tsl2561_get_gain(void) {
    return _tsl2561_gain;
}

#if MYNEWT_VAL(TSL2561_CLI)
static int tsl2561_shell_cmd(int argc, char **argv);

static struct shell_cmd tsl2561_shell_cmd_struct = {
    .sc_cmd = "tsl2561",
    .sc_cmd_func = tsl2561_shell_cmd
};

static int
tsl2561_shell_cmd(int argc, char **argv) {
    int rc;
    uint16_t full;
    uint16_t ir;

    /* Get a new data sample */
    rc = tsl2561_get_data(&full, &ir);
    console_printf("Gain:  %u\n", tsl2561_get_gain());
    console_printf("ITime: 0x%02X\n", tsl2561_get_integration_time());
    console_printf("Full:  %u\n", full);
    console_printf("IR:    %u\n", ir);

    return rc;
}
#endif

void
tsl2561_init(void) {
    int rc;

#if !MYNEWT_VAL(TSL2561_TASK)
    return;
#endif

#if MYNEWT_VAL(TSL2561_CLI)
    rc = shell_cmd_register(&tsl2561_shell_cmd_struct);
    SYSINIT_PANIC_ASSERT(rc == 0);
#endif

    tsl2561_set_gain(TSL2561_GAIN_1X);
    tsl2561_set_integration_time(TSL2561_INTEGRATIONTIME_13MS);

    /* Disable the device by default to save power */
    rc = tsl2561_enable(0);
    SYSINIT_PANIC_ASSERT(rc == 0);
}
