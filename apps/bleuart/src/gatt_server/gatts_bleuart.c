/**************************************************************************/
/*!
    @file     gatts_bleuart.c
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

#include "gatts_bleuart.h"
#include "fifo/fifo.h"

#include "bleprph.h"

#define BLEUART_UUID16_RXD  0x0002
#define BLEUART_UUID16_TXD  0x0003

FIFO_DEF(bleuart_ffout, CFG_BLE_UART_TX_BUFSIZE, char, false);
FIFO_DEF(bleuart_ffin , CFG_BLE_UART_RX_BUFSIZE, char, true );

static char bleuart_xact_buf[64];

struct
{
  uint16_t txd_attr_hdl;
}_bleuart;
//struct os_eventq bleuart_evq;

uint16_t uuid_extract_128_to_16(uint8_t uuid128[])
{
  return le16toh(uuid128 + 12);
}

bool uuid_128_equal(uint8_t uuid1[], uint8_t uuid2[])
{
  return !memcmp(uuid1, uuid2, 16);
}

int gatts_bleuart_char_access(uint16_t conn_handle, uint16_t attr_handle, uint8_t op, union ble_gatt_access_ctxt *ctxt, void *arg)
{
  uint16_t uuid16 = uuid_extract_128_to_16(ctxt->chr_access.chr->uuid128);

  switch (uuid16)
  {
    case BLEUART_UUID16_RXD:
      assert(op == BLE_GATT_ACCESS_OP_WRITE_CHR);
      BLEPRPH_LOG(DEBUG, "RXD attr_handle=%d\n", attr_handle);
      fifo_write_n(bleuart_ffin, ctxt->chr_access.data, ctxt->chr_access.len);
    break;

    case BLEUART_UUID16_TXD:
//      assert(op == BLE_GATT_ACCESS_OP_READ_CHR);
//      BLEPRPH_LOG(DEBUG, "TXD attr_handle=%d\n", attr_handle);
//      int len = fifo_read_n(bleuart_ffout, bleuart_xact_buf, sizeof(bleuart_xact_buf));
//
//      if (len > 0) ctxt->chr_access.data = bleuart_xact_buf;
//      ctxt->chr_access.len  = len;
    break;

    default:
      assert(0);
    break;
  }

  return 0;
}

err_t gatts_bleuart_init(void)
{
  varclr(_bleuart);

//  return (err_t) eventq_init(&bleuart_evq);
//ble_gattc_notify_custom
  return ERROR_NONE;
}

void gatts_bleuart_register_cb(uint8_t op, union ble_gatt_register_ctxt *ctxt)
{
  switch (op)
  {
    case BLE_GATT_REGISTER_OP_SVC:
//      ctxt->svc_reg.svc->uuid128;
//      ctxt->svc_reg.handle;
    break;

    case BLE_GATT_REGISTER_OP_CHR:
      if ( uuid_128_equal(ctxt->chr_reg.chr->uuid128, (uint8_t []) BLEUART_CHAR_TX_UUID) )
      {
        _bleuart.txd_attr_hdl = ctxt->chr_reg.val_handle;
//      ctxt->chr_reg.def_handle;
      }
    break;

    case BLE_GATT_REGISTER_OP_DSC:
//      ctxt->dsc_reg.dsc->uuid128;
//      ctxt->dsc_reg.dsc_handle;
//      ctxt->dsc_reg.chr_def_handle;
    break;

    default: break;
  }
}

extern uint16_t conn_handle;
int gatts_bleuart_putc(char ch)
{
//  return (ERROR_NONE == ble_att_svr_write_local(_bleuart.txd_attr_hdl, &ch, 1)) ? 1 : 0;
  return (ERROR_NONE == ble_gattc_notify_custom(conn_handle, _bleuart.txd_attr_hdl, &ch, 1)) ? 1 : 0;
}

int gatts_bleuart_getc(void)
{
  char ch;
  return fifo_read(bleuart_ffin, &ch) ? ch : EOF;
}
