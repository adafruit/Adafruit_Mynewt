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
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "hal/hal_cputime.h"
#include <console/console.h>
#include <shell/shell.h>
#include <log/log.h>

/* BLE */
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "host/ble_hs_adv.h"
#include "host/ble_uuid.h"
#include "host/ble_att.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_l2cap.h"
#include "host/ble_sm.h"
#include "controller/ble_ll.h"

/* RAM HCI transport. */
#include "transport/ram/ble_hci_ram.h"

/* Newtmgr include */
#include "newtmgr/newtmgr.h"
#include "nmgrble/newtmgr_ble.h"

/* RAM persistence layer. */
#include "store/ram/ble_store_ram.h"

/* Mandatory services. */
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "adafruit/adafruit_util.h"
#include "adafruit/bledis.h"
#include "adafruit/bleuart.h"


#define CFG_GAP_DEVICE_NAME     "Adafruit Bluefruit"

//--------------------------------------------------------------------+
//
//--------------------------------------------------------------------+
/** Mbuf settings. */
#define MBUF_NUM_MBUFS      (110)
#define MBUF_BUF_SIZE       OS_ALIGN(BLE_MBUF_PAYLOAD_SIZE, 4)
#define MBUF_MEMBLOCK_SIZE  (MBUF_BUF_SIZE + BLE_MBUF_MEMBLOCK_OVERHEAD)
#define MBUF_MEMPOOL_SIZE   OS_MEMPOOL_SIZE(MBUF_NUM_MBUFS, MBUF_MEMBLOCK_SIZE)

static os_membuf_t  mbuf_mpool_data[MBUF_MEMPOOL_SIZE];
struct os_mbuf_pool mbuf_pool;
struct os_mempool   mbuf_mpool;

//--------------------------------------------------------------------+
// TASK Settings
//--------------------------------------------------------------------+
/** Priority of the nimble host and controller tasks. */
#define BLE_LL_TASK_PRI             (OS_TASK_PRI_HIGHEST)

/** bleprph task settings. */
#define BLE_TASK_PRIO           1
#define BLE_STACK_SIZE          (OS_STACK_ALIGN(336))
struct os_eventq btle_evq;
struct os_task btle_task;
bssnz_t os_stack_t btle_stack[BLE_STACK_SIZE];

// shell task
#define SHELL_TASK_PRIO             (3)
#define SHELL_MAX_INPUT_LEN         (256)
#define SHELL_TASK_STACK_SIZE (OS_STACK_ALIGN(384))
os_stack_t shell_stack[SHELL_TASK_STACK_SIZE];

// netmgr task
#define NEWTMGR_TASK_PRIO            (4)
#define NEWTMGR_TASK_STACK_SIZE      (OS_STACK_ALIGN(512))
os_stack_t newtmgr_stack[NEWTMGR_TASK_STACK_SIZE];

/* Task Blinky */
#define BLINKY_TASK_PRIO     10
#define BLINKY_STACK_SIZE    OS_STACK_ALIGN(128)

struct os_task blinky_task;
os_stack_t blinky_stack[BLINKY_STACK_SIZE];

//--------------------------------------------------------------------+
//
//--------------------------------------------------------------------+
/** Our global device address (public) */
uint8_t g_dev_addr[BLE_DEV_ADDR_LEN] = {0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a};

/** Our random address (in case we need it) */
uint8_t g_random_addr[BLE_DEV_ADDR_LEN];

uint16_t conn_handle = BLE_HS_CONN_HANDLE_NONE;

//--------------------------------------------------------------------+
//
//--------------------------------------------------------------------+
static inline uint32_t tick2ms(os_time_t tick)
{
  return ((uint64_t) (tick*1000)) / OS_TICKS_PER_SEC;
}


