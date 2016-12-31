# OLED Display

- **Display Name**: oled
- **Priority**: 200
- **Stack Size**: 64 (256 bytes)

Controls and updates the 128x32 pixel mono OLED display based on incoming
events.

## Overview

This task controls the HW refresh of the 128x32 pixel monochrome OLED display.

It will update the display contents based on a series of events that can be
passed in to the internal message queue, such as updating the displayed RSSI
level, battery level, or updating the 128x24 pixel user portion of the screen.

### Display Sections

The display is divided into two sections:

The **status bar section** is 128x8 pixels. This section can not be edited
directly outside this task handler, and is updated via incoming events.

The **user section** is 128x24 pixels, and can be updated by other tasks or
code in the system. The raw pixels are accessed as a 384 byte buffer (128*24/8)
that can be updated externally, followed by a display refresh request once the
buffer contents have been modified.

## Dependencies

### Related Tasks

- sysevents

### Related Packages

- hw/drivers/display/ssd1306
- lib/monogfx

## Events

### Input Event Queues

The following event queues are executed in this task's context, and can be used
to pass events into the task handler:

#### g_display_evq

This event queue is used to store and parse events related to the display, such
as updating the status bar, or refreshing the user section of the display.

The following callbacks are available as part of this task handler:

##### display_refresh_cb

This callback is used to refresh the display contents after the user section's
buffer has been updated.

No parameters are required to use this callback.

##### display_status_update_cb

This callback is used to refresh the status bar when a specific event takes
place, such as the RSSI level changing, the battery level being updated, etc.

###### Arg Format

```
struct display_status_update_event_arg {
    enum display_status_update_ev_code code;
    union {
        int32_t i;
        int16_t i1;
        int16_t i2;
        uint32_t u;
        uint16_t u1;
        uint16_t u2;
        float f;
        uint8_t a[4];
    };
};
```

###### Code Values

```
enum display_status_update_ev_code {
    DISPLAY_STATUS_UPDATE_RSSI = 0,
    DISPLAY_STATUS_UPDATE_BATTERY_LEVEL,
    DISPLAY_STATUS_UPDATE_CONNECTED,
    DISPLAY_STATUS_UPDATE_TX,
    DISPLAY_STATUS_UPDATE_RX
};
```

The `code` values accepted by this event handler have the following meaning:

- `DISPLAY_STATUS_UPDATE_RSSI`: Indicates an update to the RSSI level between
  this device and the device on the other end of the connection. This event
  uses the `u1` and `i2` values to receive the integer RSSI level, where `u1`
  indicates the connection number, and `i2` contains the RSSI level.
- `DISPLAY_STATUS_UPDATE_BATTERY_LEVEL`: Indicates that the battery level has
  changed. This events uses the `u` value to receive the battery level in
  millivolts.
- `DISPLAY_STATUS_UPDATE_CONNECTED`: Indicates that the connection state of
  the device has changed. This event uses the `u` value to receive the current
  connection state, where 0 means no connection and a positive value indicate
  the number of currently connected devices.
- `DISPLAY_STATUS_UPDATE_TX`: Indicates that some TX activity has taken place
  on the device. This event uses the `u1` and `i2` values to receive the
  number of bytes transmitted, where `u1` indicates the connection number, and
  `i2` contains the number of bytes transmitted.
- `DISPLAY_STATUS_UPDATE_RX`: Indicates that some RX activity has taken place
  on the device. This event uses the `u1` and `i2` values to receive the
  number of bytes received, where `u1` indicates the connection number, and
  `i2` contains the number of bytes received.

## Semaphores and Mutexes

Any semaphores or mutexes used by this task.

## Notes

Any additional notes about this task.
