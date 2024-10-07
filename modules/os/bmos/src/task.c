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

#include <stdlib.h>
#include <string.h>

#include "bmos_reg.h"
#include "bmos_task.h"
#include "bmos_task_priv.h"
#include "common.h"
#include "cortexm.h"
#include "fast_log.h"
#include "hal_int.h"
#include "hal_time.h"
#include "io.h"
#include "shell.h"
#include "xassert.h"
#include "xtime.h"

volatile xtime_ms_t systick_count = 0;

#define TASK_STATE_EXIT 0
#define TASK_STATE_RUN 1
#define TASK_STATE_SLEEP 2

static inline void _task_yield()
{
  trigger_pendsv();
}

#define XPSR_THUMB BIT(24)

sched_data_t sched_data = { 0 };

static void idle_task(void *arg)
{
  for (;;)
    __WFI();
}

void task_start(void)
{
  bmos_task_t *t;

  t = task_init(idle_task, NULL, "idle", 0, 0, 64);

  set_psp((unsigned char *)t->sp + sizeof(sw_stack_frame_t));

  CURRENT = t;
  NEXT = t;

  _task_yield();

  INTERRUPT_ON();
  __ISB();
}

static void task_cleanup(void)
{
  CURRENT->state = TASK_STATE_EXIT;
  schedule();
  /* actually cleanup here - remove task from run list */
  for (;;)
    ;
}

static void *hw_task_init(task_fun_t *tf, void *ta, void *sp)
{
  stack_frame_t *sf = ((stack_frame_t *)sp) - 1;
  sw_stack_frame_t *ssf;

  sf->r0 = (unsigned int)ta;
  sf->r1 = 0;
  sf->r2 = 0;
  sf->r3 = 0;
  sf->r12 = 0;
  sf->lr = (unsigned int)task_cleanup;
  sf->pc = (unsigned int)tf;
  sf->xpsr = XPSR_THUMB;

  ssf = ((sw_stack_frame_t *)sf) - 1;
  memset(ssf, 0, sizeof(sw_stack_frame_t));

  return (void *)ssf;
}

bmos_task_t *task_init(task_fun_t *tf, void *ta, const char *name,
                       unsigned int prio,
                       void *stack, unsigned int stack_size)
{
  void *sp;
  bmos_task_t *t;

  t = calloc(sizeof(bmos_task_t), 1);
  if (!t) {
    xprintf("no memory for task allocation");
    return NULL;
  }

  if (!stack) {
    /* ensure alignment to double word */
    stack_size = ALIGN(stack_size, 3);

    stack = malloc(stack_size);
    if (!stack) {
      xprintf("no memory for stack allocation");
      return NULL;
    }
  }

#if CONFIG_POISON_STACK
  {
    unsigned int i;
    unsigned int *sp_int = (unsigned int *)stack;

    for (i = 0; i < stack_size >> 2; i++)
      *sp_int++ = POISON_VAL;
  }
#endif

  sp = hw_task_init(tf, ta, (unsigned char *)stack + stack_size);

  t->sp = sp;
  t->stack = stack;
  t->stack_size = stack_size;
  t->state = TASK_STATE_RUN;
  t->prio = prio;
  t->name = name;

  if (TASK_LIST == NULL)
    TASK_LIST = t;
  else {
    bmos_task_t *p = NULL;
    bmos_task_t *n = TASK_LIST;

    while (n) {
      if (prio > n->prio) {
        if (!p)
          TASK_LIST = t;
        else
          p->next = t;
        t->next = n;
        break;
      }
      p = n;
      n = n->next;
    }
    if (!n)
      p->next = t;
  }

  bmos_reg(BMOS_REG_TYPE_TASK, (void *)t);

  return t;
}

void task_delay(int time)
{
  unsigned int saved;

  if (time == 0)
    return;

  saved = interrupt_disable();

  if (time != 0) {
    CURRENT->sleep = time;
    CURRENT->state = TASK_STATE_SLEEP;
    CURRENT->status = TASK_STATUS_INVALID;
  }

  schedule();

  interrupt_enable(saved);
}

void task_wake(bmos_task_t *t)
{
  unsigned int saved;

  saved = interrupt_disable();

  if (t->state == TASK_STATE_SLEEP) {
    t->state = TASK_STATE_RUN;
    t->status = TASK_STATUS_TIMEOUT;
    schedule();
  }

  interrupt_enable(saved);
}

