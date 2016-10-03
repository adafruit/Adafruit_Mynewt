#include <testutil/testutil.h>
#include "test_fifo.h"

#include "adafruit/fifo.h"

#define FIFO_SIZE 10

FIFO_DEF(ff_non_overwritable , FIFO_SIZE, uint32_t, false, 0);

TEST_SUITE(test_fifo_suite)
{
  test_fifo_read_from_null();
  test_fifo_write_to_null();
  test_fifo_normal();
  test_fifo_circular();
  test_fifo_empty();
  test_fifo_full();
}

#ifdef MYNEWT_SELFTEST

int main (int argc, char **argv)
{
  tu_config.tc_print_results = 1;
  tu_init();
  test_fifo_suite();
  return tu_any_failed;
}

#endif

TEST_CASE(test_fifo_read_from_null)
{
  fifo_t ff_null = { 0 };
  uint32_t dummy;
  TEST_ASSERT(!fifo_read(&ff_null, &dummy));
}

TEST_CASE(test_fifo_write_to_null)
{
  fifo_t ff_null = { 0 };
  uint32_t dummy;
  TEST_ASSERT(!fifo_write(&ff_null, &dummy));
}

TEST_CASE(test_fifo_normal)
{
  for ( uint32_t i = 0; i < FIFO_SIZE; i++ )
  {
    fifo_write(ff_non_overwritable, &i);
  }

  for ( uint32_t i = 0; i < FIFO_SIZE; i++ )
  {
    uint32_t c;
    fifo_read(ff_non_overwritable, &c);
    TEST_ASSERT(i == c);
  }
}

TEST_CASE( test_fifo_circular)
{
  FIFO_DEF(ff_overwritable, 2, uint32_t, true, 0);

  uint32_t data;

  // feed fifo to full
  data = 1;
  fifo_write(ff_overwritable, &data);  // 1
  data = 2;
  fifo_write(ff_overwritable, &data);  // 2

  // overflow data
  data = 100;
  fifo_write(ff_overwritable, &data);

  //------------- 1st read should be 2, second is 100 -------------//
  fifo_read(ff_overwritable, &data);
  TEST_ASSERT(2 == data);

  fifo_read(ff_overwritable, &data);
  TEST_ASSERT(100 == data);
}

TEST_CASE( test_fifo_empty)
{
  uint32_t dummy;
  TEST_ASSERT(fifo_empty(ff_non_overwritable));
  fifo_write(ff_non_overwritable, &dummy);
  TEST_ASSERT(!fifo_empty(ff_non_overwritable));
}

TEST_CASE(test_fifo_full)
{
  TEST_ASSERT( !fifo_full(ff_non_overwritable));

  for ( uint32_t i = 0; i < FIFO_SIZE; i++ )
  {
    fifo_write(ff_non_overwritable, &i);
  }

  TEST_ASSERT(fifo_full(ff_non_overwritable));
}