static int cmd_nustest_exec(int argc, char **argv);
static struct shell_cmd cmd_nustest = {
    .sc_cmd = "nustest",
    .sc_cmd_func = cmd_nustest_exec
};

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
  VERIFY(data, -1);

  for(uint8_t i=0; i<size; i++)
  {
    data[i] = i%10 + '0';
  }

  // Negotiate a larger MTU if size > 20
  if ( size > 20 )
  {
    ble_gattc_exchange_mtu(conn_handle, NULL, NULL);
    // wait for the MTU procedure to complete, could use complete callback
    // but just delay 500 ms now
    os_time_delay(500);
  }

  os_time_t tick = os_time_get();

  for(uint8_t i=0; i<count; i++)
  {
    // delay a bit if out of memory (cannot send)
    bleuart_write(data, size);
  }

  tick = os_time_get() -  tick;
  uint32_t ms = tick2ms(tick);

  free(data);

  /* Print result */
  printf("Sent %lu bytes (%lu packets of %lu size) in %lu milliseconds\n", total, count, size, ms);
  printf("Speed: %lu.%lu KB/s\n", total/ms, 100*(total%ms)/ms );

  return 0;
}


static int btle_gap_event(struct ble_gap_event *event, void *arg);

/**
 * Enables advertising with the following parameters:
 *     o General discoverable mode.
 *     o Undirected connectable mode.
 */
