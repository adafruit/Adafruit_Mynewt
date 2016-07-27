/******************************************************************************/
/*!
    @file     assertion.h
    @author   hathach

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2013, K. Townsend (microBuilder.eu)
    Copyright (c) 2015, Adafruit Industries (adafruit.com)
    Copyright (c) 2016, hathach (tinyusb.org)

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

#ifndef _ASSERTION_H_
#define _ASSERTION_H_

#include "projectconfig.h"

#ifdef __cplusplus
extern "C"
{
#endif

//--------------------------------------------------------------------+
// Compile-time Assert
//--------------------------------------------------------------------+
#if defined __COUNTER__ && __COUNTER__ != __COUNTER__
  #define _ASSERT_COUNTER __COUNTER__
#else
  #define _ASSERT_COUNTER __LINE__
#endif

#define ASSERT_STATIC(const_expr, message) enum { XSTRING_CONCAT_(static_assert_, _ASSERT_COUNTER) = 1/(!!(const_expr)) }

//--------------------------------------------------------------------+
// Assert Helper
//--------------------------------------------------------------------+
#if CFG_DEBUG >= 1
//  #define ASSERT_MESSAGE(format, ...) fprintf(stderr, "[%08ld] %s: %d: " format "\r\n", get_millis(), __func__, __LINE__, __VA_ARGS__)
//  #define ASSERT_MESSAGE(format, ...) printf("[%08ld] %s: %d: " format "\r\n", get_millis(), __func__, __LINE__, __VA_ARGS__)
  #define ASSERT_MESSAGE(format, ...) fprintf(stderr, "%s: %d: " format "\r\n", __func__, __LINE__, __VA_ARGS__)
#else
  #define ASSERT_MESSAGE(format, ...)
#endif

#define VOID_RETURN

#define ASSERT_DEFINE(_setup_statement, _condition, _error, _format, ...) \
  do{\
    _setup_statement;\
    if (!(_condition)) {\
      ASSERT_MESSAGE(_format, __VA_ARGS__);\
      return _error;\
    }\
  }while(0)

#define VERIFY_DEFINE(_setup_statement, _condition, _error) \
  do{\
    _setup_statement;\
    if (!(_condition)) return _error;\
  }while(0)

//--------------------------------------------------------------------+
// Status Assert
//--------------------------------------------------------------------+
#define err_t int
#define ASSERT_STATUS(sts) \
    ASSERT_DEFINE(err_t _status = (err_t)(sts),\
                  0 == _status, _status, "error = %d", _status)

#define ASSERT_STATUS_RETVOID(sts) \
    ASSERT_DEFINE(err_t _status = (err_t)(sts),\
                  0 == _status, VOID_RETURN, "error = %d", _status)

#define VERIFY_STATUS(sts)  \
    VERIFY_DEFINE(err_t _status = (err_t)(sts), 0 == _status, _status)

#define VERIFY_STATUS_RETVOID(sts)  \
    VERIFY_DEFINE(err_t _status = (err_t)(sts), 0 == _status, VOID_RETURN)

//--------------------------------------------------------------------+
// Logical Assert
//--------------------------------------------------------------------+
#define ASSERT(_condition, _error)       ASSERT_DEFINE( , _condition, _error, "%s", "false")
#define VERIFY(_condition, _error)       VERIFY_DEFINE( , _condition, _error)

//--------------------------------------------------------------------+
// Pointer Assert
//--------------------------------------------------------------------+
#define ASSERT_PTR(_ptr, _error )        ASSERT_DEFINE( , NULL != (_ptr), _error, "%s", "pointer is NULL")
#define VERIY_PTR(_ptr, _error )         VERIFY_DEFINE( , NULL != (_ptr), _error)

#ifdef __cplusplus
}
#endif

#endif /* _ASSERTION_H_ */
