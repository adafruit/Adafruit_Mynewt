/**************************************************************************/
/*!
    @file     bluefruit_gatts.c
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

//--------------------------------------------------------------------+
// FUNCTION PROTOTYPES
//--------------------------------------------------------------------+
static void gatt_svr_register_cb(uint8_t op, union ble_gatt_register_ctxt *ctxt, void *arg);
static int  gatt_svr_chr_access_gatt(uint16_t conn_handle, uint16_t attr_handle, uint8_t op, union ble_gatt_access_ctxt *ctxt, void *arg);


//--------------------------------------------------------------------+
// VARIABLES
//--------------------------------------------------------------------+
uint8_t bf_gatts_service_changed[4];

static const struct ble_gatt_svc_def gatt_svr_svcs[] =
{
   /*** Service: GAP. */
  BLUEFRUIT_GATTS_GAP_SERVICE,

  /*** Service: GATT */
  {
      .type = BLE_GATT_SVC_TYPE_PRIMARY,
      .uuid128 = BLE_UUID16(BLE_GATT_SVC_UUID16),
      .characteristics = (struct ble_gatt_chr_def[])
      {
        {
          .uuid128 = BLE_UUID16(BLE_GATT_CHR_SERVICE_CHANGED_UUID16),
          .access_cb = gatt_svr_chr_access_gatt,
          .flags = BLE_GATT_CHR_F_INDICATE,
        },
        /* No more characteristics in this service. */
        { 0 }
      },
  },

  // BLE UART
  BLUEFRUIT_GATTS_BLEUART_SERVICE,

  /* No more services. */
  { 0 },
};


//--------------------------------------------------------------------+
// PUBLIC API
//--------------------------------------------------------------------+
err_t bf_gatts_init(void)
{
  VERIFY_STATUS( ble_gatts_register_svcs(gatt_svr_svcs, gatt_svr_register_cb, NULL) );

  return ERROR_NONE;
}


//--------------------------------------------------------------------+
// INTERNAL FUNCTION
//--------------------------------------------------------------------+
static char * gatt_svr_uuid128_to_s(void *uuid128, char *dst)
{
    uint16_t uuid16;
    uint8_t *u8p;

    uuid16 = ble_uuid_128_to_16(uuid128);
    if (uuid16 != 0) {
        sprintf(dst, "0x%04x", uuid16);
        return dst;
    }

    u8p = uuid128;

    sprintf(dst,      "%02x%02x%02x%02x-", u8p[15], u8p[14], u8p[13], u8p[12]);
    sprintf(dst + 9,  "%02x%02x-%02x%02x-", u8p[11], u8p[10], u8p[9], u8p[8]);
    sprintf(dst + 19, "%02x%02x%02x%02x%02x%02x%02x%02x",
            u8p[7], u8p[6], u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);

    return dst;
}

static void gatt_svr_register_cb(uint8_t op, union ble_gatt_register_ctxt *ctxt, void *arg)
{
  char buf[40];
  (void) buf;

  bf_gatts_bleuart_register_cb(op, ctxt);

  switch (op)
  {
    case BLE_GATT_REGISTER_OP_SVC:
      BLEPRPH_LOG(DEBUG, "registered service %s with handle=%d\n", gatt_svr_uuid128_to_s(ctxt->svc_reg.svc->uuid128, buf), ctxt->svc_reg.handle);
    break;

    case BLE_GATT_REGISTER_OP_CHR:
      BLEPRPH_LOG(DEBUG, "registering characteristic %s with "
                  "def_handle=%d val_handle=%d\n",
                  gatt_svr_uuid128_to_s(ctxt->chr_reg.chr->uuid128, buf), ctxt->chr_reg.def_handle, ctxt->chr_reg.val_handle);
    break;

    case BLE_GATT_REGISTER_OP_DSC:
      BLEPRPH_LOG(DEBUG, "registering descriptor %s with handle=%d "
                  "chr_handle=%d\n",
                  gatt_svr_uuid128_to_s(ctxt->dsc_reg.dsc->uuid128, buf), ctxt->dsc_reg.dsc_handle, ctxt->dsc_reg.chr_def_handle);
    break;

    default: break;
  }
}

static int gatt_svr_chr_access_gatt(uint16_t conn_handle, uint16_t attr_handle, uint8_t op, union ble_gatt_access_ctxt *ctxt, void *arg)
{
  uint16_t uuid16 = ble_uuid_128_to_16(ctxt->chr_access.chr->uuid128);

  switch (uuid16)
  {
    case BLE_GATT_CHR_SERVICE_CHANGED_UUID16:
      if (op == BLE_GATT_ACCESS_OP_WRITE_CHR)
      {
        if (ctxt->chr_access.len != sizeof(bf_gatts_service_changed))
        {
          return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }
        memcpy(bf_gatts_service_changed, ctxt->chr_access.data, sizeof(bf_gatts_service_changed));
      }
      else if (op == BLE_GATT_ACCESS_OP_READ_CHR)
      {
        ctxt->chr_access.data = (void *) &bf_gatts_service_changed;
        ctxt->chr_access.len = sizeof(bf_gatts_service_changed);
      }
    break;

    default: break;
  }

  return 0;
}

