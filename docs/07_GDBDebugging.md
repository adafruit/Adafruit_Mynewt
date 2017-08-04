# GDB Debugging

The definitive guide to GDB is available here:
ftp://ftp.gnu.org/old-gnu/Manuals/gdb/html_chapter/gdb_toc.html

## Starting the Debugger

You can start GDB (GNU Debugger) with newt with the following command, with a
JLink connected to the target device:

```
# Optionally build and flash the image
$ newt build target_name
$ newt create-image target_name 0.0.0
$ newt load target_name

# Start GDB
$ newt debug target_name
```

You can then start and stop code execution on the target MCU via:

```
(gbd) monitor halt
```
and

```
(gbd) monitor go
```

You can also start a fresh run of the code via:

```
(gbd) monitor reset
(gdb) c
```

You can check if the OS is running by executing the following code, which
will display the OS time counter:

```
(gdb) p/d g_os_time
```

## Displaying Values

To display the current state of a struct or global variable, use the `print
[name]` (or `p [name]`) command.

### Basic Example

The example below halts the processor and then prints a struct instance named
`ble_phy_stats`.

> Tip: You can optionally enable pretty printing via `set print pretty on`

```
(gdb) monitor halt
(gdb) set print pretty on
(gdb) print ble_phy_stats
$5 = {
  s_hdr = {
    s_name = 0x0,
    s_size = 0 '\000',
    s_cnt = 0 '\000',
    s_pad1 = 0,
    s_next = {
      stqe_next = 0x0
    }
  },
  sphy_isrs = 0,
  stx_good = 1,
  stx_fail = 0,
  stx_late = 0,
  stx_bytes = 27,
  srx_starts = 0,
  srx_aborts = 0,
  srx_valid = 0,
  srx_crc_err = 0,
  srx_late = 0,
  sno_bufs = 0,
  sradio_state_errs = 0,
  srx_hw_err = 0,
  stx_hw_err = 0
}
```

### Formatting Display Values

You can also format the printed values with the following formatters:

- `x` Regard as integer and display as hexadecimal
- `d` Regard as integer and display as signed decimal
- `u` Regard as integer and display as unsigned decimal
- `c` Regard as integer and print as a char constant
- `f` Regard as float and print in floating point syntax
- `t` Print integer as binary
- `a` Print as an address (hex plus offset). Useful to discover where an
 address is located (ex. `p/a 0x12345` yields `$3 = 0x12345 <_initialize_vx+396>`)

To print the BLE link layer stack (`g_ble_ll_stack`) in hex enter:

```
(gdb) p/x g_ble_ll_stack
$17 = {0xdeadbeef <repeats 22 times>, 0x20002568, 0x304, 0xe000e100, 0x100, 0x20001be4, 0x0, 0xffffffff, 0x0,
  0xffffffff, 0x20002204, 0x19f14, 0x20002218, 0x0, 0x20001e90, 0x10000000, 0x20002180, 0x354, 0xa0a3, 0x92b2,
  0x61000000, 0x20001e8c, 0x200021d8, 0x0, 0x9657, 0x4, 0xffffffff, 0xffffffff, 0x1fff8000, 0x0, 0xa897, 0x0, 0xa85d,
  0x1fff8000, 0xffffffff, 0xffffffff, 0x1fff8000, 0x0, 0x0, 0x8, 0xde, 0x93c9, 0x0}
```

### Displaying an Array of Values

You can display the contents of an array as follows:

```
(gdb) monitor halt
(gdb) set print pretty on
(gdb) print *array@len
```

### Running an arnitrary function whel halted at a breakpoint

