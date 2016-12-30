# TASK NAME

- **Display Name**: Public task name (as registered with the scheduler)
- **Priority**: 1..254
- **Stack Size**: 256 (1024 bytes)

Short description of the task.

## Overview

Longer overview of exactly what this task does, a summary of code flow, etc.

## Events

Indicate if this task has a dedicated event queue, or if it depends on an
externally defined event queue to receive events.

### Event Queue Arguments

If a dedicated event queue is used, and an optional `arg` parameter is used,
define the format of the option event arg here:

```
enum mytask_event_code {
    MYTASK_EVENT1 = 0,
    MYTASK_EVENT2,
};

struct mytask_event_arg {
    enum mytask_event_code code;
    union {
        int i;
        float f;
        uint8_t[4] a;
    };
};
```

### Event List

List all events that can be used by the task handler.

- `TASK_RSSI_CHANGED_EVT`: Description of the event
- `TASK_CONN_STAT_CHANGED_EVT`: Description of the event

## Semaphores and Mutexes

Any semaphores or mutexes used by this task.

## Dependencies

### Related Tasks

List of related tasks.

### Related Packages

List of related packages used by this task.

## Notes

Any additional notes about this task.
