/******************************************************************************/
/*!
    @file     common_header.h
    @author   hathach

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2013, K. Townsend (microBuilder.eu)
    Copyright (c) 2014, Adafruit Industries (adafruit.com)

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
*/
/******************************************************************************/

#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef __cplusplus
 extern "C" {
#endif

//--------------------------------------------------------------------+
// INCLUDES
//--------------------------------------------------------------------+

//------------- Standard Header -------------//
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

////------------- General Header -------------//
#include "compiler.h"
#include "projectconfig.h"
#include "bluefruit_errors.h"
#include "assertion.h"
//#include "binary.h"
#include "common_func.h"
//#include "utilities.h"
//
////#include "nvm.h"
//
////------------- MCU header -------------//
//#include "nrf.h"
//#include "nrf_sdm.h"
//#include "nrf_delay.h"
//#include "app_timer.h"

//--------------------------------------------------------------------+
// Crossworks for ARM _file declaration
//--------------------------------------------------------------------+
#ifdef __CROSSWORKS_ARM
#include <__cross_studio_io.h>

struct __RAL_FILE
{
  int _file;
};
#endif

//--------------------------------------------------------------------+
// TYPEDEFS
//--------------------------------------------------------------------+
//typedef unsigned char byte_t;
//typedef float  float32_t;
//typedef double float64_t;



#ifdef __cplusplus
 }
#endif

#endif /* _COMMON_HEADER_H_ */
