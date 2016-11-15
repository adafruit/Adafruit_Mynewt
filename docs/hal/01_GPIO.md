# GPIO HAL

ToDo!

### Blinky Example

```
/* Initialise pin as an output, setting it high */
hal_gpio_init_out(LED_BLINK_PIN, 1);

while(1) {
    hal_gpio_toggle(LED_BLINK_PIN);
    os_time_delay(OS_TICKS_PER_SEC*1);
}
```

### Input Example
```
/* Initialise pin as an input, pullup enabled */
hal_gpio_init_in(MY_PIN, HAL_GPIO_PULL_UP);

while(1) {
    int pin_state = hal_gpio_read(MY_PIN);
    os_time_delay(OS_TICKS_PER_SEC*1);
}
```
