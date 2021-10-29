#ifndef HAL_DMA_H
#define HAL_DMA_H

typedef struct {
  unsigned int ssiz : 2;
  unsigned int dsiz : 2;
  unsigned int dir : 1;
  unsigned int prio : 4;
  unsigned int sinc : 1;
  unsigned int dinc : 1;
  unsigned int irq : 1;
} dma_attr_t;

#define DMA_SIZ_1 0
#define DMA_SIZ_2 1
#define DMA_SIZ_4 2

#define DMA_DIR_TO 0
#define DMA_DIR_FROM 1

void dma_trans(unsigned int num, unsigned int chan,
               void *src, void *dst, unsigned int n, dma_attr_t flags);
void dma_set_chan(unsigned int num, unsigned int chan, unsigned int devid);
void dma_en(unsigned int num, unsigned int chan, int en);
void dma_start(unsigned int num, unsigned int chan);
void dma_irq_ack(unsigned int num, unsigned int chan);

#endif
