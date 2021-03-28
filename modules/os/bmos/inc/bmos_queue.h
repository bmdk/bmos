#ifndef BMOS_QUEUE_H
#define BMOS_QUEUE_H

typedef struct _bmos_queue_t bmos_queue_t;

#define QUEUE_TYPE_DRIVER 0
#define QUEUE_TYPE_TASK 1

bmos_queue_t *queue_create(const char *name, unsigned int type);

typedef void bmos_queue_put_f_t (void *data);

int queue_set_put_f(bmos_queue_t *queue, bmos_queue_put_f_t *f, void *f_data);

void queue_destroy(bmos_queue_t *queue);

bmos_queue_t *queue_lookup(const char *name);

const char *queue_get_name(bmos_queue_t *queue);

unsigned int queue_get_count(bmos_queue_t *queue);

#endif
