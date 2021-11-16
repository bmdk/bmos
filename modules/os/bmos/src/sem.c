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

  while (s->count == 0 && status != TASK_STATUS_TIMEOUT) {
    _waiters_add(&s->waiters, CURRENT, tms);

    schedule();

    interrupt_enable(saved);

    __ISB();
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

  interrupt_enable(saved);

  FAST_LOG('S', "sem_wait_msE '%s' %d\n", s->name, status);

  return status;
}
