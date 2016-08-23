# bletiny commands

The following commands can be used with the `bletiny` app to test devices:

### Advertise as a Peripheral

```
b set addr=c1:00:00:00:00:01
b set adv_data uuid16=0x1811 uuid16=0x1812 name=nrf1
b adv conn=und disc=gen
```