static void
btle_advertise(void)
{
    /**
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info).
     *     o Advertising tx power.
     *     o Device name.
     *     o 16-bit service UUIDs (alert notifications).
     */
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof fields);

    /* Indicate that the flags field should be included; specify a value of 0
     * to instruct the stack to fill the value in for us.
     */
    fields.flags_is_present      = 1;
    fields.flags                 = 0;

    /* Indicate that the TX power level field should be included; have the
     * stack fill this one automatically as well.  This is done by assiging the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl            = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    fields.uuids128              = (void*) BLEUART_UUID_SERVICE ;
    fields.num_uuids128          = 1;
    fields.uuids128_is_complete  = 0;

    ASSERT_STATUS_RETVOID( ble_gap_adv_set_fields(&fields) );

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
    ASSERT_STATUS_RETVOID( ble_gap_adv_start(BLE_ADDR_TYPE_PUBLIC, 0, NULL, BLE_HS_FOREVER,
                           &adv_params, btle_gap_event, NULL) );
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
static int
btle_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed. */
        if (event->connect.status == 0) {
          conn_handle = event->connect.conn_handle;
          bleuart_set_conn_handle(conn_handle);
        }else {
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

/**
 * Event loop for the main bleprph task.
 */
static void
btle_task_handler(void *unused)
{
    struct os_event *ev;
    struct os_callout_func *cf;
    int rc;

    shell_cmd_register(&cmd_nustest);
    printf("nustest <count> <packetsize>\r\n");

    rc = ble_hs_start();
    assert(rc == 0);

    /* Begin advertising. */
    btle_advertise();

    while (1) {
        ev = os_eventq_get(&btle_evq);

        /* Check if the event is a nmgr ble mqueue event */
        rc = nmgr_ble_proc_mq_evt(ev);
        if (!rc) {
            continue;
        }

        switch (ev->ev_type) {
        case OS_EVENT_T_TIMER:
            cf = (struct os_callout_func *)ev;
            assert(cf->cf_func);
            cf->cf_func(CF_ARG(cf));
            break;
        default:
            assert(0);
            break;
        }
    }
}

void blinky_task_handler(void* arg)
{
  hal_gpio_init_out(LED_BLINK_PIN, 1);

  while(1)
  {
    os_time_delay(1000);

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
    struct ble_hci_ram_cfg hci_cfg;
    struct ble_hs_cfg cfg;

    int i;

    /* Initialize OS */
    os_init();

    /* Set cputime to count at 1 usec increments */
    ASSERT_STATUS( cputime_init(1000000) );

    /* Seed random number generator with least significant bytes of device address. */
    uint32_t seed = 0;
    for (i = 0; i < 4; ++i) {
        seed |= g_dev_addr[i];
        seed <<= 8;
    }
    srand(seed);

    /* Initialize msys mbufs. */
    ASSERT_STATUS( os_mempool_init(&mbuf_mpool, MBUF_NUM_MBUFS, MBUF_MEMBLOCK_SIZE, mbuf_mpool_data, "mbuf_data") );
    ASSERT_STATUS( os_mbuf_pool_init(&mbuf_pool, &mbuf_mpool, MBUF_MEMBLOCK_SIZE, MBUF_NUM_MBUFS) );
    ASSERT_STATUS( os_msys_register(&mbuf_pool) );

    //------------- Task Init -------------//
    shell_task_init(SHELL_TASK_PRIO, shell_stack, SHELL_TASK_STACK_SIZE, SHELL_MAX_INPUT_LEN);
    console_init(shell_console_rx_cb);

    nmgr_task_init(NEWTMGR_TASK_PRIO, newtmgr_stack, NEWTMGR_TASK_STACK_SIZE);

    os_task_init(&blinky_task, "blinky", blinky_task_handler, NULL,
                 BLINKY_TASK_PRIO, OS_WAIT_FOREVER, blinky_stack, BLINKY_STACK_SIZE);

    os_task_init(&btle_task, "bleprph", btle_task_handler, NULL,
                 BLE_TASK_PRIO, OS_WAIT_FOREVER, btle_stack, BLE_STACK_SIZE);

    /* Initialize the BLE LL */
    ASSERT_STATUS( ble_ll_init(BLE_LL_TASK_PRI, MBUF_NUM_MBUFS, BLE_MBUF_PAYLOAD_SIZE) );

    /* Initialize the RAM HCI transport. */
    hci_cfg = ble_hci_ram_cfg_dflt;
    ASSERT_STATUS( ble_hci_ram_init(&hci_cfg) );

    /* Initialize the BLE host. */
    cfg = ble_hs_cfg_dflt;
    cfg.max_hci_bufs        = hci_cfg.num_evt_hi_bufs + hci_cfg.num_evt_lo_bufs;
    cfg.max_connections     = 1;
    cfg.max_gattc_procs     = 2;
    cfg.max_l2cap_chans     = 3;
    cfg.max_l2cap_sig_procs = 1;
    cfg.sm_bonding          = 1;
    cfg.sm_our_key_dist     = BLE_SM_PAIR_KEY_DIST_ENC;
    cfg.sm_their_key_dist   = BLE_SM_PAIR_KEY_DIST_ENC;
    cfg.store_read_cb       = ble_store_ram_read;
    cfg.store_write_cb      = ble_store_ram_write;

    /* Populate config with the required GATT server settings. */
    cfg.max_attrs           = 0;
    cfg.max_services        = 0;
    cfg.max_client_configs  = 0;

    /* GATT server initialization */
    ASSERT_STATUS( ble_svc_gap_init(&cfg) );
    ASSERT_STATUS( ble_svc_gatt_init(&cfg) );
    ASSERT_STATUS( nmgr_ble_gatt_svr_init(&btle_evq, &cfg) );

    bledis_cfg_t dis_cfg =
    {
        .model        = "Feather52" ,
        .serial       = NULL        ,
        .firmware_rev = "0.9.0"     ,
        .hardware_rev = "nRF52832"  ,
        .software_rev = "0.9.0"     ,
        .manufacturer = "Adafruit Industries"
    };
    bledis_init(&cfg, &dis_cfg);

#if 0
    bleuart_gatt_svr_init(&cfg);
    bleuart_init(128);
#else
	bleuart_init(&cfg);
#endif

    /* Initialize eventq */
    os_eventq_init(&btle_evq);
    ASSERT_STATUS( ble_hs_init(&btle_evq, &cfg) );

    /* Set the default device name. */
    ASSERT_STATUS( ble_svc_gap_device_name_set(CFG_GAP_DEVICE_NAME) );

    /* Start the OS */
    os_start();

    /* os start should never return. If it does, this should be an error */
    assert(0);

    return 0;
}
