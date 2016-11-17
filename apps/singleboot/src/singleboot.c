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
/* Minimal interval in ms that DFU pin must be pressed to go into DFU mode */
#define BOOTLOADER_BUTTON_HOLDING_INTERVAL 500

#define BLINKY_PERIOD               125000

struct hal_timer _blinky_timer;

void 
blinky_isr(void *arg)
{
  hal_gpio_toggle(LED_BLINK_PIN);

  os_cputime_timer_relative(&_blinky_timer, BLINKY_PERIOD);
}

void
start_boot_serial_mode(void)
{
  /* Set up fast blinky for indicator using hw timer */
  os_cputime_timer_init(&_blinky_timer, blinky_isr, NULL);
  os_cputime_timer_relative(&_blinky_timer, BLINKY_PERIOD);

  boot_serial_start(BOOT_SER_CONS_INPUT);
  assert(0);
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
    /* Check if Magic number is REST_TO_DFU or DOUBLE_RESET */
    if (BOOTLOADER_MAGIC_LOC ==  BOOTLOADER_RESET_TO_DFU_MAGIC)
    {
        start_boot_serial_mode();
    }

    /*
     * Configure a GPIO as input, and compare it against expected value.
     * If it matches, await for download commands from serial.
     */
    hal_gpio_init_in(BOOT_SERIAL_DETECT_PIN, BOOT_SERIAL_DETECT_PIN_CFG);
    if (hal_gpio_read(BOOT_SERIAL_DETECT_PIN) == BOOT_SERIAL_DETECT_PIN_VAL) {

        /* Double check the pin value after configured interval,
         * make sure the DFU pin hold long enough */
         os_cputime_delay_usecs( 1000*BOOTLOADER_BUTTON_HOLDING_INTERVAL );

        if (hal_gpio_read(BOOT_SERIAL_DETECT_PIN) == BOOT_SERIAL_DETECT_PIN_VAL) {
            start_boot_serial_mode();
        }
    }

#if MYNEWT_VAL(BOOT_DOUBLE_RESET_DFU)
    /* Double Reset  to DFU
     * - write dfu reset magic to location
     * - turn on LED
     * - delay 500ms, A reset happen within this delay will force to DFU
     * - Turn of led and clear double reset magic
     * */
    BOOTLOADER_MAGIC_LOC = BOOTLOADER_RESET_TO_DFU_MAGIC;
    LED_BLINK_ON();

    os_cputime_delay_usecs(500000);

    LED_BLINK_OFF();
    BOOTLOADER_MAGIC_LOC = 0;
#endif

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

    /* If image is not valid, go to DFU mode */
    uint8_t tempbuf[256];
    if ( bootutil_img_validate(&hdr, fa, tempbuf, 256, NULL, 0, NULL) )
    {
      start_boot_serial_mode();
    }

    hal_system_start((void *)(rsp.br_image_addr + rsp.br_hdr->ih_hdr_size));

    return 0;
}
