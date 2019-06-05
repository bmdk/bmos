#ifndef BMOS_MUTEX_H
#define BMOS_MUTEX_H

typedef struct _bmos_mutex_t bmos_mutex_t;

bmos_mutex_t *mutex_create(const char *name);
int mutex_lock_ms(bmos_mutex_t *s, int tms);
void mutex_lock(bmos_mutex_t *s);
void mutex_unlock(bmos_mutex_t *s);

#endif
