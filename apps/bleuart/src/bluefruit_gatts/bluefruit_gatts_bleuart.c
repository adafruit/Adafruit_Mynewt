/**************************************************************************/
/*!
    @file     bluefruit_gatts_bleuart.c
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

#include "bluefruit_gatts.h"
#include "fifo/fifo.h"

//--------------------------------------------------------------------+
//
//--------------------------------------------------------------------+
#define BLEUART_UUID16_RXD  0x0002
#define BLEUART_UUID16_TXD  0x0003

FIFO_DEF(bleuart_ffout, CFG_BLE_UART_TX_BUFSIZE, char, false);
FIFO_DEF(bleuart_ffin , CFG_BLE_UART_RX_BUFSIZE, char, true );

int bf_gatts_bleuart_char_access(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);

//--------------------------------------------------------------------+
//
//--------------------------------------------------------------------+
static const struct ble_gatt_svc_def _gatts_bleuart =
{
    .type            = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid128         = (uint8_t [])BLEUART_SERVICE_UUID,
    .characteristics = (struct ble_gatt_chr_def[])
    {
      { /*** Characteristic: TXD */
        .uuid128   = (uint8_t []) BLEUART_CHAR_TX_UUID,
        .access_cb = bf_gatts_bleuart_char_access,
        .flags     = BLE_GATT_CHR_F_NOTIFY,
      },
      { /*** Characteristic: RXD. */
        .uuid128   = (uint8_t []) BLEUART_CHAR_RX_UUID,
        .access_cb = bf_gatts_bleuart_char_access,
        .flags     = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
      },
      { 0 } /* No more characteristics in this service. */
    }
};

static struct
{
  uint16_t txd_attr_hdl;
}_bleuart;

//--------------------------------------------------------------------+
//
//--------------------------------------------------------------------+
uint16_t uuid_extract_128_to_16(uint8_t const uuid128[])
{
  return le16toh(uuid128 + 12);
}

bool uuid_128_equal(uint8_t const uuid1[], uint8_t const uuid2[])
{
  return !memcmp(uuid1, uuid2, 16);
}

//--------------------------------------------------------------------+
//
//--------------------------------------------------------------------+
int bf_gatts_bleuart_init(struct ble_hs_cfg *cfg)
{
  varclr(_bleuart);

  return ble_gatts_count_cfg(&_gatts_bleuart, cfg);;
}

int bf_gatts_bleuart_register(void)
{
  int result = ble_gatts_register_svcs(&_gatts_bleuart, NULL, NULL);

  ble_gatts_find_chr(_gatts_bleuart.uuid128, _gatts_bleuart.characteristics[0].uuid128, NULL, &_bleuart.txd_attr_hdl);

  return result;
}

extern uint16_t conn_handle;
int bf_gatts_bleuart_putc(char ch)
{
  return (ERROR_NONE == ble_gattc_notify_custom(conn_handle, _bleuart.txd_attr_hdl, &ch, 1)) ? 1 : 0;
}

int bf_gatts_bleuart_getc(void)
{
  char ch;
  return fifo_read(bleuart_ffin, &ch) ? ch : EOF;
}

//--------------------------------------------------------------------+
//
//--------------------------------------------------------------------+
int bf_gatts_bleuart_char_access(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
  uint16_t uuid16 = uuid_extract_128_to_16(ctxt->chr->uuid128);

  switch (uuid16)
  {
    case BLEUART_UUID16_RXD:
      assert(ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR);
      fifo_write_n(bleuart_ffin, ctxt->att->write.data, ctxt->att->write.len);
    break;

    case BLEUART_UUID16_TXD:
//      assert(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR);
//      int len = fifo_read_n(bleuart_ffout, bleuart_xact_buf, sizeof(bleuart_xact_buf));
//
//      if (len > 0) ctxt->att.read.data = bleuart_xact_buf;
//      ctxt->att.read.len  = len;
    break;

    default:
      assert(0);
    break;
  }

  return 0;
}

//--------------------------------------------------------------------+
//
//--------------------------------------------------------------------+
//void bf_gatts_bleuart_register_cb(struct ble_gatt_register_ctxt *ctxt)
//{
//  switch (ctxt->op)
//  {
//    case BLE_GATT_REGISTER_OP_SVC:
////      ctxt->svc_reg.svc->uuid128;
////      ctxt->svc_reg.handle;
//    break;
//
//    case BLE_GATT_REGISTER_OP_CHR:
//      if ( uuid_128_equal(ctxt->chr.chr_def->uuid128, (uint8_t []) BLEUART_CHAR_TX_UUID) )
//      {
//        _bleuart.txd_attr_hdl = ctxt->chr.val_handle;
////      ctxt->chr_reg.def_handle;
//      }
//    break;
//
//    case BLE_GATT_REGISTER_OP_DSC:
////      ctxt->dsc_reg.dsc->uuid128;
////      ctxt->dsc_reg.dsc_handle;
////      ctxt->dsc_reg.chr_def_handle;
//    break;
//
//    default: break;
//  }
//}

