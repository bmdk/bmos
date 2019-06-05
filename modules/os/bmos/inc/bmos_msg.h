#ifndef BMOS_MSG_H
#define BMOS_MSG_H

#include <bmos_queue.h>

typedef struct _bmos_msg_t bmos_msg_t;

struct _bmos_msg_t {
  bmos_msg_t *next;
  bmos_queue_t *home;
  bmos_queue_t *queue;
  unsigned int len;
};

#define BMOS_MSG_GET_DATA(_msg_) ((void *)((bmos_msg_t *)(_msg_) + 1))

#define BMOS_MSG_GET_MSG(_msg_) ((void *)((bmos_msg_t *)(_msg_) - 1))

bmos_msg_t *msg_create(bmos_queue_t * queue, unsigned int len);
void msg_destroy(bmos_queue_t * queue, unsigned int len);

void msg_init(bmos_msg_t *msg, bmos_queue_t *queue, unsigned int len);

#endif
