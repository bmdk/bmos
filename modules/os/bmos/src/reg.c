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
#include <string.h>

#include "bmos_reg.h"
#include "bmos_task.h"
#include "bmos_task_priv.h"
#include "bmos_queue.h"
#include "bmos_queue_priv.h"
#include "bmos_sem.h"
#include "bmos_mutex.h"
#include "bmos_msg.h"
#include "hal_int.h"
#include "xassert.h"

#include "shell.h"
#include "io.h"

#define BMOS_REG_CHUNK 4
#define MAX_WAITERS 8

typedef struct {
  void **list;
  unsigned short count;
  unsigned short alloc;
} reg_t;

static reg_t reg_list[BMOS_REG_TYPE_COUNT];

void bmos_reg(bmos_reg_type_t type, void *p)
{
  reg_t *r;

  if (type >= BMOS_REG_TYPE_COUNT)
    xpanic("invalid registration type %d", type);

  r = &reg_list[type];
  if (r->alloc == r->count) {
    r->alloc += BMOS_REG_CHUNK;

    r->list = realloc(r->list, r->alloc * sizeof(void *));
    XASSERT(r->list);
  }

  r->list[r->count] = p;
  r->count++;
}

void task_info(char opt, int debug)
{
  reg_t *r = &reg_list[BMOS_REG_TYPE_TASK];
  unsigned int i;

  if (opt == 'r') {
    for (i = 0; i < r->count; i++) {
      bmos_task_t *t = (bmos_task_t *)r->list[i];
      t->time = 0;
    }
    return;
  }


  if (debug)
    debug_printf("name       stack      size prio\n");
  else
    xprintf("name       stack      size prio\n");

  for (i = 0; i < r->count; i++) {
    bmos_task_t *t = (bmos_task_t *)r->list[i];

    if (debug)
      debug_printf("%-10s %p %4d %4d %c %4d.%06d\n", t->name, t->stack,
                   t->stack_size,
                   t->prio, task_state_to_char(t->state),
                   t->time / 1000000, t->time % 1000000);
    else
      xprintf("%-10s %p %4d %4d %c %4d.%06d\n", t->name, t->stack,
              t->stack_size, t->prio,
              task_state_to_char(t->state),
              t->time / 1000000, t->time % 1000000);
  }
}

void queue_info(char opt, int debug)
{
  reg_t *r = &reg_list[BMOS_REG_TYPE_QUEUE];
  unsigned int i;

  if (debug)
    debug_printf("name       typ count\n");
  else
    xprintf("name       typ count\n");

  for (i = 0; i < r->count; i++) {
    bmos_queue_t *q = (bmos_queue_t *)r->list[i];
    char type;
    unsigned int count;

    if (q->type == QUEUE_TYPE_TASK) {
      type = 't';
      count = q->type_data.task.sem->count;
    } else {
      type = 'd';
      count = q->type_data.driver.count;
    }

    if (debug)
      debug_printf("%-10s %c   %5d %s\n", q->name, type, count,
                   q->pool_data ? "pool" : "");
    else
      xprintf("%-10s %c   %5d %s\n", q->name, type, count,
              q->pool_data ? "pool" : "");
#if 1
    /* COPY!! */
    if (opt == 'd') {
#define N_MSGS 32
      bmos_msg_t *d[N_MSGS], *m;
      unsigned int saved, i, count = 0;

      saved = interrupt_disable();
      m = q->first;
      while (m) {
        if (count == N_MSGS)
          break;
        d[count++] = m;
        m = m->next;
      }

      interrupt_enable(saved);

      if (debug)
        debug_printf("  msg        next       home       queue        len\n");
      else
        xprintf("  msg        next       home       queue        len\n");

      for (i = 0; i < count; i++) {
        m = d[i];
        if (debug)
          debug_printf("  %p %p %-10s %-10s %5d\n", m, m->next, m->home->name,
                       m->queue ? m->queue->name : "none", m->len);
        else
          xprintf("  %p %p %-10s %-10s %5d\n", m, m->next, m->home->name,
                  m->queue ? m->queue->name : "none", m->len);
      }
    }
#endif
  }
}

