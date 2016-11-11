/**************************************************************************/
/*!
    @file     bleuart.h
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

#ifndef _ADAFRUIT_BLEUART_H_
#define _ADAFRUIT_BLEUART_H_

/*------------------------------------------------------------------*/
/* Configuration is done by syscfg.yml in application folder
 * - BLEUART_BUFSIZE: Size of RXD fifo (default 128)
 * - BLEUART_CLI    : Enable the use of shell to send/receive bleuart
 *------------------------------------------------------------------*/

#ifdef __cplusplus
 extern "C" {
#endif

#include "adafruit/adautil.h"
#include "host/ble_hs.h"

extern const uint8_t BLEUART_UUID_SERVICE[16];
extern const uint8_t BLEUART_UUID_CHR_RXD[16];
extern const uint8_t BLEUART_UUID_CHR_TXD[16];

int  bleuart_init(void);
void bleuart_set_conn_handle(uint16_t conn_handle);

int bleuart_write(void const* buffer, uint32_t size);

static inline int bleuart_putc(char ch)
{
  return bleuart_write(&ch, 1);
}

static inline int bleuart_puts(const char* str)
{
  return bleuart_write(str, strlen(str));
}

int bleuart_read(uint8_t* buffer, uint32_t size);
int bleuart_getc(void);

#ifdef __cplusplus
 }
#endif

#endif /* _ADAFRUIT_BLEUART_H_ */
