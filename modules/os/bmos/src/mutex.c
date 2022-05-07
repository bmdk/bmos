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
#include "bmos_mutex.h"
#include "bmos_task_priv.h"
#include "hal_int_cpu.h"
#include "xassert.h"

bmos_mutex_t *mutex_create(const char *name)
{
  bmos_mutex_t *m = calloc(1, sizeof(bmos_mutex_t));

  if (!m)
    return NULL;

  m->name = name;

  bmos_reg(BMOS_REG_TYPE_MUT, (void *)m);

  return m;
}

void mutex_unlock(bmos_mutex_t *m)
{
  unsigned int saved;

  saved = interrupt_disable();

  XASSERT(m->count > 0 && m->owner == CURRENT);

  if (--m->count == 0) {
    m->owner = NULL;
    _waiters_wake_first(&m->waiters);
  }

  interrupt_enable(saved);
}

int mutex_lock_ms(bmos_mutex_t *m, int tms)
{
  unsigned int saved;
  int status = TASK_STATUS_OK;
  bmos_task_t *t;

  saved = interrupt_disable();

  t = CURRENT;

  if (t == m->owner) {
    m->count++;
    goto exit;
  }

  while (m->count > 0 && status != TASK_STATUS_TIMEOUT) {
    _waiters_add(&m->waiters, t, tms);

    schedule();

    interrupt_enable(saved);

    __ISB();
    asm volatile ("" : : : "memory");

    saved = interrupt_disable();

    status = CURRENT->status;
  }

  XASSERT(status == TASK_STATUS_TIMEOUT || status == TASK_STATUS_OK);

  if (status == TASK_STATUS_TIMEOUT)
    _waiters_remove(&m->waiters, CURRENT);
  else {
    XASSERT(m->owner == NULL && m->count == 0);

    m->owner = CURRENT;
    m->count++;
  }

exit:
  interrupt_enable(saved);

  return status;
}

void mutex_lock(bmos_mutex_t *m)
{
  int status = mutex_lock_ms(m, -1);

  XASSERT(status == TASK_STATUS_OK);
}

