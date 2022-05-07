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

#ifndef BMOS_MSG_H
#define BMOS_MSG_H

#include <bmos_queue.h>

typedef struct _bmos_msg_t bmos_msg_t;

struct _bmos_msg_t {
  bmos_msg_t *next;
  bmos_queue_t *home;
  bmos_queue_t *queue;
  unsigned int len;
};

#define BMOS_MSG_GET_DATA(_msg_) ((void *)((bmos_msg_t *)(_msg_) + 1))

#define BMOS_MSG_GET_MSG(_msg_) ((void *)((bmos_msg_t *)(_msg_) - 1))

bmos_msg_t *msg_create(bmos_queue_t * queue, unsigned int len);
void msg_destroy(bmos_queue_t * queue, unsigned int len);

void msg_init(bmos_msg_t *msg, bmos_queue_t *queue, unsigned int len);

#endif
