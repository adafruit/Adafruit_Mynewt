# SPI

## HAL API

You can perform read, write an other operations using the following functions:

```
int hal_spi_config(int spi_num, struct hal_spi_settings *psettings);
int hal_spi_set_txrx_cb(int spi_num, hal_spi_txrx_cb txrx_cb, void *arg);
int hal_spi_enable(int spi_num);
int hal_spi_disable(int spi_num);
uint16_t hal_spi_tx_val(int spi_num, uint16_t val);
int hal_spi_txrx(int spi_num, void *txbuf, void *rxbuf, int cnt);
int hal_spi_txrx_noblock(int spi_num, void *txbuf, void *rxbuf, int cnt);
int hal_spi_slave_set_def_tx_val(int spi_num, uint16_t val);
int hal_spi_abort(int spi_num);
```

`hal_spi_settings` has the following structure and enum values:

```
struct hal_spi_settings {
    uint8_t         data_mode;
    uint8_t         data_order;
    uint8_t         word_size;
    uint32_t        baudrate;		/* baudrate in kHz */
};

/* SPI modes */
#define HAL_SPI_MODE0               (0)
#define HAL_SPI_MODE1               (1)
#define HAL_SPI_MODE2               (2)
#define HAL_SPI_MODE3               (3)

/* SPI data order */
#define HAL_SPI_MSB_FIRST           (0)
#define HAL_SPI_LSB_FIRST           (1)

/* SPI word size */
#define HAL_SPI_WORD_SIZE_8BIT      (0)
#define HAL_SPI_WORD_SIZE_9BIT      (1)
```
#### Baud Rate limitations

Certain MCUs impose specific `baudrate` limitations when working with the
`hal_spi_settings` struct. Consult the table below to make sure that your
select an appropriate value:

| MCU           | Baud Rates                            |
| ---:          | :---:                                 |
| **nRF51xxx**  | 125, 250, 500, 1000, 2000, 4000, 8000 |
| **nRF52xxx**  | 125, 250, 500, 1000, 2000, 4000, 8000 |
| **stm32f4xx** | Any (within peripheral range)         |

## BSP Config

### `syscfg.yml` Settings

In the BSP, target or app's `syscfg.yml` file add:

```
SPI_1_MASTER:
    description: 'SPI 1 master'
    value:  1
```

> **NOTE**: SPI1 is used here since SPI0 will be reserved for SPI based storage
peripherals like an SD card or SPI flash memory.

### Pin Settings

In the case of the **nRF51** and **nRF52**, most pin config takes place in the
`nrf_drv_config.h` file which should be part of your BSP. For example, the
following code configures pins 12, 13 and 14 for SPI:

```
#define SPI1_CONFIG_SCK_PIN         12
#define SPI1_CONFIG_MOSI_PIN        13
#define SPI1_CONFIG_MISO_PIN        14
#define SPI1_CONFIG_IRQ_PRIORITY    APP_IRQ_PRIORITY_LOW
```

## Library/Application Config

### `pkg.yml` Settings

In the `pkg.deps` section of your library or app add the following dependency:

```
pkg.deps:
    - "@apache-mynewt-core/hw/hal"
```

### Source Code

If `SPI_1_MASTER` is set to `1` in the BSP or app's `syscfg.yml` file, SPI1
will be initialised as master when `sysinit` is called, but the following
changes are required to use the SPI bus in your application or code:

Add the following header to your .c file:

```
#include "hal/hal_spi.h"
```
