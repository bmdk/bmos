/* Copyright (c) 2019-2021 Brian Thomas Murphy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>

#include "hal_dma_if.h"
#include "hal_common.h"
#include "shell.h"
#include "io.h"

#define DMA_CHANNELS 16

#define GPDMA_CHAN_ADC1 0
#define GPDMA_CHAN_ADC4 1
#define GPDMA_CHAN_DAC1_CH1 2
#define GPDMA_CHAN_DAC1_CH2 3
#define GPDMA_CHAN_TIM6_UPD 4
#define GPDMA_CHAN_TIM7_UPD 5
#define GPDMA_CHAN_SPI1_RX 6
#define GPDMA_CHAN_SPI1_TX 7
#define GPDMA_CHAN_SPI2_RX 8
#define GPDMA_CHAN_SPI2_TX 9
#define GPDMA_CHAN_SPI3_RX 10
#define GPDMA_CHAN_SPI3_TX 11
#define GPDMA_CHAN_I2C1_RX 12
#define GPDMA_CHAN_I2C1_TX 13
#define GPDMA_CHAN_I2C1_EVC 14
#define GPDMA_CHAN_I2C2_RX 15
#define GPDMA_CHAN_I2C2_TX 16
#define GPDMA_CHAN_I2C2_EVC 17
#define GPDMA_CHAN_I2C3_RX 18
#define GPDMA_CHAN_I2C3_TX 19
#define GPDMA_CHAN_I2C3_EVC 20
#define GPDMA_CHAN_I2C4_RX 21
#define GPDMA_CHAN_I2C4_TX 22
#define GPDMA_CHAN_I2C4_EVC 23
#define GPDMA_CHAN_USART1_RX 24
#define GPDMA_CHAN_USART1_TX 25
#define GPDMA_CHAN_USART2_RX 26
#define GPDMA_CHAN_USART2_TX 27
#define GPDMA_CHAN_USART3_RX 28
#define GPDMA_CHAN_USART3_TX 29
#define GPDMA_CHAN_UART4_RX 30
#define GPDMA_CHAN_UART4_TX 31
#define GPDMA_CHAN_UART5_RX 32
#define GPDMA_CHAN_UART5_TX 33
#define GPDMA_CHAN_LPUART1_RX 34
#define GPDMA_CHAN_LPUART1_TX 35
#define GPDMA_CHAN_SAI1_A 36
#define GPDMA_CHAN_SAI1_B 37
#define GPDMA_CHAN_SAI2_A 38
#define GPDMA_CHAN_SAI2_B 39
#define GPDMA_CHAN_OCTOSPI1 40
#define GPDMA_CHAN_OCTOSPI2 41
#define GPDMA_CHAN_TIM1_CC1 42
#define GPDMA_CHAN_TIM1_CC2 43
#define GPDMA_CHAN_TIM1_CC3 44
#define GPDMA_CHAN_TIM1_CC4 45
#define GPDMA_CHAN_TIM1_UPD 46
#define GPDMA_CHAN_TIM1_TRG 47
#define GPDMA_CHAN_TIM1_COM 48
#define GPDMA_CHAN_TIM8_CC1 49
#define GPDMA_CHAN_TIM8_CC2 50
#define GPDMA_CHAN_TIM8_CC3 51
#define GPDMA_CHAN_TIM8_CC4 52
#define GPDMA_CHAN_TIM8_UPD 53
#define GPDMA_CHAN_TIM8_TRG 54
#define GPDMA_CHAN_TIM8_COM 55
#define GPDMA_CHAN_TIM2_CC1 56
#define GPDMA_CHAN_TIM2_CC2 57
#define GPDMA_CHAN_TIM2_CC3 58
#define GPDMA_CHAN_TIM2_CC4 59
#define GPDMA_CHAN_TIM2_UPD 60
#define GPDMA_CHAN_TIM3_CC1 61
#define GPDMA_CHAN_TIM3_CC2 62
#define GPDMA_CHAN_TIM3_CC3 63
#define GPDMA_CHAN_TIM3_CC4 64
#define GPDMA_CHAN_TIM3_UPD 65
#define GPDMA_CHAN_TIM3_TRG 66
#define GPDMA_CHAN_TIM4_CC1 67
#define GPDMA_CHAN_TIM4_CC2 68
#define GPDMA_CHAN_TIM4_CC3 69
#define GPDMA_CHAN_TIM4_CC4 70
#define GPDMA_CHAN_TIM4_UPD 71
#define GPDMA_CHAN_TIM5_CC1 72
#define GPDMA_CHAN_TIM5_CC2 73
#define GPDMA_CHAN_TIM5_CC3 74
#define GPDMA_CHAN_TIM5_CC4 75
#define GPDMA_CHAN_TIM5_UPD 76
#define GPDMA_CHAN_TIM5_TRG 77
#define GPDMA_CHAN_TIM15_CC1 78
#define GPDMA_CHAN_TIM15_UPD 79
#define GPDMA_CHAN_TIM15_TRG 80
#define GPDMA_CHAN_TIM15_COM 81
#define GPDMA_CHAN_TIM16_CC1 82
#define GPDMA_CHAN_TIM16_UPD 83
#define GPDMA_CHAN_TIM17_CC1 84
#define GPDMA_CHAN_TIM17_UPD 85
#define GPDMA_CHAN_DCMI 86
#define GPDMA_CHAN_AES_IN 87
#define GPDMA_CHAN_AES_OUT 88
#define GPDMA_CHAN_HASH_IN 89
#define GPDMA_CHAN_UCPD1_TX 90
#define GPDMA_CHAN_UCPD1_RX 91
#define GPDMA_CHAN_MDF1_FLT0 92
#define GPDMA_CHAN_MDF1_FLT1 93
#define GPDMA_CHAN_MDF1_FLT2 94
#define GPDMA_CHAN_MDF1_FLT3 95
#define GPDMA_CHAN_MDF1_FLT4 96
#define GPDMA_CHAN_MDF1_FLT5 97
#define GPDMA_CHAN_ADF1_FLT0 98
#define GPDMA_CHAN_FMAC_READ 99
#define GPDMA_CHAN_FMAC_WRITE 100
#define GPDMA_CHAN_CORDIC_READ 101
#define GPDMA_CHAN_CORDIC_WRITE 102
#define GPDMA_CHAN_SAES_IN 103
#define GPDMA_CHAN_SAES_OUT 104
#define GPDMA_CHAN_LPTIM1_IC1 105
#define GPDMA_CHAN_LPTIM1_IC2 106
#define GPDMA_CHAN_LPTIM1_UE 107
#define GPDMA_CHAN_LPTIM2_IC1 108
#define GPDMA_CHAN_LPTIM2_IC2 109
#define GPDMA_CHAN_LPTIM2_UE 110
#define GPDMA_CHAN_LPTIM3_IC1 111
#define GPDMA_CHAN_LPTIM3_IC2 112
#define GPDMA_CHAN_LPTIM3_UE 113

#define GPDMA_TCF BIT(8)
#define GPDMA_HTF BIT(9)
#define GPDMA_DTEF BIT(10)
#define GPDMA_ULEF BIT(11)
#define GPDMA_USEF BIT(12)
#define GPDMA_SUSPF BIT(13)
#define GPDMA_TOF BIT(14)

typedef struct {
  reg32_t lbar;
  unsigned int pad0[2];
  reg32_t fcr;
  reg32_t sr;
  reg32_t cr;
  unsigned int pad1[10];
  reg32_t tr1;
  reg32_t tr2;
  reg32_t br1;
  reg32_t sar;
  reg32_t dar;
  reg32_t tr3;
  reg32_t br2;
  unsigned int pad2[8];
  reg32_t llr;
} stm32_gpdma_chan_t;

typedef struct {
  reg32_t seccfgr;
  reg32_t privcfgr;
  reg32_t rcfglockr;
  reg32_t misr;
  reg32_t smisr;
  unsigned int pad0[15];
  stm32_gpdma_chan_t chan[16];
} stm32_gpdma_t;

#define GPDMA_CR_EN BIT(0)
#define GPDMA_CR_RESET BIT(1)
#define GPDMA_CR_SUSP BIT(2)
#define GPDMA_CR_TCIE BIT(8)
#define GPDMA_CR_HTIE BIT(9)
#define GPDMA_CR_DTEIE BIT(10)
#define GPDMA_CR_ULEIE BIT(11)
#define GPDMA_CR_USEIE BIT(12)
#define GPDMA_CR_SUSPIE BIT(13)
#define GPDMA_CR_TOIE BIT(14)
#define GPDMA_CR_LSM BIT(16)
#define GPDMA_CR_LAP BIT(17)
#define GPDMA_CR_PRIO(_n_) (((_n_) & 0x3) << 22)

#define GPDMA_TR1_SDW(_n_) (((_n_) & 0x3) << 0)
#define GPDMA_TR1_SINC BIT(3)
#define GPDMA_TR1_SBL(_n_) (((_n_) & 0x1f) << 4)
#define GPDMA_TR1_PAM(_n_) (((_n_) & 0x3) << 11)
#define GPDMA_TR1_SBX BIT(13)
#define GPDMA_TR1_SAP BIT(14)
#define GPDMA_TR1_SSEC BIT(15)
#define GPDMA_TR1_DDW(_n_) (((_n_) & 0x3) << 16)
#define GPDMA_TR1_DINC BIT(19)
#define GPDMA_TR1_DBL(_n_) (((_n_) & 0x1f) << 20)
#define GPDMA_TR1_DBX BIT(26)
#define GPDMA_TR1_DHX BIT(27)
#define GPDMA_TR1_DAP BIT(30)
#define GPDMA_TR1_DSEC BIT(31)

#define GPDMA_TR2_SWREQ BIT(9)

void stm32_gpdma_set_chan(void *addr, unsigned int chan,
                          unsigned int devid)
{
  stm32_gpdma_t *d = addr;
  stm32_gpdma_chan_t *c = &d->chan[chan];

  reg_set_field(&c->tr2, 7, 0, devid);
}

void stm32_gpdma_irq_ack(void *addr, unsigned int chan)
{
  stm32_gpdma_t *d = addr;
  stm32_gpdma_chan_t *c = &d->chan[chan];

  c->fcr = GPDMA_TCF;
}

void stm32_gpdma_en(void *addr, unsigned int chan, int en)
{
  stm32_gpdma_t *d = addr;
  stm32_gpdma_chan_t *c = &d->chan[chan];

  if (en)
    c->cr |= GPDMA_CR_EN;
  else
    c->cr &= ~GPDMA_CR_EN;
}

void stm32_gpdma_start(void *addr, unsigned int chan)
{
  stm32_gpdma_t *d = addr;
  stm32_gpdma_chan_t *c = &d->chan[chan];

  stm32_gpdma_en(addr, chan, 0);
  c->tr2 |= GPDMA_TR2_SWREQ;
  stm32_gpdma_en(addr, chan, 1);
}

void stm32_gpdma_trans(void *addr, unsigned int chan,
                       void *src, void *dst, unsigned int n,
                       dma_attr_t attr)
{
  stm32_gpdma_t *d = addr;
  stm32_gpdma_chan_t *c = &d->chan[chan];
  unsigned int flags;

  if (chan > 15)
    return;

  c->cr &= ~GPDMA_CR_EN;

  c->sar = (unsigned int)src;
  c->dar = (unsigned int)dst;
  c->br1 = n;

  flags = GPDMA_TR1_DDW(attr.dsiz) | GPDMA_TR1_SDW(attr.ssiz);

  if (attr.sinc)
    flags |= GPDMA_TR1_SINC;

  if (attr.dinc)
    flags |= GPDMA_TR1_DINC;

  if (attr.irq)
    flags |= GPDMA_TCF;

  c->tr1 = flags;
}

#if 0
void stm32_gpdma_memcpy(void *addr, void *src, void *dst, unsigned int n)
{
  stm32_gpdma_trans(num, 0, src, dst, n,
                    GPDMA_TR1_DDW(0) | GPDMA_TR1_SDW(0) | \
                    GPDMA_TR1_SINC | GPDMA_TR1_DINC);
  stm32_gpdma_sw_trig(num, 0);
}

void stm32_gpdma_chan_dump(void *addr)
{
  stm32_gpdma_t *d = addr;
  unsigned int i;

  for (i = 0; i < DMA_CHANNELS; i++) {
    stm32_gpdma_chan_t *c = &d->chan[i];
    xprintf("%d ST: %08x C: %08x S: %08x D: %08x N: %d\n", i,
            c->sr, c->cr, c->sar, c->dar, c->tr1 & 0xffff);
  }
}

#define DMANUM 0

int cmd_gpdma(int argc, char *argv[])
{
  unsigned int chan, devid;
  unsigned int src, dst, n;

  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  case 'c':
    if (argc < 4)
      return -1;
    chan = atoi(argv[2]);
    devid = atoi(argv[3]);

    stm32_gpdma_set_chan(DMANUM, chan, devid);
    break;
  case 'm':
    if (argc < 5)
      return -1;
    src = strtoul(argv[2], 0, 16);
    dst = strtoul(argv[3], 0, 16);
    n = strtoul(argv[4], 0, 0);

    xprintf("copy src: %08x dst: %08x cnt: %d\n", src, dst, n);
    stm32_gpdma_memcpy(DMANUM, (void *)src, (void *)dst, n);
    break;
  case 'p':
    if (argc < 5)
      return -1;
    src = strtoul(argv[2], 0, 16);
    dst = strtoul(argv[3], 0, 16);
    n = strtoul(argv[4], 0, 0);
    stm32_gpdma_trans(DMANUM, 0, (void *)src, (void *)dst, n,
                      GPDMA_TR1_DDW(0) | GPDMA_TR1_SDW(0));
    break;
  case 't':
    if (argc < 3)
      return -1;
    stm32_gpdma_sw_trig(DMANUM, atoi(argv[2]));
    break;
  case 'e':
    if (argc < 3)
      return -1;
    stm32_gpdma_en(DMANUM, atoi(argv[2]), 1);
    break;
  case 's':
    if (argc < 3)
      return -1;
    stm32_gpdma_en(DMANUM, atoi(argv[2]), 0);
    break;
  case 'd':
    stm32_gpdma_chan_dump(DMANUM);
    break;
  }

  return 0;
}

SHELL_CMD(gpdma, cmd_gpdma);
#endif

dma_controller_t stm32_gpdma_controller = {
  stm32_gpdma_trans,
  stm32_gpdma_set_chan,
  stm32_gpdma_en,
  stm32_gpdma_start,
  stm32_gpdma_irq_ack
};
