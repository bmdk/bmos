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

#include "bmos_reg.h"
#include "bmos_sem.h"
#include "bmos_task_priv.h"
#include "fast_log.h"
#include "hal_int_cpu.h"
#include "xassert.h"

bmos_sem_t *sem_create(const char *name, unsigned int count)
{
  bmos_sem_t *s = malloc(sizeof(bmos_sem_t));

  if (!s)
    return NULL;

  s->count = count;
  s->name = name;
  s->waiters.first = 0;
  s->waiters.last = 0;

  bmos_reg(BMOS_REG_TYPE_SEM, (void *)s);

  return s;
}

unsigned int sem_count(bmos_sem_t *s)
{
  unsigned int saved, count;

  saved = interrupt_disable();

  count = s->count;

  interrupt_enable(saved);

  return count;
}

void sem_post(bmos_sem_t *s)
{
  unsigned int saved;

  FAST_LOG('S', "sem_post '%s'\n", s->name, 0);

  saved = interrupt_disable();

  s->count++;

  _waiters_wake_first(&s->waiters);

  interrupt_enable(saved);
}

void sem_wait(bmos_sem_t *s)
{
  (void)sem_wait_ms(s, -1);
}

int sem_wait_ms(bmos_sem_t *s, int tms)
{
  unsigned int saved;
  int status = TASK_STATUS_OK;

  FAST_LOG('S', "sem_wait_msS '%s' %d\n", s->name, tms);

  saved = interrupt_disable();

  if (tms == 0) {
    if (s->count == 0)
      status = TASK_STATUS_TIMEOUT;
    else {
      s->count--;
      status = TASK_STATUS_OK;
    }
    goto exit;
  }

  while (s->count == 0 && status != TASK_STATUS_TIMEOUT) {
    _waiters_add(&s->waiters, CURRENT, tms);

    schedule();

    interrupt_enable(saved);

    __ISB();
    __DSB();
    asm volatile ("" : : : "memory");

    saved = interrupt_disable();

    status = CURRENT->status;
  }

  XASSERT(status == TASK_STATUS_TIMEOUT || status == TASK_STATUS_OK);

  if (status == TASK_STATUS_TIMEOUT)
    _waiters_remove(&s->waiters, CURRENT);
  else {
    XASSERT(s->count);

    s->count--;
  }

exit:
  interrupt_enable(saved);

  FAST_LOG('S', "sem_wait_msE '%s' %d\n", s->name, status);

  return status;
}
