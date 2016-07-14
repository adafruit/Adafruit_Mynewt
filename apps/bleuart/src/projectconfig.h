/**************************************************************************/
/*!
    @file     projectconfig.h

    @section LICENSE

    Software License Agreement (BSD License)

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
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/
#ifndef _PROJECTCONFIG_H_
#define _PROJECTCONFIG_H_

/*=========================================================================
    MCU & BOARD SELECTION
    -----------------------------------------------------------------------
    CFG_BOARD           Must be one of the value defined in board.h
    CFG_MCU_STRING      The text representation of the MCU used (for use
                        in commands like 'ATI' in the AT Parser, etc.)

    NOTE:               CFG_BOARD is in an 'ifndef' wrapper so that it can
                        be overridden at compile time by defining the target
                        board in the makefile.
    -----------------------------------------------------------------------*/
    #ifndef CFG_BOARD
      #define CFG_BOARD                                CFG_BOARD_PCA10040
    #endif

    #define CFG_MCU_STRING                             "nRF52832"
/*=========================================================================*/


/*=========================================================================
    FIRMWARE VERSION SETTINGS
    -----------------------------------------------------------------------
    This version number can be freely changed for your specific project
    -----------------------------------------------------------------------*/
    #define CFG_FIRMWARE_VERSION_MAJOR                 0
    #define CFG_FIRMWARE_VERSION_MINOR                 1
    #define CFG_FIRMWARE_VERSION_REVISION              0
    #define CFG_FIRMWARE_VERSION_STRING                XSTRING_(CFG_FIRMWARE_VERSION_MAJOR) "." \
                                                       XSTRING_(CFG_FIRMWARE_VERSION_MINOR) "." \
                                                       XSTRING_(CFG_FIRMWARE_VERSION_REVISION)
/*=========================================================================*/


/*=========================================================================
    DEBUG LEVEL
    -----------------------------------------------------------------------

    CFG_DEBUG             Level 3: Full debug output, any failed assert
                                   will produce a breakpoint for the
                                   debugger
                          Level 2: ATTR_ALWAYS_INLINE is null, ASSERT
                                   has text
                          Level 1: ATTR_ALWAYS_INLINE is an attribute,
                                   ASSERT has no text
                          Level 0: No debug information generated

    NOTE:                 'ifndef' to allow it to be overridden at compile time
    -----------------------------------------------------------------------*/
    #ifndef CFG_DEBUG
      #define CFG_DEBUG                                  (0)
    #endif

    #if (CFG_DEBUG > 3) || (CFG_DEBUG < 0)
      #error "CFG_DEBUG must be a value between 0 (no debug) and 3"
    #endif
/*=========================================================================*/


/*=========================================================================
    SEMIHOSTING DEBUG CONSOLE
    -----------------------------------------------------------------------
    CFG_PRINT_CWDEBUG       Enable Semihosting debug console
    -----------------------------------------------------------------------*/
    #define CFG_PRINT_CWDEBUG                        0
/*=========================================================================*/


/*=========================================================================
    UART
    -----------------------------------------------------------------------
    CFG_UART_BUFSIZE          The length in bytes of the UART RX FIFO. This
                              will determine the maximum number of received
                              characters to store in memory.
    -----------------------------------------------------------------------*/
    #define CFG_UART_BUFSIZE                           (128)
/*=========================================================================*/


/*=========================================================================
    AT COMMAND PARSER
    -----------------------------------------------------------------------
    CFG_ATPARSER                    Set to 1 to enable the AT parser
    CFG_ATPARSER_BUFSIZE            Sets the size of the AT parser buffer(bytes)
    CFG_ATPARSER_DELIM              The default delimiter for command & arguments
    CFG_ATPARSER_BLEUART_MODESWITCH If true, the user can dynamically switch
                                    between CMD and DATA mode over BLE UART
                                    via the '+++' command
    -----------------------------------------------------------------------*/
    #define CFG_ATPARSER                               1
    #define CFG_ATPARSER_BUFSIZE                       256
    #define CFG_ATPARSER_DELIM                         ","
    #define CFG_ATPARSER_BLEUART_MODESWITCH            1
/*=========================================================================*/


/*=========================================================================
    SIMPLE DATA EXCHANGE PROTOCOL (SDEP) SETTINGS
    -----------------------------------------------------------------------
    CFG_SDEP                  Set to 1 to enable SDEP (simple data
                              exchange protocol)
    -----------------------------------------------------------------------*/
    #define CFG_SDEP                                   1
    #define CFG_SDEP_BUFSIZE                           256
/*=========================================================================*/


/*=========================================================================
    DOTSTAR SETTINGS
    -----------------------------------------------------------------------
    CFG_DRIVERS_DOTSTAR                    Set to 1 to enable driver
    CFG_DRIVERS_DOTSTAR_LEDSPERSTRIP       MAX number of LED per strip
    -----------------------------------------------------------------------*/
    #define CFG_DRIVERS_DOTSTAR                         0
    #define CFG_DRIVERS_DOTSTAR_LEDSPERSTRIP            8
