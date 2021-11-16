#ifndef BMOS_TASK_PRIV_H
#define BMOS_TASK_PRIV_H

#include "bmos_task.h"

#define POISON_VAL 0x5aa5f00f

#define MAX_TASK_TLS 2

#define TASK_STATUS_INVALID 1
#define TASK_STATUS_OK 0
#define TASK_STATUS_TIMEOUT -1

struct _bmos_task_t {
  void *sp;
  unsigned char prio;
  unsigned char state;
  signed char status;
  unsigned char pad0[1];
  int sleep;
  bmos_task_t *next;
  bmos_task_t *next_waiter;
#if MAX_TASK_TLS
  void *tls[MAX_TASK_TLS];
#endif
  const char *name;
  void *stack;
  unsigned int stack_size;
  unsigned int start;
  unsigned int time;
};

typedef struct {
  bmos_task_t *first;
  bmos_task_t *last;
} bmos_task_list_t;

struct _bmos_sem_t {
  bmos_task_list_t waiters;
  unsigned int count;
  const char *name;
};

struct _bmos_mutex_t {
  bmos_task_list_t waiters;
  unsigned int count;
  const char *name;
  bmos_task_t *owner;
};

char task_state_to_char(unsigned int state);

typedef struct {
  unsigned int tot;
  unsigned int max;
  unsigned int count_switch;
  unsigned int count_sched;
} sched_info_t;

extern sched_info_t sched_info;

#include "bmos_task.h"

typedef struct {
  bmos_task_t *task_list;
  bmos_task_t *current;
  bmos_task_t *next;
} sched_data_t;

extern sched_data_t sched_data;

#define CURRENT sched_data.current
#define NEXT sched_data.next
#define TASK_LIST sched_data.task_list

void schedule(void);

void _waiters_add(bmos_task_list_t *waiters, bmos_task_t *t, int tms);
void _waiters_remove(bmos_task_list_t *waiters, bmos_task_t *c);
void _waiters_wake_first(bmos_task_list_t *waiters);

#endif

