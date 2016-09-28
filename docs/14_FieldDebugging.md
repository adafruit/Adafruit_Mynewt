# Debugging Devices in the Field

Debugging devices deployed remotely is always a challenge but this documentation
gives some basic techniques to help debug issues remotely.

## Debugging Crash Dumps

If a task crashes (enter the fault_handler etc.) it will normally generate a
simple dump of the system registers, which can be useful to find the cause
of the problem if you have the matching .elf file locally.

This example uses the `crash_test` library from apache-mynewt-core to
simulate various crash events locally via the `crash` command from shell:

```
crash div0
6976:Unhandled interrupt (3), exception sp 0x2000a960
6976: r0:0x00000000  r1:0x000242bd  r2:0x00000000  r3:0x0000002a
6976: r4:0x00000000  r5:0x2000c002  r6:0x2000bffc  r7:0x00025e34
6976: r8:0x0000d04d  r9:0x0000000c r10:0x20009068 r11:0x55555556
6976:r12:0x00000000  lr:0x00009e45  pc:0x00009e54 psr:0x61000000
6976:ICSR:0x00419803 HFSR:0x40000000 CFSR:0x02000000
6976:BFAR:0xe000ed38 MMFAR:0xe000ed34
```

In the example above we see the output of a divide by zero crash.

The important register value is **`pc`**. Make a note of this address since your
will use it in one of the debugging methods described below:

### Option 1: Debugging Crash Dumps with GDB

If you have access to the matching app revision, build the target and deploy it
to the device under test:

```
$ newt build throughput
$ newt create-image throughput 0.1.0
$ newt load throughput
```

To start GDB run `newt debug throughput`, changing `throughput` to whatever
your app is called. This will start up the GDB server and connect to the device
under test.

> See [GDB Debugging](07_GDBDebugging.bd) for details on using GDB.

Run the following commands from the GDB shell:

```
(gdb) monitor go
(gdb) list *0x00009e54
0x9e54 is in crash_device (crash_test.c:46).
41	    if (!strcmp(how, "div0")) {
42
43	        val1 = 42;
44	        val2 = 0;
45
46	        val3 = val1 / val2;
47	        console_printf("42/0 = %d\n", val3);
48	    } else if (!strcmp(how, "jump0")) {
49	        ((void (*)(void))0)();
50	    } else if (!strcmp(how, "ref0")) {
```

You can see here that line 46 of crash_test.c caused the fault, which is
where the divide by zero error occurs.

### Option 2: Debugging Crash Dumps with `objdump`

If you have the .elf file but can't use GDB debugger you can see the code for
the specified address from the command line using the `objdump` tool that is
part of GCC.

From the command-line (with GCC available as part of the system path) run the
following command:

> Note: You must specify a `--stop-address` that is higher than the
`--start-address` with this command, but you can increment the hex value by 1
byte to return only the line of code that caused the crash. You can play
with the start and stop addresses to provide some context to the error.

```
$ arm-none-eabi-objdump -S --disassemble --start-address=0x00009e54 --stop-address=0x00009e55 bin/throughput/apps/throughput/throughput.elf

bin/throughput/apps/throughput/throughput.elf:     file format elf32-littlearm


Disassembly of section .text:

00009e54 <crash_device+0x1c>:
    if (!strcmp(how, "div0")) {

        val1 = 42;
        val2 = 0;

        val3 = val1 / val2;
    9e54:	fb93 f3f2 	sdiv	r3, r3, r2
```
