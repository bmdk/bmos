#ifndef BMOS_MSG_QUEUE_H
#define BMOS_MSG_QUEUE_H

#include "bmos_msg.h"
#include "bmos_queue.h"

int msg_put(bmos_queue_t *queue, bmos_msg_t *msg);

bmos_msg_t *msg_get(bmos_queue_t *queue);

bmos_msg_t *msg_wait(bmos_queue_t *queue);

bmos_msg_t *msg_wait_ms(bmos_queue_t *queue, int tms);

int msg_return(bmos_msg_t *msg);

bmos_queue_t *msg_pool_create(const char *name, int type,
                              unsigned int cnt, unsigned int size);

#endif
