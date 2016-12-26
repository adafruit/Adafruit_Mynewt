# GPIO HAL

ToDo!

## Basic Examples

### Input (`hal_gpio_read`)
```
/* Initialise pin as an input, pullup enabled */
hal_gpio_init_in(MY_PIN, HAL_GPIO_PULL_UP);

while(1) {
    int pin_state = hal_gpio_read(MY_PIN);
    os_time_delay(OS_TICKS_PER_SEC*1);
}
```

### Output

ToDo!

### Toggle (`hal_gpio_toggle`)

```
/* Initialise pin as an output, setting it high */
hal_gpio_init_out(LED_BLINK_PIN, 1);

while(1) {
    hal_gpio_toggle(LED_BLINK_PIN);
    os_time_delay(OS_TICKS_PER_SEC*1);
}
```

### Interrupt (`hal_gpio_irq_*`)

To enable HW interrupts, first declare an interrupt handler ...

```
#define IRQ_PIN (20)

static void
my_irq_handler(void *arg) {
    /* Do something ... */

    /* Optionally disable the interrupt (won't affect internal resistor) */
    // hal_gpio_irq_release(IRQ_PIN);
}
```

... then configure the pin as an interrupt, pointing to the interrupt handler:

```
/* Init IRQ pin: falling edge, pullup enabled */
hal_gpio_irq_init(IRQ_PIN,
                  my_irq_handler,
                  NULL,
                  HAL_GPIO_TRIG_FALLING,
                  HAL_GPIO_PULL_UP);

/* Enable the IRQ */
hal_gpio_irq_enable(IRQ_PIN);
```
