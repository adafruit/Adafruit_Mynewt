/**************************************************************************/
/*!
    @file     main.c

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

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "bsp/bsp.h"
#include "os/os.h"
#include "hal/hal_gpio.h"

#include "sysinit/sysinit.h"
#include <console/console.h>
#include <shell/shell.h>
#include <log/log.h>
#include <imgmgr/imgmgr.h>

/* BLE */
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"

/* Newtmgr include */
#include "newtmgr/newtmgr.h"
#include "nmgrble/newtmgr_ble.h"

/* Crash test ... just because */
#include "crash_test/crash_test.h"

/* Adafruit libraries and helpers */
#include "adafruit/adautil.h"
#include "adafruit/bledis.h"
#include "adafruit/bleuart.h"

/** Default device name */
#define CFG_GAP_DEVICE_NAME     "Adafruit Mynewt"

/*------------------------------------------------------------------*/
/* TASK Settings
 *------------------------------------------------------------------*/
/* Blinky task settings */
#define BLINKY_TASK_PRIO              (10)
#define BLINKY_STACK_SIZE             OS_STACK_ALIGN(128)

struct os_task blinky_task;
os_stack_t blinky_stack[BLINKY_STACK_SIZE];

/*------------------------------------------------------------------*/
/* Global values
 *------------------------------------------------------------------*/
uint16_t conn_handle = BLE_HS_CONN_HANDLE_NONE;

/*------------------------------------------------------------------*/
/* Functions prototypes
 *------------------------------------------------------------------*/
static int cmd_nustest_exec(int argc, char **argv);
static int btle_gap_event(struct ble_gap_event *event, void *arg);

/*------------------------------------------------------------------*/
/* Functions
 *------------------------------------------------------------------*/
static struct shell_cmd cmd_nustest =
{
    .sc_cmd      = "nustest",
    .sc_cmd_func = cmd_nustest_exec
};

/**
 *  'nustest' shell command handler
 */
static int cmd_nustest_exec(int argc, char **argv)
{
  /* 1st arg is number of packet (default 100)
   * 2nd arg is size of each packet (deault 20)
   */

  uint32_t count = (argc > 1) ? strtoul(argv[1], NULL, 10) : 100;
  uint32_t size  = (argc > 2) ? strtoul(argv[2], NULL, 10) : 20;

  if ( count > 100 )
  {
    printf("count must not exceed 100\n");
    return -1;
  }

  if ( size > 240 )
  {
    printf("size must not exceed 240\n");
    return -1;
  }

  uint32_t total = count * size;

  char *data = malloc(size);
  if(!data) return (-1);

  for(uint8_t i=0; i<size; i++)
  {
    data[i] = i%10 + '0';
  }

  /* Negotiate a larger MTU if size > 20 */
  if ( size > 20 )
  {
    ble_gattc_exchange_mtu(conn_handle, NULL, NULL);
    /* wait for the MTU procedure to complete. We could use a callback */
    /* but for now we simply delay 500 ms */
    os_time_delay(500);
  }

  for(uint8_t i=0; i<count; i++)
  {
    bleuart_write(data, size);
  }

  free(data);

  /* Print the results */
  printf("Submitted %lu bytes (%lu packets of %lu size)\n", total, count, size);

  return 0;
}

/**
 * Enables advertising with the following parameters:
 *     o General discoverable mode.
 *     o Undirected connectable mode.
 */
static void btle_advertise(void)
{
  struct ble_hs_adv_fields fields =
  {
      .flags                 = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP,

      /* Indicate that the TX power level field should be included; have the
       * stack fill this one automatically as well.  This is done by assiging the
       * special value BLE_HS_ADV_TX_PWR_LVL_AUTO. */
      .tx_pwr_lvl_is_present = 1,
      .tx_pwr_lvl            = BLE_HS_ADV_TX_PWR_LVL_AUTO,

      .uuids128              = (ble_uuid128_t*) &BLEUART_UUID_SERVICE,
      .num_uuids128          = 1,
      .uuids128_is_complete  = 0,
  };

  VERIFY_STATUS( ble_gap_adv_set_fields(&fields), RETURN_VOID );

  //------------- Scan response data -------------//
  const char *name = ble_svc_gap_device_name();
  struct ble_hs_adv_fields rsp_fields =
  {
      .name = (uint8_t*) name,
      .name_len = strlen(name),
      .name_is_complete = 1
  };
  ble_gap_adv_rsp_set_fields(&rsp_fields);

  /* Begin advertising. */
  struct ble_gap_adv_params adv_params;
  memset(&adv_params, 0, sizeof adv_params);
  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
  VERIFY_STATUS(ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &adv_params, btle_gap_event, NULL),
                RETURN_VOID);
}

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * bleprph uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param ctxt                  Various information pertaining to the event.
 * @param arg                   Application-specified argument; unuesd by
 *                                  bleprph.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
static int btle_gap_event(struct ble_gap_event *event, void *arg)
{
  switch ( event->type )
  {
    case BLE_GAP_EVENT_CONNECT:
      /* A new connection was established or a connection attempt failed. */
      if ( event->connect.status == 0 )
      {
        conn_handle = event->connect.conn_handle;
        bleuart_set_conn_handle(conn_handle);
      }
      else
      {
        /* Connection failed; resume advertising. */
        btle_advertise();

        conn_handle = BLE_HS_CONN_HANDLE_NONE;
      }
    return 0;

    case BLE_GAP_EVENT_DISCONNECT:
      /* Connection terminated; resume advertising. */
      btle_advertise();
    return 0;

  }

  return 0;
}

static void btle_on_sync(void)
{
  /* Begin advertising. */
  btle_advertise();
}

/**
 * Blinky task handler
 */
void blinky_task_handler(void* arg)
{
  hal_gpio_init_out(LED_BLINK_PIN, 1);

  while(1)
  {
    int32_t delay = OS_TICKS_PER_SEC * 1;
    os_time_delay(delay);
    
    hal_gpio_toggle(LED_BLINK_PIN);
  }
}


/**
 * main
 *
 * The main function for the project. This function initializes the os, calls
 * init_tasks to initialize tasks (and possibly other objects), then starts the
 * OS. We should not return from os start.
 *
 * @return int NOTE: this function should never return!
 */
int main(void)
{
  /* Initialize OS */
  sysinit();

  /* Set initial BLE device address. */
  memcpy(g_dev_addr, (uint8_t[6]){0xAD, 0xAF, 0xAD, 0xAF, 0xAD, 0xAF}, 6);

  //------------- Task Init -------------//
  os_task_init(&blinky_task, "blinky", blinky_task_handler, NULL,
               BLINKY_TASK_PRIO, OS_WAIT_FOREVER, blinky_stack, BLINKY_STACK_SIZE);

  /* Initialize the BLE host. */
  ble_hs_cfg.sync_cb        = btle_on_sync;
//  ble_hs_cfg.store_read_cb  = ble_store_ram_read;
//  ble_hs_cfg.store_write_cb = ble_store_ram_write;

  /* Init BLE Device Information Service */
  bledis_init();

  /* Nordic UART service (NUS) settings */
  bleuart_init();

  /* Command usage: nustest <count> <packetsize> */
  shell_cmd_register(&cmd_nustest);

  /* Set the default device name. */
  VERIFY_STATUS(ble_svc_gap_device_name_set(CFG_GAP_DEVICE_NAME));

  while (1) {
    os_eventq_run(os_eventq_dflt_get());
  }

  /* OS start should never return. If it does, this should be an error */
  assert(0);

  return 0;
}