/*=========================================================================*/


/*=========================================================================
    SPI SLAVE SETTINGS
    -----------------------------------------------------------------------
    CFG_SPIS                  Set to 1 to enable SPI in slave mode
    CFG_SPIS_BUFSIZE          Size of the FIFO buffer for SPIS in bytes
    -----------------------------------------------------------------------*/
    #define CFG_SPIS                                    1
    #define CFG_SPIS_TX_BUFSIZE                         (1024*1)
    #define CFG_SPIS_RX_BUFSIZE                         (1024*1)
/*=========================================================================*/


/*=========================================================================
    GENERAL NRF51 PERIPHERAL SETTINGS
    -----------------------------------------------------------------------
    CFG_SCHEDULER_ENABLE            Set this to 'true' or 'false' depending on
                                    whether you use the event scheduler

    CFG_GPIOTE_MAX_USERS            Maximum number of users of the GPIOTE
                                    handler

    CFG_TIMER_PRESCALER             Value of the RTC1 PRESCALER register.
                                    Freq = (32768/(PRESCALER-1))
    CFG_TIMER_OPERATION_QUEUE_SIZE  Size of timer operation queues

    -----------------------------------------------------------------------*/
    #define CFG_SCHEDULER_ENABLE                       false

    /*------------------------------- GPIOTE ------------------------------*/
    #define CFG_GPIOTE_MAX_USERS                       2

    /*-------------------------------- TIMER ------------------------------*/
    #define CFG_TIMER_PRESCALER                        32
    #define CFG_TIMER_OPERATION_QUEUE_SIZE             6
/*=========================================================================*/


/*=========================================================================
    Watchdog Timer
    -----------------------------------------------------------------------
    CFG_WDT                   Set to 1 to enable the watchdog timer
    CFG_WDT_TIMEOUT           WDT timeout in seconds
    -----------------------------------------------------------------------*/
    #define CFG_WDT                                    1
    #define CFG_WDT_TIMEOUT                            10
/*=========================================================================*/


/*=========================================================================
    BTLE SETTINGS
    -----------------------------------------------------------------------
    CFG_PIN_ENABLED             Enable PIN code for bonding connection

    CFG_PIN_DEFAULTVALUE        Default PIN code must be 6 digits.

    CFG_BLE_TX_POWER_LEVEL      The TX power level in dBm (valid values are
                                -40, -20, -16, -12, -8, -4, 0, 4)

    CFG_GAP_APPEARANCE          The value for the appearance flag in GAP
                                advertising data
    CFG_GAP_DEFAULT_NAME        The default device name in the GAP
                                advertising data

    CFG_GAP_CONNECTION_MIN_INTERVAL_MS        Minimum acceptable connection interval (in ms)
                                              Parameter should be a multiple of 1.25ms, if not
                                              it is reduced to the next lower value (e.g 12 --> 11.25)

    CFG_GAP_CONNECTION_MAX_INTERVAL_MS        Maximum acceptable connection interval (in ms)
                                              Parameter should be a multiple of 1.25ms, if not
                                              it is reduced to the next lower value (e.g 12 --> 11.25)

    CFG_GAP_CONNECTION_SUPERVISION_TIMEOUT_MS Connection supervision timeout
    CFG_GAP_CONNECTION_SLAVE_LATENCY          Slave latency in number of
                                              connection events
    CFG_GAP_ADV_INTERVAL_MS                   The advertising interval in
                                              milliseconds (should be a
                                              multiple of 0.625)
    CFG_GAP_ADV_TIMEOUT_S                     The advertising timeout in units
                                              of seconds. After this delay it
                                              will advertise at the low power
                                              rate.
    CFG_GAP_ADV_LOWPOWER_INTERVAL_MS          Low power advertiing rate. This
                                              is used after the initial
                                              interval defined by
                                              CFG_GAP_ADV_TIMEOUT_S if no
                                              connection was established in
                                              that timeframe.
    -----------------------------------------------------------------------*/
    #define CFG_GAP_APPEARANCE                         0
    #define CFG_GAP_DEVICE_NAME                       "Adafruit Bluefruit LE"
    #define CFG_GAP_PRPH_PRIVACY_FLAG                  0


    #define CFG_PIN_ENABLED                            0
    #define CFG_PIN_DEFAULTVALUE                       "000000"

    #define CFG_BLE_TX_POWER_LEVEL                     0

    #define CFG_GAP_CONNECTION_MIN_INTERVAL_MS         20
    #define CFG_GAP_CONNECTION_MAX_INTERVAL_MS         40
    #define CFG_GAP_CONNECTION_SUPERVISION_TIMEOUT_MS  3000
    #define CFG_GAP_CONNECTION_SLAVE_LATENCY           0

    #define CFG_GAP_ADV_INTERVAL_MS                    20
    #define CFG_GAP_ADV_TIMEOUT_S                      30
    #define CFG_GAP_ADV_LOWPOWER_INTERVAL_MS           418 // actually 417.5 ms
    #define CFG_GAP_ADV_NONCONNECTABLE_INTERVAL_MIN_MS 100 // ADV_NONCONN_IND is tested not working with lower interval

    // TODO change to ms later
    #define CFG_GAP_CENTRAL_SCAN_INTERVAL              0x00A0 /**< Determines scan interval in units of 0.625 millisecond. */
    #define CFG_GAP_CENTRAL_SCAN_WINDOW                0x0050 /**< Determines scan window in units of 0.625 millisecond. */
