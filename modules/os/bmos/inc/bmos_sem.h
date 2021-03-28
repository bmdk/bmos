#ifndef BMOS_SEM_H
#define BMOS_SEM_H

typedef struct _bmos_sem_t bmos_sem_t;

bmos_sem_t *sem_create(const char *name, unsigned int count);
void sem_post(bmos_sem_t *s);
void sem_wait(bmos_sem_t *s);
int sem_wait_ms(bmos_sem_t *s, int tms);
unsigned int sem_count(bmos_sem_t *s);

#endif
