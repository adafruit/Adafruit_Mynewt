# TASK NAME

- **Display Name**: Public task name (as registered with the scheduler)
- **Priority**: 1..254
- **Stack Size**: 256 (1024 bytes)

Short description of the task.

## Overview

Longer overview of exactly what this task does, a summary of code flow, etc.

## Dependencies

### Related Tasks

List of related tasks handlers.

### Related Packages

List of packages used by this task.

## Events

### Input Event Queues

The following event queues are executed as inputs in this task's context:

#### g_internal_event_queue_name

Short description of what this event queue is used for, followed by a list of
all implemented callback event handlers.

##### int_event_callback_handler

###### Arg Format

```
struct mytask_event_arg {
    enum mytask_event_code code;
    union {
        int i;
        float f;
        uint8_t[4] a;
    };
};
```

###### Arg Values

```
enum mytask_event_code {
    MYTASK_EVENT1 = 0,
    MYTASK_EVENT2,
};
```

The `code` values accepted by this event handler have the following meaning:

- `MYTASK_EVENT1`: Description/meaning of this event flag.
- `MYTASK_EVENT2`: Description/meaning of this event flag.

### Output Events Queues

The following external event queues receive events from this task:

#### [g_external_event_queue_name](taskname.md)

##### ext_event_callback_handler

Description of why this callback handler is fired, and any arg values
passed in when calling it:

- `TASK_OUTPUT_EVENT`: Description of this event arg.

## Semaphores and Mutexes

Any semaphores or mutexes used by this task.

## Notes

Any additional notes about this task.
