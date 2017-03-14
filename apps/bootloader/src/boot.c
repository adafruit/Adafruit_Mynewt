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

#include <hal/hal_bsp.h>
#include <hal/hal_system.h>
#include <hal/hal_flash.h>

#include <sysinit/sysinit.h>
#include <console/console.h>
#include "bootutil/image.h"
#include "bootutil/bootutil.h"
#include "boot_serial/boot_serial.h"

#define BOOT_AREA_DESC_MAX  (256)
#define AREA_DESC_MAX       (BOOT_AREA_DESC_MAX)

int
main(void)
{
    struct boot_rsp rsp;
    int rc;

    hal_bsp_init();
    sysinit();

#if 0
    /* Check if Magic number is REST_TO_DFU */
    if (BOOTLOADER_MAGIC_LOC ==  BOOTLOADER_RESET_TO_DFU_MAGIC)
    {
        start_boot_serial_mode();
    }
#endif

    /* Go on with normal boot progress */
    rc = boot_go(&rsp);

    /* No bootable image, go to boot_serial dfu mode
     * Does not work due to assert() somewhere in boot_go() */
    if ( rc )
    {
        const int BOOT_SERIAL_INPUT_MAX = 128;
        boot_serial_start(BOOT_SERIAL_INPUT_MAX);
        assert(0);
    }

    assert(rc == 0);

    hal_system_start((void *)(rsp.br_image_addr + rsp.br_hdr->ih_hdr_size));

    return 0;
}