When halted at a breakpoint, you can run a function via the `call` command. Tip via [Håkon Alseth](https://devzone.nordicsemi.com/question/161648/call-function-from-gdb-at-breakpoint/?answer=161674#post-id-161674).

> Make sure to include the parenthesis after the function name when issuing the `call` command, which will cause the device to go back to the halt state once the function has completed execution.

```
arm-none-eabi-gdb _build/*.out 
(gdb) target remote :2331 
(gdb) load 
(gdb) mon reset 
(gdb) c 
<break somewhere in your code once the 500 ms routine starts, using CTRL+C> 
(gdb) call test_function()
```

### Useful Mynewt/Nimble Structs and Fields

Some useful Mynewt or nimble fields to display can be seen below:

- `ble_phy_stats` - PHY stats for traffic tracking
- `ble_ll_stats` - Link layer stats
- `ble_ll_conn_stats` - Connection stats
- `g_ble_ll_adv_sm` - Advertising state machine
- `g_ble_ll_stack` - Link layer stack

For example:

```
(gdb) monitor halt
(gdb) set print pretty on
(gdb) p ble_phy_stats
$16 = {
  s_hdr = {
    s_name = 0x0,
    s_size = 0 '\000',
    s_cnt = 0 '\000',
    s_pad1 = 0,
    s_next = {
      stqe_next = 0x0
    }
  },
  sphy_isrs = 0,
  stx_good = 1,
  stx_fail = 0,
  stx_late = 0,
  stx_bytes = 27,
  srx_starts = 0,
  srx_aborts = 0,
  srx_valid = 0,
  srx_crc_err = 0,
  srx_late = 0,
  sno_bufs = 0,
  sradio_state_errs = 0,
  srx_hw_err = 0,
  stx_hw_err = 0
}
```

## Memory Manipulation

You can display the memory contents of a specific address via the `x` command.

To see the main stack pointer location on an ARM chip, for example, run:

```
(gdb) x 0
0x0:	0x20008000
```

You can also adjust the output size with the optional `x/nfu` flags:

- `n` Indicates how much memory to display (in `u` units), default = 1
- `f` Indicates the display format, where:
  - `s` means null-terminated string
  - `i` means machine instruction
  - `x` Display as hexadecimal (default)
  - `d` Display as signed decimal
  - `u` Display as unsigned decimal
  - `c` Print as a char constant
  - `f` Print in floating point syntax
  - `t` Print integer as binary
- `u` The unit size, which can be:
  - `b` Bytes
  - `h` Halfwords (two bytes)
  - `w` Words (four bytes)
  - `g` Giant worlds (eight bytes)

> Note: Whenever you change the unit size (`u`), the updated value becomes
> the system default and will be retained on future requests until it is
> changed again.

For example, to display the same 32-bit stack pointer at address 0 on an ARM
processor as four individual bytes displayed as unsigned decimal values you
would enter:

```
(gdb) x/4ub 0
0x0:	0	128	0	32
```

To display the 32-bit value in binary notation you would enter:

```
(gdb) x/4tb 0
0x0:	00000000	10000000	00000000	00100000
```

To display 4 'words' as 32-bit values in hex notation:

```
(gdb) x/4xw 0
0x0:	0x20008000	0x00003049	0x00003099	0x00003099
```

## Stack Manipulation

The following commands can be used to work with the stack such as producing a
dump of the stack frames, or checking for stack overflow.

### Display the Stack Trace

You can display a list of function calls up to the point where the MCU halted
with the `backtrace` or `bt` command, which will dump individual stack
frame records:

```
(gdb) bt
#0  os_tick_idle (ticks=131072) at hal_os_tick.c:146
#1  0x000091f6 in os_idle_task (arg=<optimized out>) at os.c:64
#2  0x00000000 in ?? ()
```

Each line shows the frame number, and the function name and return address.
In this case, the code has stopped at `os_tick_idle` in hal_os_tick.c, which
was called from `os_idle_task` in os.c.

### Display Stack Frame Details

You can display detailed information about a specific stack frame via the
`info frame [n]` command:

```
(gdb) info frame
Stack level 0, frame at 0x20001e60:
 pc = 0x184aa in os_tick_idle (hal_os_tick.c:146); saved pc = 0x91f6
 called by frame at 0x20001e80
 source language c.
 Arglist at 0x20001e40, args: ticks=131072
 Locals at 0x20001e40, Previous frame's sp is 0x20001e60
 Saved registers:
  r3 at 0x20001e48, r4 at 0x20001e4c, r5 at 0x20001e50, r6 at 0x20001e54, r7 at 0x20001e58, r8 at 0x20001e40,
  r9 at 0x20001e44, lr at 0x20001e5c
```

To display the arguments for the current stack frame you can run:

```
(gdb) info args
ticks = 131072
```

To display the local variables (one per line) for the stack frame run (data
may or may not be available depending on build setings):

```
(gdb) info locals
ocmp = <optimized out>
```

### Displaying ARM Registers

You can also display a list of the ARM registers via `info registers`.

The following example shows the same `pc` value seen above where we are halted
at 0x184aa on `os_tick_idle`, and the stack pointer (`sp`) is at 0x20001e40,
one 32 byte (0x20) stack frame away from the value seen earlier.

```
(gdb) info registers
r0             0x800000	8388608
r1             0x4000b000	1073786880
r2             0xb4a5e0	11838944
r3             0x3fffef	4194287
r4             0xffffffff	4294967295
r5             0xffffff	16777215
r6             0x504	1284
r7             0x540	1344
r8             0xff800000	4286578688
r9             0x20000	131072
r10            0x1fff8000	536838144
r11            0x0	0
r12            0x20	32
sp             0x20001e40	0x20001e40 <g_idle_task_stack+192>
lr             0x18435	99381
pc             0x184aa	0x184aa <os_tick_idle+182>
xpsr           0x21000000	553648128
MSP            0x20008000	536903680
PSP            0x20001e40	536878656
PRIMASK        0x1	1
BASEPRI        0x0	0
FAULTMASK      0x0	0
CONTROL        0x2	2
```

## Working with Source Code

You can display source code inside GDB with some basic commands.

### Displaying a function

This will display the first 10 lines of the specified function:

```
(gdb) list os_idle_task
39	#endif
40	#define MAX_IDLE_TICKS  (600 * OS_TICKS_PER_SEC)        /* 10 minutes */
41
42	void
43	os_idle_task(void *arg)
44	{
45	    os_sr_t sr;
46	    os_time_t now;
47	    os_time_t iticks, sticks, cticks;
48
```

To display the next 10 lines enter:

```
(gdb) list
49	    /* For now, idle task simply increments a counter to show it is running. */
50	    while (1) {
51	        ++g_os_idle_ctr;
52	        OS_ENTER_CRITICAL(sr);
53	        now = os_time_get();
54	        sticks = os_sched_wakeup_ticks(now);
55	        cticks = os_callout_wakeup_ticks(now);
56	        iticks = min(sticks, cticks);
57	        if (iticks < MIN_IDLE_TICKS) {
58	            iticks = 0;
```

> Tip: You can adjust the number of lines displayed via `set listsize [count]`

### Displaying code at a specific address

If you wish to display the code at a specific address you can prefix the address
with a `*` character as shown below:

```
(gdb) list *0x00009e54
0x9e54 is in crash_device (crash_test.c:46).
41      if (!strcmp(how, "div0")) {
42
43          val1 = 42;
44          val2 = 0;
45
46          val3 = val1 / val2;
47          console_printf("42/0 = %d\n", val3);
48      } else if (!strcmp(how, "jump0")) {
49          ((void (*)(void))0)();
50      } else if (!strcmp(how, "ref0")) {
```

