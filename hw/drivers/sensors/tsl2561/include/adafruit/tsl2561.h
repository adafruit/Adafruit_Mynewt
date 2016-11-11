/**************************************************************************/
/*!
    @file     tsl2561.h
    @author   ktown (Adafruit Industries)

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
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/

#ifndef __ADAFRUIT_TSL2561_H__
#define __ADAFRUIT_TSL2561_H__

#include <os/os.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the tls2561. This function is normally called by sysinit.
 */
void tsl2561_init (void);

/**
 * Gets a new data sample from the light sensor.
 *
 * @param broadband The full (visible + ir) sensor output
 * @param ir        The ir sensor output
 *
 * @return 0 on success, and non-zero error code on failure
 */
int tsl2561_get_data (uint16_t *broadband, uint16_t *ir);

/**
 * Sets the integration time used when sampling light values.
 *
 * @param int_time The integration time which can be one of:
 *                  - 0x00: 13ms
 *                  - 0x01: 101ms
 *                  - 0x02: 402ms
 *
 * @return 0 on success, and non-zero error code on failure
 */
int tsl2561_set_integration_time (uint8_t int_time);

/**
 * Gets the current integration time used when sampling light values.
 *
 * @return The integration time which can be one of:
 *         - 0x00: 13ms
 *         - 0x01: 101ms
 *         - 0x02: 402ms
 */
uint8_t tsl2561_get_integration_time (void);

/**
 * Sets the gain increment used when sampling light values.
 *
 * @param gain The gain increment which can be one of:
 *                  - 0x00: 1x (no gain)
 *                  - 0x10: 16x gain
 *
 * @return 0 on success, and non-zero error code on failure
 */
int tsl2561_set_gain (uint8_t gain);

/**
 * Gets the current gain increment used when sampling light values.
 *
 * @return The gain increment which can be one of:
 *         - 0x00: 1x (no gain)
 *         - 0x10: 16x gain
 */
uint8_t tsl2561_get_gain (void);

#ifdef __cplusplus
}
#endif

#endif /* __ADAFRUIT_TSL2561_H__ */
