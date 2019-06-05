#ifndef HAL_UART_H
#define HAL_UART_H

#if BMOS
#include "bmos_queue.h"
#include "bmos_op_msg.h"
#endif

typedef struct {
  const char *name;
  void *base;
  unsigned int clock;
  unsigned char irq;
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
} uart_t;

#if BMOS
bmos_queue_t *uart_open(uart_t *u, unsigned int baud, bmos_queue_t *rxq,
                        unsigned int op);
#endif

extern uart_t debug_uart;

#endif
