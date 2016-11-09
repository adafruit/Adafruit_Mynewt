# I2C HAL

## BSP Config

Make sure that your BSP is configured for I2C support. In the case of the
**nRF51** and **nRF52**, this means adding the `nrf_drv_config.h` file to
your BSP with the following settings (for example):

```
#define TWI0_ENABLED 1

#if (TWI0_ENABLED == 1)
#define TWI0_USE_EASY_DMA 0

#define TWI0_CONFIG_FREQUENCY    NRF_TWI_FREQ_100K
#define TWI0_CONFIG_SCL          26
#define TWI0_CONFIG_SDA          25
#define TWI0_CONFIG_IRQ_PRIORITY APP_IRQ_PRIORITY_LOW

#define TWI0_INSTANCE_INDEX      0
#endif
```

Then in the BSP's `syscfg.yml` file add:

```
I2C_0:
    description: 'NRF52 I2C (TWI) interface 0'
    value:  '1'
```

## Library/Application Config

### `pkg.yml` Settings

ToDo

### `syscfg.yml` Settings

ToDo

### Source Code

Add the following header to your .c file:

```
#include "hal/hal_i2c.h"
```
