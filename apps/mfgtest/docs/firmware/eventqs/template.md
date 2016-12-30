# Event Queue

- **Location**: path/filename.h
- **Scope**: Global

Short description of this event queue.

## Overview

Full description of this event queue and where it can be used, etc.

## Events

The following events can be inserted into the event queue:

```
enum mytask_event_code {
    MYTASK_EVENT1 = 0,
    MYTASK_EVENT2,
};
```

### MYTASK_EVENT1

Description of the event.

#### Event Arg

If the event accepts an `arg` parameter describe it here:

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

#### Notes

Any note relating to this event.

### MYTASK_EVENT2

Description of the event.

## Notes

Any notes on this event queue.
