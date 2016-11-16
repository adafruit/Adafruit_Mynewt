/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <assert.h>
#include <stddef.h>
#include <inttypes.h>
#include "syscfg/syscfg.h"
#include <flash_map/flash_map.h>
#include <os/os.h>
#include <bsp/bsp.h>
#include <hal/hal_bsp.h>
#include <hal/hal_system.h>
#include <hal/hal_flash.h>
#include <os/os_cputime.h>

#if MYNEWT_VAL(BOOT_SERIAL)
#include <hal/hal_gpio.h>
#include <boot_serial/boot_serial.h>
#include <sysinit/sysinit.h>
#endif
#include <console/console.h>
#include "bootutil/image.h"
#include "bootutil/bootutil.h"

#define BOOT_AREA_DESC_MAX  (256)
#define AREA_DESC_MAX       (BOOT_AREA_DESC_MAX)

#if MYNEWT_VAL(BOOT_SERIAL)
#define BOOT_SER_CONS_INPUT         128
#endif

#define BLINKY_PERIOD               125000

struct hal_timer _blinky_timer;

void 
blinky_isr(void *arg)
{
  hal_gpio_toggle(LED_BLINK_PIN);

  os_cputime_timer_relative(&_blinky_timer, BLINKY_PERIOD);
}

int
main(void)
{
    struct boot_rsp rsp;
    int rc;

#if MYNEWT_VAL(BOOT_SERIAL)
    sysinit();
#else
    flash_map_init();
    hal_bsp_init();
#endif

#if MYNEWT_VAL(BOOT_SERIAL)
    /*
     * Configure a GPIO as input, and compare it against expected value.
     * If it matches, await for download commands from serial.
     */
    hal_gpio_init_in(BOOT_SERIAL_DETECT_PIN, BOOT_SERIAL_DETECT_PIN_CFG);
    if (hal_gpio_read(BOOT_SERIAL_DETECT_PIN) == BOOT_SERIAL_DETECT_PIN_VAL) {

        /* Set up fast blinky for indicator using hw timer */
        os_cputime_timer_init(&_blinky_timer, blinky_isr, NULL);
        os_cputime_timer_relative(&_blinky_timer, BLINKY_PERIOD);

        boot_serial_start(BOOT_SER_CONS_INPUT);
        assert(0);
    }
#endif

    /* single boot just boot from the first image slot */
    const struct flash_area* fa;
    struct image_header hdr;

    uint8_t area_id = (uint8_t) flash_area_id_from_image_slot(0);

    rc = flash_area_open(area_id, &fa);
    assert(rc == 0);

    rc = flash_area_read(fa, 0, &hdr, sizeof(hdr));
    assert(rc == 0);

    flash_area_close(fa);

    rsp.br_flash_id = fa->fa_device_id;
    rsp.br_image_addr = fa->fa_off;
    rsp.br_hdr = &hdr;

    hal_system_start((void *)(rsp.br_image_addr + rsp.br_hdr->ih_hdr_size));

    return 0;
}
