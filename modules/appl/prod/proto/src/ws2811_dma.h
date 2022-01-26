#ifndef WS2811_DMA_H
#define WS2811_DMA_H

typedef struct {
  unsigned char wsbit;
  unsigned char wsirq;
  unsigned char wsgpio;
  unsigned char dmanum;
  unsigned char chan_tim_up;
  unsigned char devid_tim_up;
  unsigned char chan_tim_ch1;
  unsigned char devid_tim_ch1;
  unsigned char chan_tim_ch2;
  unsigned char devid_tim_ch2;

  unsigned char one;
  unsigned int compare[2];
  unsigned int buflen;
  void        *gpio_addr_set;
  void        *gpio_addr_clear;
} ws2811_dma_t;

void ws2811_dma_init(ws2811_dma_t *w, unsigned int pixels);
void ws2811_dma_tx(ws2811_dma_t *w, unsigned char *buf);
void ws2811_dma_enc_col(unsigned char *buf, unsigned int pix,
                        unsigned int val, unsigned int string);
unsigned int ws2811_dma_scale(unsigned int v, unsigned int s);

#endif
