#ifndef HAL_DMA_IF_H
#define HAL_DMA_IF_H

#include "hal_dma.h"

typedef void dma_trans_t(void *data, unsigned int chan,
                         void *src, void *dst, unsigned int n,
                         dma_attr_t flags);
typedef void dma_set_chan_t(void *data, unsigned int chan, unsigned int devid);
typedef void dma_en_t(void *data, unsigned int chan, int en);
typedef void dma_start_t(void *data, unsigned int chan);
typedef void dma_irq_ack_t(void *data, unsigned int chan);

typedef struct {
  dma_trans_t *trans;
  dma_set_chan_t *set_chan;
  dma_en_t *en;
  dma_start_t *start;
  dma_irq_ack_t *irq_ack;
} dma_controller_t;

typedef struct {
  dma_controller_t *cont;
  void *data;
} dma_cont_data_t;

extern dma_cont_data_t dma_cont_data[];
extern unsigned int dma_cont_data_len;

int dma_controller_reg(unsigned int num, dma_controller_t *cont, void *data);

#endif
