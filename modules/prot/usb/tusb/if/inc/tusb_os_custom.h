#ifndef _TUSB_OSAL_CUSTOM_H_
#define _TUSB_OSAL_CUSTOM_H_

#include <stdbool.h>

#include "bmos_msg_queue.h"
#include "bmos_mutex.h"
#include "bmos_queue.h"
#include "bmos_sem.h"
#include "bmos_syspool.h"
#include "bmos_task.h"
#include "common.h"
#include "xassert.h"

typedef bmos_mutex_t *osal_mutex_def_t, *osal_mutex_t;
typedef bmos_sem_t *osal_semaphore_def_t, *osal_semaphore_t;

static inline osal_mutex_t osal_mutex_create(osal_mutex_def_t* mdef)
{
  bmos_mutex_t *m = mutex_create("tusb");

  XASSERT(m);
  mdef = (osal_mutex_def_t *)m;
  return (osal_mutex_t)m;
}

static inline bool osal_mutex_lock(osal_mutex_t mutex_hdl, uint32_t msec)
{
  bmos_mutex_t *m = (bmos_mutex_t *)mutex_hdl;
  int err;

  err = mutex_lock_ms(m, msec);

  return err == 0;
}

static inline bool osal_mutex_unlock(osal_mutex_t mutex_hdl)
{
  bmos_mutex_t *m = (bmos_mutex_t *)mutex_hdl;

  mutex_unlock(m);

  return true;
}

typedef struct {
  const char *name;
  uint16_t cnt;
  uint16_t size;
  bmos_queue_t *q;
} osal_queue_def_t;

typedef osal_queue_def_t *osal_queue_t;

#define OSAL_QUEUE_DEF(_role, _name, _depth, _type) \
  osal_queue_def_t _name = { \
    .name = "q" #_name, \
    .cnt  = _depth, \
    .size = sizeof(_type) \
  }

static inline osal_queue_t osal_queue_create(osal_queue_def_t* qdef)
{
  bmos_queue_t *q;

  XASSERT(qdef->size <= SYSPOOL_SIZE);

  q = queue_create(qdef->name, QUEUE_TYPE_TASK);
  XASSERT(q);

  qdef->q = q;

  return qdef;
}

static inline bool osal_queue_receive(osal_queue_t qhdl, void *data)
{
  bmos_queue_t *q = qhdl->q;
  bmos_msg_t *m;

#if 0
  m = msg_get(q);

  if (!m)
    return false;
#else
  m = msg_wait(q);
#endif

  memcpy(data, BMOS_MSG_GET_DATA(m), qhdl->size);

  msg_return(m);

  return true;
}

static inline bool osal_queue_send(osal_queue_t qhdl, void const * data,
                                   bool in_isr)
{
  bmos_queue_t *q = qhdl->q;
  bmos_msg_t *m;
  void *mdata;

  if (in_isr)
    m = msg_get(syspool);
  else
    m = msg_wait(syspool);

  if (!m)
    return false;

  mdata = BMOS_MSG_GET_DATA(m);

  memcpy(mdata, data, qhdl->size);

  msg_put(q, m);

  return true;
}

static inline bool osal_queue_empty(osal_queue_t qhdl)
{
  return queue_get_count(qhdl->q) == 0;
}
#endif
