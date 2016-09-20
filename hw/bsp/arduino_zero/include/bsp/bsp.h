/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#ifndef __ARDUINO_BSP_H
#define __ARDUINO_BSP_H

#if !defined(ARDUINO_ZERO_PRO) && !defined(ARDUINO_ZERO)
  #error you must defined arduino_zero_pro or arduino_zero in your target features 
#endif

#ifndef BSP_SYSID_H
#include <bsp/bsp_sysid.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


 
#define ARDUINO_ZERO_PIN_UART_RX (ARDUINO_ZERO_D0)    
#define ARDUINO_ZERO_PIN_UART_TX (ARDUINO_ZERO_D1)       
      
#define LED_BLINK_PIN   (ARDUINO_ZERO_D13)
#define CONSOLE_UART    (2)    

/* This defines the maximum NFFS areas (block) are in the BSPs NFS file 
 * system space.  This in conjunction with flash map determines how 
 * many NFS blocks there will be.  A minimum is the number of individually
 * erasable sectors in the flash area and the maximum is this number. If
 * your max is less than the number of sectors then the NFFS will combine
 * multiple sectors into an NFFS area */
#define NFFS_AREA_MAX    (8)
    
int bsp_imgr_current_slot(void);
#ifdef __cplusplus
}
#endif

#endif  /* __ARDUINO_BSP_H */
