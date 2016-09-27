# Unit Testing with libs/testutil

Notes on creating and running unit tests with `libs/testutil`.

See http://mynewt.apache.org/os/tutorials/unit_test/ for further details.

## Add testutil reference

Before any tests can be run, you will need to add `libs/testutil` to your
`pkg.yml` file as follows:

```
pkg.deps.TEST:
   - libs/testutil
```

## Create test folder

Then you need to create the correct folder structure by adding a `test`
folder in the `libs/[module_name]/src` folder, for example:

```
├── include
│   └── adafruit
│       └── fifo.h
├── pkg.yml
└── src
    ├── fifo.c
    └── test
        ├── test_fifo.c
        └── test_fifo.h
```

## Create a test suite

In the `test_fifo.c` file you need to create a test suite that will hold all
of the unit tests to run, which can be done via the following template:

```
#include <testutil/testutil.h>
#include "test_fifo.h"

TEST_SUITE(test_fifo_suite) {
    /* empty for now, add test cases later */
}

#ifdef MYNEWT_SELFTEST
int
main(int argc, char **argv)
{
    tu_config.tc_print_results = 1;
    tu_init();
    test_fifo_suite();
    return tu_any_failed;
}
#endif
```

## Run the (empty) test suite

You can test this by running the following command:

```
$ newt test libs/fifo
```

If everything is properly configured mynewt will use the `sim` compiler config
to build a native application and run the test suite locally:

```
Testing package libs/fifo
Compiling hal_bsp.c
Compiling os_bsp.c
Compiling sbrk.c
Archiving native.a
Compiling flash_map.c
...
Linking test_fifo
Executing test: [localpath]/bin/unittest/libs/fifo/test_fifo
Passed tests: [libs/fifo]
All tests passed
```

See the sub-section below if you got an error trying to build and run the
test suite.

### GCC compiler error(s)

If you get an error like this your GCC setup may not be properly configured:

```
$ newt test libs/fifo
Testing package libs/fifo
Compiling hal_bsp.c
hal_bsp.c:20:19: fatal error: stdio.h: No such file or directory
compilation terminated.
Error: Test failure(s):
Passed tests: []
Failed tests: [libs/fifo]
```

Make sure that `*gcc-5` is available on the system path, or update to use the
native compiler you have on your system, as described below:

#### To use GCC-6 instead of the default GCC-5

Change `repos/apache-mynewt-core/compiler/sim/compiler.yml` as follows:

```
# OS X.
compiler.path.cc.DARWIN.OVERWRITE: "/usr/local/bin/gcc-6"
compiler.path.as.DARWIN.OVERWRITE: "/usr/local/bin/gcc-6 -x assembler-with-cpp"
compiler.path.objdump.DARWIN.OVERWRITE: "gobjdump"
compiler.path.objcopy.DARWIN.OVERWRITE: "gobjcopy"
compiler.flags.base.DARWIN: >
    -DMN_OSX
compiler.ld.resolve_circular_deps.DARWIN.OVERWRITE: false
```

#### To use clang (OS X) for native compiling

Change `repos/apache-mynewt-core/compiler/sim/compiler.yml` as follows:

```
# OS X.
#compiler.path.cc.DARWIN.OVERWRITE: "/usr/local/bin/gcc-5"
#compiler.path.as.DARWIN.OVERWRITE: "/usr/local/bin/gcc-5 -x assembler-with-cpp"
#compiler.path.objdump.DARWIN.OVERWRITE: "gobjdump"
#compiler.path.objcopy.DARWIN.OVERWRITE: "gobjcopy"
compiler.flags.base.DARWIN: >
    -DMN_OSX
compiler.ld.resolve_circular_deps.DARWIN.OVERWRITE: false
```

## Adding unit tests

To add meaningful unit tests you need to create your function prototype(s) in
the `test_fifo.h` header file:

```
#ifndef TEST_FIFO_H
#define TEST_FIFO_H

TEST_CASE_DECL(test_fifo_no_init);

#endif /* TEST_FIFO_H */
```

Then create one or more .c files that will hold the actual unit tests (we use
`test_fifo_simple.c` in this example):

```
#include "testutil/testutil.h"
#include "test_fifo.h"

TEST_CASE(test_fifo_no_init) {
}
```

In the test suite (defined in `test_fifo.c`) you then need to include a
reference to the test function:

```
TEST_SUITE(test_fifo_suite) {
    test_fifo_no_init();
}
```

You can run this test suite again with the following command:

```
$ newt test lib/fifo
$ newt test libs/fifo
Testing package libs/fifo
Compiling test_fifo.c
Compiling test_fifo_simple.c
Archiving fifo.a
Linking test_fifo
Executing test: [localpath]/bin/unittest/libs/fifo/test_fifo
Passed tests: [libs/fifo]
All tests passed
```

## Add test

The last step is the add actual test code that can pass or fail in your newt
unit test function.

The following `TEST_ASSERT` statement, for example, will cause the unit test
to fail since it resolves to `false`:

```
TEST_CASE(test_fifo_no_init) {
    TEST_ASSERT(0);
}
```

Running the test command at this point will generate:

```
$ newt test libs/fifo
Testing package libs/fifo
Compiling test_fifo_simple.c
Archiving fifo.a
Linking test_fifo
Executing test: /Users/ktown/Dropbox/microBuilder/code/nRF52/Mynewt/Adafruit_Mynewt/bin/unittest/libs/fifo/test_fifo
Test failure (libs/fifo):
[FAIL] test_fifo_suite/test_fifo_no_init |test_fifo_simple.c:5| failed assertion: 0
Error: Test failure(s):
Passed tests: []
Failed tests: [libs/fifo]
```

## Further Reading

See the **design.txt** file in `apache-mynewt-core/libs/testutil` for a list
of the various ASSERT commands available when creating your unit tests.

Some of the key ASSERT macros are shown below for convenience sake though:

- `TEST_ASSERT(expression, fail_msg, ...)``
  - Ex: `TEST_ASSERT(num_blocks == 1024, "expected: 1024 blocks; got: %d", num_blocks);`
- `TEST_ASSERT_FATAL(expression)``
  - Failure causes the current test case to be aborted.
- `TEST_ASSERT_FATAL(expression, fail_msg, ...)``
- `TEST_PASS(msg, ...)``
  - Reports a success result for the current test and stops executing the test.
