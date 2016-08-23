# NFFS Config Memory

[NFFS](https://mynewt.incubator.apache.org/develop/os/modules/fs/nffs/nffs/) can
be used to store config settings in flash memory, and access these settings as
files through the various helper functions available in the file system
abstraction layer.

## Flash Memory Layout

The flash memory layout is defined in `hw/hal/include/hal/flash_map.h`, and
normally includes a section for NFFS storage. In the case of the nRF52 (as of
version 0.9.0) the following sections are defined:

```
#define FLASH_AREA_BOOTLOADER           0
#define FLASH_AREA_IMAGE_0              1
#define FLASH_AREA_IMAGE_1              2
#define FLASH_AREA_IMAGE_SCRATCH        3
#define FLASH_AREA_NFFS                 4
#define FLASH_AREA_CORE                 2
#define FLASH_AREA_REBOOT_LOG           5
```

The area we need is called `FLASH_AREA_NFFS`.

The actual memory size and location in flash memory is defined at the BSP level,
typically in the `hw/bsp/yourbspname/src/os_bsp.c` file. For example, this is
the defined flash memory layout for a sample nRF52 BSP:

```
static struct flash_area bsp_flash_areas[] = {
    [FLASH_AREA_BOOTLOADER] = {
        .fa_flash_id = 0,       /* internal flash */
        .fa_off = 0x00000000,   /* beginning */
        .fa_size = (32 * 1024)
    },
    /* 2*16K and 1*64K sectors here */
    [FLASH_AREA_IMAGE_0] = {
        .fa_flash_id = 0,
        .fa_off = 0x00008000,
        .fa_size = (232 * 1024)
    },
    [FLASH_AREA_IMAGE_1] = {
        .fa_flash_id = 0,
        .fa_off = 0x00042000,
        .fa_size = (232 * 1024)
    },
    [FLASH_AREA_IMAGE_SCRATCH] = {
        .fa_flash_id = 0,
        .fa_off = 0x0007c000,
        .fa_size = (4 * 1024)
    },
    [FLASH_AREA_NFFS] = {
        .fa_flash_id = 0,
        .fa_off = 0x0007d000,
        .fa_size = (12 * 1024)
    }
};
```

You can see here that `FLASH_AREA_NFFS` starts at a flash offset of 0x7D000,
which corresponds to 500KB, meaning the last 12KB flash is reserved for the
NFFS partition (the nRF52 has 512KB of flash memory available).

## Enabling NFFS Flash memory

ToDo

## Saving Config data

Config data is saved through the config file abstraction defined in
`sys/config/include/config/config.h` and (in the case of NFFS)
`sys/config/include/config/config_file.h`.

After creating and initialising a config file in the NFFS partition, you can
read and write config values via the helper functions defined in `config.h`.
