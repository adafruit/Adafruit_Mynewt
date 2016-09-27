#include <testutil/testutil.h>
#include "test_fifo.h"

TEST_SUITE(test_fifo_suite) {
    test_fifo_no_init();
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
