/**************************************************************************/
/*!
    @file     bledis.c
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

#include "adafruit_util.h"
#include "host/ble_hs.h"
#include "bledis/bledis.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF
//--------------------------------------------------------------------+
enum { BLEDIS_MAX_CHAR = sizeof(bledis_cfg_t)/4 };

//--------------------------------------------------------------------+
// VARIABLE DECLARATION
//--------------------------------------------------------------------+
static union
{
  bledis_cfg_t named;
  const char * arrptr[BLEDIS_MAX_CHAR];
}_dis_cfg;

static struct ble_gatt_chr_def _dis_chars[BLEDIS_MAX_CHAR+1];

static const struct ble_gatt_svc_def _dis_service[] =
{
  {
      .type    = BLE_GATT_SVC_TYPE_PRIMARY,
      .uuid128 = BLE_UUID16(UUID16_SVC_DEVICE_INFORMATION),
      .characteristics = _dis_chars
  }

  , { 0 } /* No more services. */
};

//--------------------------------------------------------------------+
// FUNCTION DECLARATION
//--------------------------------------------------------------------+
static int bledis_access_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);


int bledis_init(struct ble_hs_cfg * ble_cfg, bledis_cfg_t const * dis_cfg)
{
  memclr(_dis_chars, sizeof(_dis_chars));
  _dis_cfg.named = *dis_cfg;

  // Include only configured characteristics
  int count = 0;
  for (int i=0; i<BLEDIS_MAX_CHAR; i++)
  {
    if ( _dis_cfg.arrptr[i] != NULL )
    {
      _dis_chars[count].uuid128   = BLE_UUID16(UUID16_CHR_MODEL_NUMBER_STRING + i);
      _dis_chars[count].access_cb = bledis_access_cb;
      _dis_chars[count].flags     = BLE_GATT_CHR_F_READ;

      count++;
    }
  }

  // Register Service
  ble_gatts_count_cfg(_dis_service, ble_cfg);
  ble_gatts_add_svcs (_dis_service);

  return 0;
}


int bledis_access_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
  VERIFY(ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR, -1);

  uint16_t uuid16 = ble_uuid_128_to_16(ctxt->chr->uuid128);
  VERIFY( is_within(UUID16_CHR_MODEL_NUMBER_STRING, uuid16, UUID16_CHR_MANUFACTURER_NAME_STRING), -1);

  const char* str = _dis_cfg.arrptr[uuid16 - UUID16_CHR_MODEL_NUMBER_STRING];

  os_mbuf_append(ctxt->om, str, strlen(str));

  return 0;
}
