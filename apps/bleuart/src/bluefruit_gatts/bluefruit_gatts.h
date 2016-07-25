/**************************************************************************/
/*!
    @file     bluefruit_gatts.h
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

#ifndef _BLUEFRUIT_GATTS_H_
#define _BLUEFRUIT_GATTS_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "common_header.h"
#include "host/ble_hs.h"
#include "bluefruit_uuid.h"

typedef struct
{
  int  (* const init        ) (struct ble_hs_cfg *cfg);
  int  (* const register_svc) (void);
} bf_gatts_driver_t;

int bf_gatts_init(struct ble_hs_cfg *cfg);
int bf_gatts_register(void);

// TODO remove later
#include "log/log.h"
extern struct log bleprph_log;
#define BLEPRPH_LOG_MODULE  (LOG_MODULE_PERUSER + 0)
#define BLEPRPH_LOG(lvl, ...) LOG_ ## lvl(&bleprph_log, BLEPRPH_LOG_MODULE, __VA_ARGS__)

#include "bluefruit_gatts_dis.h"
#include "bluefruit_gatts_bleuart.h"

#ifdef __cplusplus
 }
#endif

#endif /* _BLUEFRUIT_GATTS_H_ */