/*=========================================================================*/


/*=========================================================================
    BTLE SERVICES
    -----------------------------------------------------------------------
    CFG_BLE_SDEP              Enables the SDEP BLE service
    CFG_BLE_SENSOR            Enables the unified sensor service
    CFG_BLE_DFU               Enables the DFU BLE service, which allows
                              user to trigger a DFU update via the GATT
                              service
    CFG_BLE_UART              Enables the nRF UART service
    CFG_BLE_UART_BUFSIZE      The FIFO buffer size for BLE UART data
    CFG_BLE_DEVICE_INFO       Whether to enable the Device Information
                              Service
    -----------------------------------------------------------------------*/
    #define CFG_BLE_SDEP                               0
    #define CFG_BLE_SENSOR                             0
    #define CFG_BLE_DFU                                1
    #define CFG_BLE_UART                               1
    #define CFG_BLE_UART_TX_BUFSIZE                    (1024*1)
    #define CFG_BLE_UART_RX_BUFSIZE                    (1024*1)
    #define CFG_BLE_DEVICE_INFO                        1
    #define CFG_BLE_DEVICE_INFO_MANUFACTURER           "Adafruit Industries"
    #define CFG_BLE_DEVICE_INFO_MODEL_NUMBER           BOARD_NAME

    #define CFG_BLE_HID                                1
    #define CFG_BLE_HID_KEYBOARD                       1
    #define CFG_BLE_HID_MOUSE                          1

    #define CFG_BLE_MIDI                               1
    #define CFG_BLE_MIDI_TX_BUFSIZE                    (256*2)
    #define CFG_BLE_MIDI_RX_BUFSIZE                    (256*2)

    #define CFG_BLE_ANCS                               0

    #define CFG_BLE_EDDYSTONE_CONFIG_MODE              1
//    #define CFG_BLE_EDDYSTONE_DEFAULT_KEY              "bluefruit"
    #define CFG_BLE_EDDYSTONE_DEFAULT_TX_LEVELS        {+4, 0, -4, -12}
    #define CFG_BLE_EDDYSTONE_DEFAULT_URL              "\x00" "adafruit\x07" // http://www.adafruit.com
/*=========================================================================*/


/*=========================================================================
    BLE SENSOR MANAGEMENT SERVICE SETTINGS
    -----------------------------------------------------------------------
    CFG_SENSORS_SERVICECOUNT  The maximum number of sensors that can be
                              instantiated on the system (due to memory
                              resource constraints)
    CFG_SENSORS_GPIOEVENTS    Set to 1 to toggle a GPIO pin every time a
                              sensor is read by the sensor management
                              system, one pin per sensor. Pin locations
                              are defined in the board config file(s).
    CFG_SENSORS_SIMULATED     Set to 1 to include sensor simulators
    -----------------------------------------------------------------------*/
    #if CFG_BLE_SENSOR

    #define CFG_SENSORS_SERVICECOUNT                    5
    #define CFG_SENSORS_GPIOEVENTS                      0
    #define CFG_SENSORS_SIMULATED                       1

    #endif
/*=========================================================================*/


/*=========================================================================
    CONFIG FILE VALIDATION
    -----------------------------------------------------------------------*/
    #define CHECK_TX_POWER(tx)        ( (tx) == -40 || (tx) == -20 || \
                                        (tx) == -16 || (tx) == -12 || \
                                        (tx) == -8  || (tx) == -4  || \
                                        (tx) == 0   || (tx) == 4 )

    #if !CHECK_TX_POWER(CFG_BLE_TX_POWER_LEVEL)
      #error "CFG_BLE_TX_POWER_LEVEL must be -40, -20, -16, -12, -8, -4, 0 or 4"
    #endif

    #if CFG_BLE_SDEP && !CFG_SDEP
      #error "CFG_SDEP must be enable to use the CFG_BLE_SDEP service"
    #endif

    #ifndef __CROSSWORKS_ARM
      #undef  CFG_PRINT_CWDEBUG
      #define CFG_PRINT_CWDEBUG 0
    #endif
/*=========================================================================*/

#endif /* _PROJECTCONFIG_H_ */
