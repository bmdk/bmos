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

#ifndef BMOS_OP_MSG_H
#define BMOS_OP_MSG_H

#include "bmos_msg.h"
#include "bmos_queue.h"

typedef struct {
  unsigned short op;
  unsigned short len;
} bmos_op_msg_t;

#define BMOS_OP_MSG_GET_DATA(_msg_) ((void *)((bmos_op_msg_t *)(_msg_) + 1))
#define BMOS_OP_MSG_GET_MSG(_data_) ((bmos_op_msg_t *)(_data_) - 1)
#define BMOS_OP_MSG_SIZE(_msg_) (((bmos_msg_t *)(_msg_) - 1)->len - \
                                 sizeof(bmos_op_msg_t))

int op_msg_put(bmos_queue_t *queue, bmos_op_msg_t *msg, unsigned int op,
               unsigned int len);

bmos_op_msg_t *op_msg_get(bmos_queue_t *queue);

bmos_op_msg_t *op_msg_wait(bmos_queue_t *queue);

bmos_op_msg_t *op_msg_wait_ms(bmos_queue_t *queue, int tms);

int op_msg_return(bmos_op_msg_t *msg);

bmos_queue_t *op_msg_pool_create(const char *name, int type,
                                 unsigned int cnt, unsigned int size);

#endif
