/* Copyright (c) 2019-2022 Brian Thomas Murphy
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

#ifndef BMOS_QUEUE_PRIV_H
#define BMOS_QUEUE_PRIV_H

#include "bmos_msg_queue.h"
#include "bmos_queue.h"
#include "bmos_sem.h"

typedef struct {
  unsigned int count;
  bmos_queue_put_f_t *put_f;
  void *put_f_data;
} queue_driver_data_t;

typedef struct {
  bmos_sem_t *sem;
} queue_task_data_t;

typedef struct {
  unsigned short cnt;
  unsigned short size;
} bmos_pool_data_t;

struct _bmos_queue_t {
  bmos_msg_t *first;
  bmos_msg_t *last;
  const char *name;
  unsigned char type;
  char pad[3];
  bmos_pool_data_t *pool_data; /* only used when the queue is a pool */
  union {
    queue_driver_data_t driver;
    queue_task_data_t task;
  } type_data;
};

#endif
