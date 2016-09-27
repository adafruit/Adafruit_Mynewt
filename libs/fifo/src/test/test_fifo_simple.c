#include "testutil/testutil.h"
#include "test_fifo.h"

TEST_CASE(test_fifo_no_init) {
  /* Force this test to fail as a test */
  TEST_ASSERT(0, "Force failure as a proof of concept: %s", "test_fifo");
}
