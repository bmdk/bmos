/* Copyright (c) 2019 Brian Thomas Murphy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "xassert.h"
#include "circ_buf.h"

void circ_buf_init(circ_buf_t *cb, unsigned int pow2)
{
  cb->len = 1U << pow2;
  cb->next_in = 0;
  cb->next_out = 0;
  cb->used = 0;
  cb->data = malloc(cb->len);
  XASSERT(cb->data);
}

int circ_buf_space(circ_buf_t *cb)
{
  unsigned short space = cb->len - cb->used;
  unsigned short to_end = cb->len - cb->next_in;

  if (space < to_end)
    return space;
  else
    return to_end;
}

int circ_buf_write(circ_buf_t *cb, const unsigned char *data, unsigned int len)
{
  unsigned short space = circ_buf_space(cb);

  if (len > space)
    len = space;

  memcpy(cb->data + cb->next_in, data, len);
  cb->next_in = (cb->next_in + len) & (cb->len - 1);
  cb->used += len;

  return len;
}

int circ_buf_read(circ_buf_t *cb, unsigned char *data, unsigned int len)
{
  unsigned short to_end = cb->len - cb->next_out;

  if (len > cb->used)
    len = cb->used;

  if (to_end < len)
    len = to_end;

  memcpy(data, cb->data + cb->next_out, len);
  cb->used -= len;
  cb->next_out = (cb->next_out + len) & (cb->len - 1);

  return len;
}
