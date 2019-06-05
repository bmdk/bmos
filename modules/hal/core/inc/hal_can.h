#ifndef HAL_CAN_H
#define HAL_CAN_H

#if BMOS
#include "bmos_queue.h"
#include "bmos_op_msg.h"
#endif

typedef struct {
  const char *name;
  void *base;
  unsigned char irq;
  unsigned char tx_irq;
  unsigned int flags;
  const char *pool_name;
  const char *tx_queue_name;
  struct {
    unsigned int overrun;
    unsigned int hw_overrun;
  } stats;
#if BMOS
  bmos_queue_t *txq;
  bmos_queue_t *rxq;
  bmos_queue_t *pool;
  bmos_op_msg_t *msg;
  unsigned char *data;
  unsigned short data_len;
  unsigned short op;
#endif
} candev_t;

typedef struct {
  unsigned short id;
  unsigned char data[8];
  unsigned char len;
} can_t;

#if BMOS
bmos_queue_t *can_open(candev_t *c, unsigned int *id, unsigned int id_len,
                       bmos_queue_t *rxq, unsigned int op);
#endif

#endif