void pool_info(char opt, int debug)
{
  reg_t *r = &reg_list[BMOS_REG_TYPE_QUEUE];
  unsigned int i;

  if (debug)
    debug_printf("name       typ count alloc size\n");
  else
    xprintf("name       typ count alloc size\n");

  for (i = 0; i < r->count; i++) {
    bmos_queue_t *q = (bmos_queue_t *)r->list[i];
    char type;
    unsigned int count;

    if (!q->pool_data)
      continue;

    if (q->type == QUEUE_TYPE_TASK) {
      type = 't';
      count = q->type_data.task.sem->count;
    } else {
      type = 'd';
      count = q->type_data.driver.count;
    }

    if (debug)
      debug_printf("%-10s %c   %5d %5d %4d\n", q->name, type, count,
                   q->pool_data->cnt, q->pool_data->size);
    else
      xprintf("%-10s %c   %5d %5d %4d\n", q->name, type, count,
              q->pool_data->cnt, q->pool_data->size);

    if (opt == 'd') {
      bmos_msg_t *m = (bmos_msg_t *)(q->pool_data + 1);
      unsigned int j;

      if (debug)
        debug_printf("  msg        next       home       queue        len\n");
      else
        xprintf("  msg        next       home       queue        len\n");

      for (j = 0; j < q->pool_data->cnt; j++) {
        if (debug)
          debug_printf("  %p %p %-10s %-10s %5d\n", m, m->next, m->home->name,
                       m->queue ? m->queue->name : "none", m->len);
        else
          xprintf("  %p %p %-10s %-10s %5d\n", m, m->next, m->home->name,
                  m->queue ? m->queue->name : "none", m->len);
        m = (bmos_msg_t *)((char *)(m + 1) + q->pool_data->size);
      }
    }
  }
}

static void show_waiters(bmos_task_list_t *wait_list, int debug)
{
  unsigned int saved, count, i;
  bmos_task_t *waiters[MAX_WAITERS], *t;

  saved = interrupt_disable();

  for (count = 0, t = wait_list->first;
       count < MAX_WAITERS && t;
       count++, t = t->next_waiter)
    waiters[count] = t;

  interrupt_enable(saved);

  if (count > 0) {
    for (i = 0; i < count; i++)
      if (debug)
        debug_printf("  %s\n", waiters[i]->name);
      else
        xprintf("  %s\n", waiters[i]->name);
  }
}

void sem_info(char opt, int debug)
{
  reg_t *r = &reg_list[BMOS_REG_TYPE_SEM];
  unsigned int i;

  if (debug)
    debug_printf("name       count\n");
  else
    xprintf("name       count\n");

  for (i = 0; i < r->count; i++) {
    bmos_sem_t *s = (bmos_sem_t *)r->list[i];

    if (debug)
      debug_printf("%-10s %d\n", s->name, s->count);
    else
      xprintf("%-10s %d\n", s->name, s->count);

    if (opt == 'd')
      show_waiters(&s->waiters, debug);
  }
}

void mutex_info(char opt, int debug)
{
  reg_t *r = &reg_list[BMOS_REG_TYPE_MUT];
  unsigned int i;

  if (debug)
    debug_printf("name       count owner\n");
  else
    xprintf("name       count owner\n");

  for (i = 0; i < r->count; i++) {
    bmos_mutex_t *m = (bmos_mutex_t *)r->list[i];
    bmos_task_t *t = m->owner;

    if (debug)
      debug_printf("%-10s %5d %s\n", m->name, m->count, t ? t->name : "");
    else
      xprintf("%-10s %5d %s\n", m->name, m->count, t ? t->name : "");

    if (opt == 'd')
      show_waiters(&m->waiters, debug);
  }
}

void show_sched_info(char opt)
{
  if (opt == 'r') {
    memset(&sched_info, 0, sizeof(sched_info));
    return;
  }

  xprintf("max(us) %u\n", sched_info.max);
  xprintf("tot(us) %u\n", sched_info.tot);
  xprintf("sched   %u\n", sched_info.count_sched);
  xprintf("switch  %u\n", sched_info.count_switch);
  if (sched_info.count_sched > 0) {
    unsigned int ns = (sched_info.tot * 1000) / sched_info.count_sched;
    xprintf("avg(ns) %u\n", ns);
  }
}

int cmd_os(int argc, char *argv[])
{
  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  case 'q':
    queue_info(argv[1][1], 0);
    break;
  case 'p':
    pool_info(argv[1][1], 0);
    break;
  case 's':
    sem_info(argv[1][1], 0);
    break;
  case 'm':
    mutex_info(argv[1][1], 0);
    break;
  default:
  case 't':
    task_info(argv[1][1], 0);
    break;
  case 'h':
    show_sched_info(argv[1][1]);
    break;
  }

  return 0;
}

SHELL_CMD_H(os, cmd_os,
"display os information\n\n"
"os q[d]: queue [details]\n"
"os p[d]: pool [details]\n"
"os s[d]: semaphore [details]\n"
"os m[d]: mutex [details]\n"
"os t: task\n"
"os h: scheduling statistics"
);
