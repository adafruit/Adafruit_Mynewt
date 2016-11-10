/**************************************************************************/
/*!
    @file     ada_cfg.c
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

#include "adafruit/adautil.h"

#if defined (NFFS_PRESENT) && defined(FS_PRESENT)

#include <bsp/bsp.h>
#include <hal/hal_flash.h>

#include <fs/fs.h>
#include <nffs/nffs.h>
#include <config/config.h>
#include <config/config_file.h>

/*------------------------------------------------------------------*/
/* MACRO TYPEDEF CONSTANT ENUM
 *------------------------------------------------------------------*/



/*------------------------------------------------------------------*/
/* VARIABLE DECLARATION
 *------------------------------------------------------------------*/
static struct conf_file _cfg_file =
{
    .cf_name     = CFG_ADACFG_FILE,
    .cf_maxlines = 32
};

static char* cfg_get    (int argc, char **argv, char *val, int max_len);
static int   cfg_set    (int argc, char **argv, char *val);
static int   cfg_commit (void);
static int   cfg_export (void (*export_func)(char *name, char *val), enum conf_export_tgt tgt);

static struct
{
  struct conf_handler hdl;
  uint8_t list_count;
  const adacfg_info_t* varlist[CFG_ADACFG_MAXCONFIG];
} _adacfg =
{
    .hdl = {
      .ch_name   = "adafruit",
      .ch_get    = cfg_get,
      .ch_set    = cfg_set,
      .ch_commit = cfg_commit,
      .ch_export = cfg_export
    }
};

/*------------------------------------------------------------------*/
/* FUNCTION DECLARATION
 *------------------------------------------------------------------*/
int adacfg_nffs_init(void)
{
  /*------------- Init NFFS -------------*/
  VERIFY_STATUS( hal_flash_init() );

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

  // Mkdir anyway, if existed, no action is executed
  (void) fs_mkdir(ADACFG_DIR);

  return 0;
}

/**
 * Initialize Config module with a common prefix
 * @param prefix
 * @return
 */
int adacfg_init(const char* prefix)
{
  adacfg_nffs_init();

  VERIFY_STATUS( conf_file_src(&_cfg_file) );
  VERIFY_STATUS( conf_file_dst(&_cfg_file) );

  /*------------- init Config module & Register config file -------------*/
  conf_init();

  _adacfg.list_count = 0;
  if (prefix) _adacfg.hdl.ch_name = (char*) prefix;
  VERIFY_STATUS( conf_register(&_adacfg.hdl) );

  return 0;
}

/**
 * Add an configure array
 * @param cfg
 * @return
 */
int adacfg_add(const adacfg_info_t* cfg)
{
  if ( _adacfg.list_count >= CFG_ADACFG_MAXCONFIG ) return -1;

  _adacfg.varlist[_adacfg.list_count++] = cfg;

  conf_load();

  return 0;
}

/**
 * Helper to check if var name in [argc, argv] is equal to string array
 * @param name
 * @param argc
 * @param argv
 * @return
 */
static bool name_equal(char const * name, int argc, char **argv)
{
  for(int i=0; i<argc; i++)
  {
    int len = strlen(argv[i]);
    if ( memcmp(argv[i], name, len) ) return false;

    name += len;

    if (*name != 0)
    {
      if (*name != '/') return false;
      name++;
    }
  }

  return true;
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
  for ( uint8_t i = 0; i < _adacfg.list_count; i++ )
  {
    const adacfg_info_t* cfgvar = _adacfg.varlist[i];

    while ( cfgvar->name )
    {
      if ( name_equal(cfgvar->name, argc, argv) )
      {
        return conf_value_from_str(val, cfgvar->type, cfgvar->value, cfgvar->len);
      }

      cfgvar++;
    }
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
  char* tempbuf  = malloc(CONF_MAX_VAL_LEN+1);
  char* fullname = malloc(CONF_MAX_NAME_LEN);

  strcpy(fullname, _adacfg.hdl.ch_name);

  char* varname = fullname + strlen(fullname);
  *varname++ = '/';

  for ( uint8_t i = 0; i < _adacfg.list_count; i++ )
  {
    const adacfg_info_t* cfgvar = _adacfg.varlist[i];
    while ( cfgvar->name )
    {
      char* value = conf_str_from_value(cfgvar->type, cfgvar->value, tempbuf, sizeof(tempbuf));

      strcpy(varname, cfgvar->name);
      func(fullname, value);

      cfgvar++;
    }
  }

  free(tempbuf);
  free(fullname);

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
  for ( uint8_t i = 0; i < _adacfg.list_count; i++ )
  {
    const adacfg_info_t* cfgvar = _adacfg.varlist[i];

    while ( cfgvar->name )
    {
      if ( name_equal(cfgvar->name, argc, argv) )
      {
        return conf_str_from_value(cfgvar->type, cfgvar->value, buf, max_len);
      }

      cfgvar++;
    }
  }

  return NULL;
}


#endif
