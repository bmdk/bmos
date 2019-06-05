#ifndef BMOS_OP_MSG_H
#define BMOS_OP_MSG_H

#include "bmos_msg.h"
#include "bmos_queue.h"

typedef struct {
  unsigned short op;
  unsigned short len;
} bmos_op_msg_t;

#define BMOS_OP_MSG_GET_DATA(_msg_) ((void *)((bmos_op_msg_t *)(_msg_) + 1))
#define BMOS_OP_MSG_GET_MSG(_data_) ((bmos_op_msg_t *)(_data_) - 1)
#define BMOS_OP_MSG_SIZE(_msg_) (((bmos_msg_t *)(_msg_) - 1)->len - \
                                 sizeof(bmos_op_msg_t))

int op_msg_put(bmos_queue_t *queue, bmos_op_msg_t *msg, unsigned int op,
               unsigned int len);

bmos_op_msg_t *op_msg_get(bmos_queue_t *queue);

bmos_op_msg_t *op_msg_wait(bmos_queue_t *queue);

bmos_op_msg_t *op_msg_wait_ms(bmos_queue_t *queue, int tms);

int op_msg_return(bmos_op_msg_t *msg);

bmos_queue_t *op_msg_pool_create(const char *name, int type,
                                 unsigned int cnt, unsigned int size);

#endif
