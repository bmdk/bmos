#ifndef BMOS_TASK_H
#define BMOS_TASK_H

typedef struct _bmos_task_t bmos_task_t;
typedef void task_fun_t (void *arg);

void task_start(void);
void task_delay(int time);
void task_wake(bmos_task_t *t);
bmos_task_t *task_init(task_fun_t *tf, void *ta,
                       const char *name, unsigned int prio,
                       void *stack, unsigned int stack_size);

#define TLS_IND_STDOUT 0

void *task_get_tls(unsigned int n);
void task_set_tls(unsigned int n, void *data);
bmos_task_t *task_get_current(void);

void systick_hook();

#endif
