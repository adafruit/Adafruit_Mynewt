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

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "bsp/bsp.h"
#include "os/os.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "hal/hal_cputime.h"
#include <hal/hal_flash.h>
#include <hal/flash_map.h>

/*NFFS*/
#include <fs/fs.h>
#include <nffs/nffs.h>
#include <config/config.h>
#include <config/config_file.h>

#include <console/console.h>
#include <shell/shell.h>
#include <log/log.h>
#include <imgmgr/imgmgr.h>

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

/*------------------------------------------------------------------*/
/* Global values
 *------------------------------------------------------------------*/
static char serialnumber[16 + 1];

/** Our global device address (public) */
uint8_t g_dev_addr[BLE_DEV_ADDR_LEN] = {0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a};

/** Our random address (in case we need it) */
uint8_t g_random_addr[BLE_DEV_ADDR_LEN];


/*------------------------------------------------------------------*/
/* Mbuf settings
 *------------------------------------------------------------------*/
#define MBUF_NUM_MBUFS      (12)
#define MBUF_BUF_SIZE       OS_ALIGN(BLE_MBUF_PAYLOAD_SIZE, 4)
#define MBUF_MEMBLOCK_SIZE  (MBUF_BUF_SIZE + BLE_MBUF_MEMBLOCK_OVERHEAD)
#define MBUF_MEMPOOL_SIZE   OS_MEMPOOL_SIZE(MBUF_NUM_MBUFS, MBUF_MEMBLOCK_SIZE)

static os_membuf_t  mbuf_mpool_data[MBUF_MEMPOOL_SIZE];
struct os_mbuf_pool mbuf_pool;
struct os_mempool   mbuf_mpool;

/*------------------------------------------------------------------*/
/* TASK Settings
 *------------------------------------------------------------------*/
/** Priority of the nimble host and controller tasks. */
#define BLE_LL_TASK_PRI               (OS_TASK_PRI_HIGHEST)

/** bleprph task settings. */
#define BLE_TASK_PRIO                 1
#define BLE_STACK_SIZE                (OS_STACK_ALIGN(336))
struct os_eventq btle_evq;
struct os_task btle_task;
bssnz_t os_stack_t btle_stack[BLE_STACK_SIZE];

/* shell task */
#define SHELL_TASK_PRIO               (3)
#define SHELL_MAX_INPUT_LEN           (256)
#define SHELL_TASK_STACK_SIZE         (OS_STACK_ALIGN(384))
os_stack_t shell_stack[SHELL_TASK_STACK_SIZE];

/* netmgr task */
#define NEWTMGR_TASK_PRIO             (4)
#define NEWTMGR_TASK_STACK_SIZE       (OS_STACK_ALIGN(512))
os_stack_t newtmgr_stack[NEWTMGR_TASK_STACK_SIZE];

/* Task Blinky */
#define BLINKY_TASK_PRIO              10
#define BLINKY_STACK_SIZE             OS_STACK_ALIGN(128)

struct os_task blinky_task;
os_stack_t blinky_stack[BLINKY_STACK_SIZE];

/* BLEUART to UART bridge task */
#define BLEUART_BRIDGE_TASK_PRIO      5
#define BLEUART_BRIDGE_STACK_SIZE     OS_STACK_ALIGN(256)

struct os_task bleuart_bridge_task;
os_stack_t bleuart_bridge_stack[BLEUART_BRIDGE_STACK_SIZE];

/*------------------------------------------------------------------*/
/* NFFS Config
 *------------------------------------------------------------------*/
#define MY_CONFIG_FILE "/config"
#define MY_CONFIG_MAX_LINES  32

static struct conf_file cfg_file =
{
    .cf_name = MY_CONFIG_FILE,
    .cf_maxlines = MY_CONFIG_MAX_LINES
};

static char* cfg_get(int argc, char **argv, char *val, int max_len);
static int   cfg_set(int argc, char **argv, char *val);
static int   cfg_commit(void);
static int   cfg_export(void (*export_func)(char *name, char *val), enum conf_export_tgt tgt);

static struct conf_handler ada_cfg_handler =
{
    .ch_name   = "adafruit",
    .ch_get    = cfg_get,
    .ch_set    = cfg_set,
    .ch_commit = cfg_commit,
    .ch_export = cfg_export
};

struct
{
  char ble_devname[32]; /* ble/devname */
}ada_cfg =
{
    .ble_devname = CFG_GAP_DEVICE_NAME
};


/*------------------------------------------------------------------*/
/* Functions prototypes
 *------------------------------------------------------------------*/
static int btle_gap_event(struct ble_gap_event *event, void *arg);
static int setup_for_nffs(void);

/**
 * Enables advertising with the following parameters:
 *     o General discoverable mode.
 *     o Undirected connectable mode.
 */
