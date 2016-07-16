/**************************************************************************/
/*!
    @file     bluefruit_gatts_bleuart.h
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

#ifndef _BLUEFRUIT_GATTS_BLEUART_H_
#define _BLUEFRUIT_GATTS_BLEUART_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "common_header.h"
#include "host/ble_hs.h"

// UART Serivce: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
// UART RXD    : 6E400002-B5A3-F393-E0A9-E50E24DCCA9E
// UART TXD    : 6E400003-B5A3-F393-E0A9-E50E24DCCA9E

#define BLEUART_SERVICE_UUID  {0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E}
#define BLEUART_CHAR_RX_UUID  {0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E}
#define BLEUART_CHAR_TX_UUID  {0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E}

#define GATTS_BLEUART_SERVICE \
  {\
    .type = BLE_GATT_SVC_TYPE_PRIMARY,\
    .uuid128 = (uint8_t [])BLEUART_SERVICE_UUID,\
    .characteristics = (struct ble_gatt_chr_def[])\
    {\
      { /*** Characteristic: TXD */\
          .uuid128 = (uint8_t []) BLEUART_CHAR_TX_UUID,\
          .access_cb = gatts_bleuart_char_access,\
          .flags = BLE_GATT_CHR_F_NOTIFY,\
      }, {\
          /*** Characteristic: RXD. */\
          .uuid128 = (uint8_t []) BLEUART_CHAR_RX_UUID,\
          .access_cb = gatts_bleuart_char_access,\
          .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,\
      }, {\
          0, /* No more characteristics in this service. */\
      }\
    }\
  }\

int   gatts_bleuart_char_access(uint16_t conn_handle, uint16_t attr_handle, uint8_t op, union ble_gatt_access_ctxt *ctxt, void *arg);
err_t gatts_bleuart_init(void);
void  gatts_bleuart_register_cb(uint8_t op, union ble_gatt_register_ctxt *ctxt);

int gatts_bleuart_putc(char ch);
int gatts_bleuart_getc(void);


#ifdef __cplusplus
 }
#endif

#endif /* _BLUEFRUIT_GATTS_BLEUART_H_ */
