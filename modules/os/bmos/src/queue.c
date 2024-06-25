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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "bmos_msg_queue.h"
#include "bmos_queue_priv.h"
#include "bmos_reg.h"
#include "bmos_sem.h"
#include "common.h"
#include "hal_int.h"
#include "xassert.h"

#include "fast_log.h"

bmos_queue_t *queue_create(const char *name, unsigned int type)
{
  bmos_queue_t *q;

  if (type > QUEUE_TYPE_TASK)
    return NULL;

  q = calloc(1, sizeof(bmos_queue_t));
  if (!q)
    return NULL;

  q->name = name;
  q->type = type;

  if (type == QUEUE_TYPE_TASK) {
    bmos_sem_t *sem = sem_create(name, 0);
    if (!sem) {
      free(q);
      return NULL;
    }
    q->type_data.task.sem = sem;
  }

  bmos_reg(BMOS_REG_TYPE_QUEUE, (void *)q);

  return q;
}

void queue_destroy(bmos_queue_t *queue)
{
  xpanic("queue_destroy %p", queue);
}

int queue_control(bmos_queue_t *queue, int control, ...)
{
  bmos_queue_control_f_t *control_f;
  va_list ap;
  int r;

  if (queue->type != QUEUE_TYPE_DRIVER)
    return -1;

  control_f = queue->type_data.driver.control_f;

  if (!control_f)
    return -1;

  va_start(ap, control);

  r = control_f(queue->type_data.driver.put_f_data, control, ap);

  va_end(ap);

  return r;
}

bmos_queue_t *queue_lookup(const char *name)
{
  return NULL;
}

static inline bmos_msg_t *_get(bmos_queue_t *queue)
{
  bmos_msg_t *m = queue->first;

  if (m)
    queue->first = m->next;
  m->queue = NULL;
  m->next = NULL;
  return m;
}

static inline void _put(bmos_queue_t *queue, bmos_msg_t *msg)
{
  if (queue->first) {
    queue->last->next = msg;
    queue->last = msg;
  } else {
    queue->first = msg;
    queue->last = msg;
  }
  msg->next = NULL;
  msg->queue = queue;
}

static int msg_put_task(bmos_queue_t *queue, bmos_msg_t *msg)
{
  unsigned int saved;

  saved = interrupt_disable();
  _put(queue, msg);
  interrupt_enable(saved);

  sem_post(queue->type_data.task.sem);

  return 0;
}

static int msg_put_driver(bmos_queue_t *queue, bmos_msg_t *msg)
{
  unsigned int saved;
  bmos_queue_put_f_t *f;
  void *f_data;

  FAST_LOG('Q', "put %s %p\n", queue->name, msg);

  saved = interrupt_disable();

  queue->type_data.driver.count++;
  _put(queue, msg);

  f = queue->type_data.driver.put_f;
  f_data = queue->type_data.driver.put_f_data;

  interrupt_enable(saved);

  if (f)
    f(f_data);

  return 0;
}

int msg_put(bmos_queue_t *queue, bmos_msg_t *msg)
{
  XASSERT(msg->queue == NULL);

  if (queue->type == QUEUE_TYPE_TASK)
    return msg_put_task(queue, msg);
  else     /* QUEUE_TYPE_DRIVER */
    return msg_put_driver(queue, msg);
}

static bmos_msg_t *msg_wait_ms_task(bmos_queue_t *queue, int tms)
{
  bmos_msg_t *msg;

  if (sem_wait_ms(queue->type_data.task.sem, tms) == 0) {
    unsigned int saved;

    saved = interrupt_disable();
    msg = _get(queue);
    interrupt_enable(saved);
  } else
    msg = NULL;

  return msg;
}

static bmos_msg_t *msg_wait_ms_driver(bmos_queue_t *queue, int tms)
{
  unsigned int saved;
  bmos_msg_t *msg;

  saved = interrupt_disable();

  if (queue->type_data.driver.count > 0) {
    msg = _get(queue);
    XASSERT(msg);
    queue->type_data.driver.count--;
  } else
    msg = NULL;

  FAST_LOG('Q', "get %s %p\n", queue->name, msg);

  interrupt_enable(saved);

  return msg;
}

bmos_msg_t *msg_wait_ms(bmos_queue_t *queue, int tms)
{
  if (queue->type == QUEUE_TYPE_TASK)
    return msg_wait_ms_task(queue, tms);
  else     /* QUEUE_TYPE_DRIVER */
    return msg_wait_ms_driver(queue, tms);
}

bmos_msg_t *msg_get(bmos_queue_t *queue)
{
  return msg_wait_ms(queue, 0);
}

bmos_msg_t *msg_wait(bmos_queue_t *queue)
{
  return msg_wait_ms(queue, -1);
}

int msg_return(bmos_msg_t *msg)
{
  return msg_put(msg->home, msg);
}

bmos_queue_t *msg_pool_create(const char *name, int type,
                              unsigned int cnt, unsigned int size)
{
  bmos_queue_t *q;
  unsigned int i, asize;
  bmos_pool_data_t *p;
  bmos_msg_t *m;

  q = queue_create(name, type);
  if (!q)
    return NULL;

  size = ALIGN(size, 2); /* round up to to multiple of 4 */
  asize = sizeof(bmos_msg_t) + size;

  p = malloc(sizeof(bmos_pool_data_t) + cnt * asize);
  if (!p) {
    queue_destroy(q);
    return NULL;
  }

  q->pool_data = p;

  p->cnt = cnt;
  p->size = size;

  m = (bmos_msg_t *)(p + 1);

  for (i = 0; i < cnt; i++) {
    msg_init(m, q, size);
    msg_put(q, m);

    m = (bmos_msg_t *)((char *)m + asize);
  }

  return q;
}

void msg_init(bmos_msg_t *msg, bmos_queue_t *queue, unsigned int len)
{
  memset(msg, 0, sizeof(bmos_msg_t));
  msg->home = queue;
  msg->len = len;
}

int queue_set_put_f(bmos_queue_t *queue, bmos_queue_put_f_t *f,
                    bmos_queue_control_f_t *cf, void *f_data)
{
  unsigned int saved;

  if (queue->type != QUEUE_TYPE_DRIVER)
    return -1;

  saved = interrupt_disable();

  queue->type_data.driver.put_f = f;
  queue->type_data.driver.control_f = cf;
  queue->type_data.driver.put_f_data = f_data;

  interrupt_enable(saved);

  return 0;
}

const char * queue_get_name(bmos_queue_t *queue)
{
  return queue->name;
}

static unsigned int queue_count_task(bmos_queue_t *queue)
{
  return sem_count(queue->type_data.task.sem);
}

static unsigned int queue_count_driver(bmos_queue_t *queue)
{
  unsigned int saved, count;

  saved = interrupt_disable();
  count = queue->type_data.driver.count;
  interrupt_enable(saved);

  return count;
}

unsigned int queue_get_count(bmos_queue_t *queue)
{
  unsigned int count;

  if (queue->type == QUEUE_TYPE_TASK)
    count = queue_count_task(queue);
  else     /* QUEUE_TYPE_DRIVER */
    count = queue_count_driver(queue);

  return count;
}