static void btle_advertise(void)
{
  struct ble_hs_adv_fields fields =
  {
      /* Indicate that the flags field should be included; specify a value of 0
       * to instruct the stack to fill the value in for us. */
      .flags_is_present      = 1,
      .flags                 = 0,

      /* Indicate that the TX power level field should be included; have the
       * stack fill this one automatically as well.  This is done by assiging the
       * special value BLE_HS_ADV_TX_PWR_LVL_AUTO. */
      .tx_pwr_lvl_is_present = 1,
      .tx_pwr_lvl            = BLE_HS_ADV_TX_PWR_LVL_AUTO,

      .uuids128              = (void*) BLEUART_UUID_SERVICE,
      .num_uuids128          = 1,
      .uuids128_is_complete  = 0,
  };

  VERIFY_STATUS(ble_gap_adv_set_fields(&fields), RETURN_VOID);

  /*------------- Scan response data -------------*/
  const char *name = ble_svc_gap_device_name();
  struct ble_hs_adv_fields rsp_fields = { .name = (uint8_t*) name, .name_len = strlen(name), .name_is_complete = 1 };
  ble_gap_adv_rsp_set_fields(&rsp_fields);

  /* Begin advertising. */
  struct ble_gap_adv_params adv_params;
  memset(&adv_params, 0, sizeof adv_params);
  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
  VERIFY_STATUS(ble_gap_adv_start(BLE_ADDR_TYPE_PUBLIC, 0, NULL, BLE_HS_FOREVER, &adv_params, btle_gap_event, NULL),
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
  switch (event->type)
  {
    case BLE_GAP_EVENT_CONNECT:
      /* A new connection was established or a connection attempt failed. */
      if (event->connect.status == 0)
      {
        bleuart_set_conn_handle(event->connect.conn_handle);
      }
      else
      {
        /* Connection failed; resume advertising. */
        btle_advertise();
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
 * Event loop for the main btle task.
 */
static void btle_task_handler (void *unused)
{
  struct os_event *ev;
  struct os_callout_func *cf;
  int rc;

  rc = ble_hs_start();
  assert(rc == 0);

  /* Begin advertising. */
  btle_advertise();

  while ( 1 )
  {
    ev = os_eventq_get(&btle_evq);

    /* Check if the event is a nmgr ble mqueue event */
    rc = nmgr_ble_proc_mq_evt(ev);
    if ( !rc )
    {
      continue;
    }

    switch ( ev->ev_type )
    {
      case OS_EVENT_T_TIMER:
        cf = (struct os_callout_func *) ev;
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

void bleuart_bridge_task_handler(void* arg)
{
  // register 'nus' command to send BLEUART
  bleuart_shell_register();

  while(1)
  {
    int ch;

    // Get data from bleuart to hwuart
    if ( (ch = bleuart_getc()) != EOF )
    {
      console_write( (char*)&ch, 1);
    }

    os_time_delay(1);
  }
}


int main(void)
{
  struct ble_hci_ram_cfg hci_cfg;
  struct ble_hs_cfg cfg;
  uint32_t seed;
  int i;

  /* Initialize OS */
  os_init();

  /* Set cputime to count at 1 usec increments */
  VERIFY_STATUS( cputime_init(1000000) );

  /* Seed random number generator with least significant bytes of device
   * address. */
  seed = 0;
  for (i = 0; i < 4; ++i) {
    seed |= g_dev_addr[i];
    seed <<= 8;
  }
  srand(seed);

  /* Initialize msys mbufs. */
  VERIFY_STATUS( os_mempool_init(&mbuf_mpool, MBUF_NUM_MBUFS, MBUF_MEMBLOCK_SIZE, mbuf_mpool_data, "mbuf_data") );
  VERIFY_STATUS( os_mbuf_pool_init(&mbuf_pool, &mbuf_mpool, MBUF_MEMBLOCK_SIZE, MBUF_NUM_MBUFS) );
  VERIFY_STATUS( os_msys_register(&mbuf_pool) );

  /* Init Config & NFFS */
  conf_init();
  VERIFY_STATUS( conf_register(&ada_cfg_handler) );
  VERIFY_STATUS(hal_flash_init());
  setup_for_nffs();
  conf_load();

  //------------- Task Init -------------//
  shell_task_init(SHELL_TASK_PRIO, shell_stack, SHELL_TASK_STACK_SIZE, SHELL_MAX_INPUT_LEN);
  console_init(shell_console_rx_cb);

  nmgr_task_init(NEWTMGR_TASK_PRIO, newtmgr_stack, NEWTMGR_TASK_STACK_SIZE);
  imgmgr_module_init();

  os_task_init(&blinky_task, "blinky", blinky_task_handler, NULL,
               BLINKY_TASK_PRIO, OS_WAIT_FOREVER, blinky_stack, BLINKY_STACK_SIZE);

  os_task_init(&bleuart_bridge_task, "bleuart_bridge", bleuart_bridge_task_handler, NULL,
               BLEUART_BRIDGE_TASK_PRIO, OS_WAIT_FOREVER, bleuart_bridge_stack, BLEUART_BRIDGE_STACK_SIZE);

  os_task_init(&btle_task, "bleprph", btle_task_handler, NULL,
               BLE_TASK_PRIO, OS_WAIT_FOREVER, btle_stack, BLE_STACK_SIZE);

  /* Initialize the BLE LL */
  VERIFY_STATUS( ble_ll_init(BLE_LL_TASK_PRI, MBUF_NUM_MBUFS, BLE_MBUF_PAYLOAD_SIZE) );

  /* Initialize the RAM HCI transport. */
  hci_cfg = ble_hci_ram_cfg_dflt;
  VERIFY_STATUS( ble_hci_ram_init(&hci_cfg) );

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
  VERIFY_STATUS( ble_svc_gap_init(&cfg) );
  VERIFY_STATUS( ble_svc_gatt_init(&cfg) );
  VERIFY_STATUS( nmgr_ble_gatt_svr_init(&btle_evq, &cfg) );

  /* Convert MCU Unique Identifier to string as serial number */
  sprintf(serialnumber, "%08lX%08lX", NRF_FICR->DEVICEID[1], NRF_FICR->DEVICEID[0]);

  bledis_cfg_t dis_cfg =
  {
      .model        = "Feather52"  ,
      .serial       = serialnumber ,
      .firmware_rev = "0.9.0"      ,
      .hardware_rev = "nRF52832"   ,
      .software_rev = "0.9.0"      ,
      .manufacturer = "Adafruit Industries"
  };
  bledis_init(&cfg, &dis_cfg);

  bleuart_init(&cfg);

  /* Initialize eventq */
  os_eventq_init(&btle_evq);
  VERIFY_STATUS( ble_hs_init(&btle_evq, &cfg) );

  /* Set the default device name. */
  VERIFY_STATUS( ble_svc_gap_device_name_set(ada_cfg.ble_devname) );

  /* Start the OS */
  os_start();

  /* os start should never return. If it does, this should be an error */
  assert(0);

  return 0;
}

static int setup_for_nffs(void)
{
  /* NFFS_AREA_MAX is defined in the BSP-specified bsp.h header file. */
  struct nffs_area_desc descs[NFFS_AREA_MAX + 1];
  int cnt;

  /* Initialize nffs's internal state. */
  VERIFY_STATUS( nffs_init() );

  /* Convert the set of flash blocks we intend to use for nffs into an array
   * of nffs area descriptors.
   */
  cnt = NFFS_AREA_MAX;
  VERIFY_STATUS( flash_area_to_nffs_desc(FLASH_AREA_NFFS, &cnt, descs) );

  /* Attempt to restore an existing nffs file system from flash. */
  if ( nffs_detect(descs) == FS_ECORRUPT )
  {
    /* No valid nffs instance detected; format a new one. */
    VERIFY_STATUS ( nffs_format(descs) );
  }

  VERIFY_STATUS( conf_file_src(&cfg_file) );
  VERIFY_STATUS( conf_file_dst(&cfg_file) );

  return 0;
}

/**
 * Callback from config management to load data from Flash to local variable
 * @param argc
 * @param argv
 * @param val
 * @return
 */
static int cfg_set (int argc, char **argv, char *val)
{
  if ( (argc == 2) && !strcmp(argv[0], "ble") && !strcmp(argv[1], "devname") )
  {
    return CONF_VALUE_SET(val, CONF_STRING, ada_cfg.ble_devname);
  }

  return OS_ENOENT;
}

/**
 * Callback from config management when data is written to Flash
 * @return
 */
static int cfg_commit (void)
{
  /*not used for now*/
  return 0;
}

/**
 * Callback from config management to store all local variables to flash
 * @param func
 * @param tgt
 * @return
 */
static int cfg_export (void (*func) (char *name, char *val), enum conf_export_tgt tgt)
{
  func("adafruit/ble/devname", ada_cfg.ble_devname);
  return 0;
}

/**
 * Callback from config management to get current value from local variable
 * @param argc
 * @param argv
 * @param buf
 * @param max_len
 * @return
 */
static char* cfg_get (int argc, char **argv, char *buf, int max_len)
{
  if ( (argc == 2) && !strcmp(argv[0], "ble") && !strcmp(argv[1], "devname") )
  {
    return ada_cfg.ble_devname;
  }

  return NULL;
}

