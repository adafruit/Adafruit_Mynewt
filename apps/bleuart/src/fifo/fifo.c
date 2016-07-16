/******************************************************************************/
/*!
    @file     fifo.c
    @author   hathach (tinyusb.org)

    @section DESCRIPTION

    Light-weight FIFO buffer with basic mutex support

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2012, K. Townsend (microBuilder.eu)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/******************************************************************************/
#include "fifo.h"

static inline bool is_fifo_initalized(fifo_t* f) ATTR_ALWAYS_INLINE;

/******************************************************************************/
/*!
    @brief Read one byte out of the RX buffer.

    This function will return the byte located at the array index of the
    read pointer, and then increment the read pointer index.  If the read
    pointer exceeds the maximum buffer size, it will roll over to zero.

    @param[in]  f
                Pointer to the FIFO buffer to manipulate
    @param[in]  p_buffer
                Pointer to the place holder for data read from the buffer

    @returns TRUE if the queue is not empty
*/
/******************************************************************************/
bool fifo_read(fifo_t* f, void * p_buffer)
{
  if( !is_fifo_initalized(f) || fifo_is_empty(f) ) return false;

  ff_mutex_lock(f);

  memcpy(p_buffer,
         f->buffer + (f->rd_idx * f->item_size),
         f->item_size);
  f->rd_idx = (f->rd_idx + 1) % f->depth;
  f->count--;

  ff_mutex_unlock(f);

  return true;
}

/******************************************************************************/
/*!
    @brief This function will read n elements into the array index specified by
    the write pointer and increment the write index. If the write index
    exceeds the max buffer size, then it will roll over to zero.

    @param[in]  f
                Pointer to the FIFO buffer to manipulate
    @param[in]  p_data
                The pointer to data location
    @param[in]  count
                Number of element that buffer can afford

    @returns number of bytes read from the FIFO
*/
/******************************************************************************/
uint16_t fifo_read_n (fifo_t* f, void * p_buffer, uint16_t count)
{
  if( count == 0 ) return 0;

  uint8_t* p_buf = (uint8_t*) p_buffer;

  uint16_t len = 0;
  while( (len < count) && fifo_read(f, p_buf) )
  {
    len++;
    p_buf += f->item_size;
  }

  return len;
}

/******************************************************************************/
/*!
    @brief Reads one item without removing it from the FIFO

    @param[in]  f
                Pointer to the FIFO buffer to manipulate
    @param[in]  position
                Position to read from in the FIFO buffer
    @param[in]  p_buffer
                Pointer to the place holder for data read from the buffer

    @returns TRUE if the queue is not empty
*/
/******************************************************************************/
bool fifo_peek_at(fifo_t* f, uint16_t position, void * p_buffer)
{
  if( !is_fifo_initalized(f) || fifo_is_empty(f) || (position >= f->count) ) return false;

  uint16_t index = (f->rd_idx + position) % f->depth; // rd_idx is position=0
  memcpy(p_buffer,
         f->buffer + (index * f->item_size),
         f->item_size);

  return true;
}

/******************************************************************************/
/*!
    @brief Write one element into the RX buffer.

    This function will write one element into the array index specified by
    the write pointer and increment the write index. If the write index
    exceeds the max buffer size, then it will roll over to zero.

    @param[in]  f
                Pointer to the FIFO buffer to manipulate
    @param[in]  p_data
                The byte to add to the FIFO

    @returns TRUE if the data was written to the FIFO (overwrittable
             FIFO will always return TRUE)
*/
/******************************************************************************/
bool fifo_write(fifo_t* f, void const * p_data)
{
  if ( !is_fifo_initalized(f) || (fifo_is_full(f) && !f->overwritable) ) return false;

  ff_mutex_lock(f);

  memcpy( f->buffer + (f->wr_idx * f->item_size),
          p_data,
          f->item_size);

  f->wr_idx = (f->wr_idx + 1) % f->depth;

  if (fifo_is_full(f))
  {
    f->rd_idx = f->wr_idx; // keep the full state (rd == wr && len = size)
  }
  else
  {
    f->count++;
  }

  ff_mutex_unlock(f);

  return true;
}

/******************************************************************************/
/*!
    @brief This function will write n elements into the array index specified by
    the write pointer and increment the write index. If the write index
    exceeds the max buffer size, then it will roll over to zero.

    @param[in]  f
                Pointer to the FIFO buffer to manipulate
    @param[in]  p_data
                The pointer to data to add to the FIFO
    @param[in]  count
                Number of element
    @return Number of written elements
*/
/******************************************************************************/
uint16_t fifo_write_n(fifo_t* f, void const * p_data, uint16_t count)
{
  if ( count == 0 ) return 0;

  uint8_t* p_buf = (uint8_t*) p_data;

  uint16_t len = 0;
  while( (len < count) && fifo_write(f, p_buf) )
  {
    len++;
    p_buf += f->item_size;
  }

  return len;
}

/******************************************************************************/
/*!
    @brief Clear the fifo read and write pointers and set length to zero

    @param[in]  f
                Pointer to the FIFO buffer to manipulate
*/
/******************************************************************************/
void fifo_clear(fifo_t *f)
{
  ff_mutex_lock(f);
  f->rd_idx = f->wr_idx = f->count = 0;
  ff_mutex_unlock(f);
}

//--------------------------------------------------------------------+
// HELPER FUNCTIONS
//--------------------------------------------------------------------+
static inline bool is_fifo_initalized(fifo_t* f)
{
  if( f->buffer == NULL || f->depth == 0 || f->item_size == 0)
  {
    return false;
  }
  else
  {
    return true;
  }
}
