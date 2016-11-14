/**************************************************************************/
/*!
    @file     timeout_timer.h
    @author   hathach (tinyusb.org)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2013, hathach (tinyusb.org)
    Copyright (c) 2015, Adafruit Industries (adafruit.com)

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
    INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    INCLUDING NEGLIGENCE OR OTHERWISE ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    This file is part of the tinyusb stack.
*/
/**************************************************************************/

#ifndef _TIMEOUT_TTIMER_H_
#define _TIMEOUT_TTIMER_H_

#include "os/os.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  uint32_t start;    ///< millisecond
  uint32_t interval; ///< in milliseconds
}timeout_t;

static inline uint32_t tick2ms(os_time_t tick) ATTR_ALWAYS_INLINE;
static inline uint32_t tick2ms(os_time_t tick)
{
  return ((uint64_t) (tick*1000)) / OS_TICKS_PER_SEC;
}

static inline void timeout_set(timeout_t* tt, uint32_t msec) ATTR_ALWAYS_INLINE;
static inline void timeout_set(timeout_t* tt, uint32_t msec)
{
  tt->interval = msec;
  tt->start    = tick2ms( os_time_get() );
}

static inline bool timeout_expired(timeout_t* tt) ATTR_ALWAYS_INLINE;
static inline bool timeout_expired(timeout_t* tt)
{
  return ( tick2ms( os_time_get() ) - tt->start ) >= tt->interval;
}

static inline void timeout_reset(timeout_t* tt) ATTR_ALWAYS_INLINE;
static inline void timeout_reset(timeout_t* tt)
{
  tt->start = tick2ms( os_time_get() );
}

static inline void timeout_periodic_reset(timeout_t* tt) ATTR_ALWAYS_INLINE;
static inline void timeout_periodic_reset(timeout_t* tt)
{
  tt->start += tt->interval;
}

// Using the GCC's Extension: "Statement Expression"
// Blocking wait until condition or timeout.
// return true if "condition" occurred, otherwise timed out.
#define timeout_wait_until(condition, timeout_ms, attempt_intveral) \
  ({\
    uint8_t _tm = timeout_ms;\
    while ( !(condition) && _tm-- ) {\
      if ( attempt_intveral ) os_time_delay(attempt_intveral);\
    }\
    _tm > 0;\
  })


#ifdef __cplusplus
 }
#endif

#endif /* _TIMEOUT_TTIMER_H_ */