sched_info_t sched_info;

void schedule(void)
{
  bmos_task_t *t;
  unsigned int start, diff;

  start = hal_time_us();

  /* find next task */
  t = TASK_LIST;
  while (t) {
    if (t->state == TASK_STATE_RUN) {
      /* round robin of tasks at this priority level */
      bmos_task_t *c = CURRENT;
      if ((t->prio == c->prio) && c->next && (c->next->prio == c->prio)) {
        c = c->next;
        while (c && (c->prio == t->prio)) {
          if (c->state == TASK_STATE_RUN) {
            t = c;
            break;
          }
          c = c->next;
        }
      }
      break;
    }
    t = t->next;
  }

  if (t != CURRENT) {
    NEXT = t;

    _task_yield();
  }

  diff = hal_time_us() - start;

  sched_info.tot += diff;
  if (diff > sched_info.max)
    sched_info.max = diff;

  sched_info.count_sched++;
}

static void _task_periodic(void)
{
  bmos_task_t *t;

  /* handle sleep */
  for (t = TASK_LIST; t; t = t->next) {
    if ((t->state == TASK_STATE_SLEEP) && (t->sleep > 0)) {
      t->sleep--;
      if (t->sleep == 0) {
        t->state = TASK_STATE_RUN;
        t->status = TASK_STATUS_TIMEOUT;
      }
    }
  }

  schedule();
}

void *_pendsv_handler(void *sp)
{
  unsigned int now;

  CURRENT->sp = sp;

  now = hal_time_us();

  CURRENT->time += now - CURRENT->start;
  NEXT->start = now;

  FAST_LOG('T', "task switch '%s' -> '%s'\n", CURRENT->name, NEXT->name);

  CURRENT = NEXT;

  sched_info.count_switch++;

  return CURRENT->sp;
}

void systick_handler(void)
{
  systick_count++;

  systick_hook();

  _task_periodic();
}

void svc_handler(void)
{
}

void _waiters_add(bmos_task_list_t *waiters, bmos_task_t *t, int tms)
{
  if (!waiters->first) {
    waiters->first = t;
    waiters->last = t;
  } else {
    waiters->last->next_waiter = t;
    waiters->last = t;
  }
  t->next_waiter = 0;
  if (tms == 0)
    t->status = TASK_STATUS_TIMEOUT;
  else {
    t->state = TASK_STATE_SLEEP;
    t->sleep = tms;
    t->status = TASK_STATUS_INVALID;
  }
}

void _waiters_remove(bmos_task_list_t *waiters, bmos_task_t *c)
{
  bmos_task_t *t, *p = 0;

  t = waiters->first;

  while (t) {
    if (t == c) {
      if (p)
        p->next_waiter = t->next_waiter;
      else
        waiters->first = t->next_waiter;
      t->next_waiter = 0;
      break;
    }
    p = t;
    t = t->next_waiter;
  }
}

void _waiters_wake_first(bmos_task_list_t *waiters)
{
  bmos_task_t *t;

  t = waiters->first;

  if (t) {
    if (t->state == TASK_STATE_SLEEP) {
      waiters->first = t->next_waiter;
      t->state = TASK_STATE_RUN;
      t->status = TASK_STATUS_OK;
      t->next_waiter = 0;
      schedule();
    }
  }
}

static void *_task_get_tls(bmos_task_t *t, unsigned int n)
{
#if !MAX_TASK_TLS
  return NULL;
#else
  XASSERT(n < MAX_TASK_TLS);

  return t->tls[n];
#endif
}

static void _task_set_tls(bmos_task_t *t, unsigned int n, void *data)
{
#if MAX_TASK_TLS
  XASSERT(n < MAX_TASK_TLS);

  t->tls[n] = data;
#endif
}

void *task_get_tls(unsigned int n)
{
  return _task_get_tls(CURRENT, n);
}

void task_set_tls(unsigned int n, void *data)
{
  _task_set_tls(CURRENT, n, data);
}

bmos_task_t *task_get_current(void)
{
  return CURRENT;
}

char task_state_to_char(unsigned int state)
{
  char state_char[] = "xrs";

  if (state > TASK_STATE_SLEEP)
    return 'I';
  return state_char[state];
}
