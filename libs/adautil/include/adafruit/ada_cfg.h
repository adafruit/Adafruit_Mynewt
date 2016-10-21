/**************************************************************************/
/*!
    @file     ada_cfg.h
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
#ifndef ADA_CFG_H_
#define ADA_CFG_H_

#if defined (NFFS_PRESENT) && defined(FS_PRESENT)

#include <config/config.h>

#define ADACFG_DIR        "/cfg"
#define ADACFG_FILE       "adafruit"

#ifndef CFG_ADACFG_FILE
#define CFG_ADACFG_FILE   ADACFG_DIR "/" ADACFG_FILE
#endif

#ifndef CFG_ADACFG_MAXCONFIG
#define CFG_ADACFG_MAXCONFIG 10
#endif

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct
{
  const char* name;
  enum conf_type type;
  uint16_t len;
  void* value;
} adacfg_info_t;

int adacfg_init(const char* prefix);
int adacfg_add(const adacfg_info_t* cfg);


#ifdef __cplusplus
 }
#endif

#endif

#endif /* ADA_CFG_H_ */
