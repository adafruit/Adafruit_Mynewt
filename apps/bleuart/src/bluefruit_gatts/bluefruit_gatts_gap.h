/**************************************************************************/
/*!
    @file     bluefruit_gatts_gap.h
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

#ifndef _BLUEFRUIT_GATTS_GAP_H_
#define _BLUEFRUIT_GATTS_GAP_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "common_header.h"
#include "host/ble_hs.h"

#define BLUEFRUIT_GATTS_GAP_SERVICE \
  {\
    .type = BLE_GATT_SVC_TYPE_PRIMARY,\
    .uuid128 = BLE_UUID16(BLE_GAP_SVC_UUID16),\
    .characteristics = (struct ble_gatt_chr_def[])\
    {\
      { /*** Characteristic: Device Name. */\
          .uuid128 = BLE_UUID16(BLE_GAP_CHR_UUID16_DEVICE_NAME),\
          .access_cb = bf_gatts_gap_char_access,\
          .flags = BLE_GATT_CHR_F_READ,\
      }, {\
          /*** Characteristic: Appearance. */\
          .uuid128 = BLE_UUID16(BLE_GAP_CHR_UUID16_APPEARANCE),\
          .access_cb = bf_gatts_gap_char_access,\
          .flags = BLE_GATT_CHR_F_READ,\
      }, {\
          /*** Characteristic: Peripheral Privacy Flag. */\
          .uuid128 = BLE_UUID16(BLE_GAP_CHR_UUID16_PERIPH_PRIV_FLAG),\
          .access_cb = bf_gatts_gap_char_access,\
          .flags = BLE_GATT_CHR_F_READ,\
      }, {\
          /*** Characteristic: Reconnection Address. */\
          .uuid128 = BLE_UUID16(BLE_GAP_CHR_UUID16_RECONNECT_ADDR),\
          .access_cb = bf_gatts_gap_char_access,\
          .flags = BLE_GATT_CHR_F_WRITE,\
      }, {\
          /*** Characteristic: Peripheral Preferred Connection Parameters. */\
          .uuid128 = BLE_UUID16(BLE_GAP_CHR_UUID16_PERIPH_PREF_CONN_PARAMS),\
          .access_cb = bf_gatts_gap_char_access,\
          .flags = BLE_GATT_CHR_F_READ,\
      }, {\
          0, /* No more characteristics in this service. */\
      }\
    }\
  }\


int   bf_gatts_gap_char_access(uint16_t conn_handle, uint16_t attr_handle, uint8_t op, union ble_gatt_access_ctxt *ctxt, void *arg);
err_t bf_gatts_gap_init(void);

#ifdef __cplusplus
 }
#endif

#endif /* _BLUEFRUIT_GATTS_GAP_H_ */
