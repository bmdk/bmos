#ifndef BMOS_QUEUE_PRIV_H
#define BMOS_QUEUE_PRIV_H

#include "bmos_msg_queue.h"
#include "bmos_queue.h"
#include "bmos_sem.h"

typedef struct {
  unsigned int count;
  bmos_queue_put_f_t *put_f;
  void *put_f_data;
} queue_driver_data_t;

typedef struct {
  bmos_sem_t *sem;
} queue_task_data_t;

typedef struct {
  unsigned int cnt;
  unsigned int size;
} bmos_pool_data_t;

struct _bmos_queue_t {
  bmos_msg_t *first;
  bmos_msg_t *last;
  const char *name;
  unsigned char type;
  char pad[3];
  bmos_pool_data_t *pool_data; /* only used when the queue is a pool */
  union {
    queue_driver_data_t driver;
    queue_task_data_t task;
  } type_data;
};

#endif
