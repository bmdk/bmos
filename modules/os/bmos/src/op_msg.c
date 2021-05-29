/* Copyright (c) 2019-2021 Brian Thomas Murphy
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

#include "bmos_msg.h"
#include "bmos_msg_queue.h"
#include "bmos_op_msg.h"
#include "bmos_queue.h"

int op_msg_put(bmos_queue_t *queue, bmos_op_msg_t *msg, unsigned int op,
               unsigned int len)
{
  bmos_msg_t *m = BMOS_MSG_GET_MSG(msg);

  msg->op = (unsigned short)op;
  msg->len = (unsigned short)len;

  return msg_put(queue, m);
}

bmos_op_msg_t *op_msg_get(bmos_queue_t *queue)
{
  return op_msg_wait_ms(queue, 0);
}

bmos_op_msg_t *op_msg_wait(bmos_queue_t *queue)
{
  return op_msg_wait_ms(queue, -1);
}

bmos_op_msg_t *op_msg_wait_ms(bmos_queue_t *queue, int tms)
{
  bmos_msg_t *msg = msg_wait_ms(queue, tms);

  if (!msg)
    return NULL;

  return (bmos_op_msg_t *)BMOS_MSG_GET_DATA(msg);
}

int op_msg_return(bmos_op_msg_t *msg)
{
  bmos_msg_t *m;

  m = BMOS_MSG_GET_MSG(msg);

  return msg_return(m);
}

bmos_queue_t *op_msg_pool_create(const char *name, int type,
                                 unsigned int cnt, unsigned int size)
{
  return msg_pool_create(name, type, cnt, size + sizeof(bmos_op_msg_t));
}
