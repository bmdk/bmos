/* Copyright (c) 2019-2022 Brian Thomas Murphy
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

#include "common.h"
#include "fast_log.h"
#include "hal_common.h"
#include "hal_dma.h"
#include "hal_int.h"
#include "hal_time.h"
#include "io.h"
#include "shell.h"
#include "stm32_hal.h"
#include "stm32_hal_adc.h"

#if STM32_G4XX
#define ADC_USE_DMA 1
/* since we have a DMAMUX the dma number and channel are arbitrary */
#define DMA_NUM 0
#define DMA_CHAN 1
#define DMA_DEVID 5 /* ADC1 */
#define DMA_IRQ 12  /* DMA1 CH1 */
#endif

typedef struct {
  reg32_t isr;
  reg32_t ier;
  reg32_t cr;
  reg32_t cfgr;
  reg32_t cfgr2;
  reg32_t smpr[2];
  reg32_t pcsel;
  reg32_t tr[3];
  reg32_t pad1;
  reg32_t sqr[4];
  reg32_t dr;
  reg32_t pad2[2];
  reg32_t jsqr;
  reg32_t pad3[4];
  reg32_t ofr[4];
  reg32_t pad4[4];
  reg32_t jdr[4];
  reg32_t pad5[4];
  reg32_t awd2cr;
  reg32_t awd3cr;
  reg32_t pad6[2];
  reg32_t difsel;
  reg32_t calfact;
  reg32_t calfact2;
  reg32_t pad7[3];
  reg32_t or;
} stm32_adc_t;

typedef struct {
  reg32_t csr;
  reg32_t pad0;
  reg32_t ccr;
  reg32_t cdr;
} stm32_adc_com_t;

#if STM32_L4XX
#define ADC_BASE 0x50040000
#define ADC_COM_BASE (ADC_BASE + 0x300)
#define ADC_CKMODE 1
#define ADC1_IRQ 18
#elif STM32_G4XX
#define ADC_BASE 0x50000000
#define ADC_COM_BASE (ADC_BASE + 0x300)
#define ADC_CKMODE 3
#define ADC1_IRQ 18
#elif STM32_H5XX
#include "stm32_h5xx.h"
#define ADC_COM_BASE (ADC_BASE + 0x300)
/* clock range 1.5 - 75MHz
   250MHz / 4 = 62.5 MHz */
#define ADC_CKMODE 3
#define ADC1_IRQ 37
#elif STM32_U5XX
#define ADC_BASE 0x42028000
#define ADC_COM_BASE (ADC_BASE + 0x300)
#define ADC_CKMODE 0
#define ADC1_IRQ 37
#else
#error Define ADC_BASE for this platform
#endif

#define ADC (stm32_adc_t *)ADC_BASE
#define ADC_COM (stm32_adc_com_t *)ADC_COM_BASE

#define SAM_RATE_2_5 0
#define SAM_RATE_6_5 1
#define SAM_RATE_12_5 2
#define SAM_RATE_24_5 3
#define SAM_RATE_47_5 4
#define SAM_RATE_92_5 5
#define SAM_RATE_247_5 6
#define SAM_RATE_640_5 7

#define CR_ADCAL BIT(31)
#define CR_ADCALDIF BIT(30)
#define CR_DEEPPWD BIT(29)
#define CR_ADVREGEN BIT(28)
#define CR_JADSTP BIT(5)
#define CR_ADSTP BIT(4)
#define CR_JADSTART BIT(3)
#define CR_ADSTART BIT(2)
#define CR_ADDIS BIT(1)
#define CR_ADEN BIT(0)

#define CFGR_DMAEN BIT(0)
#define CFGR_DMACFG BIT(1)
#define CFGR_CONT BIT(13)
#define CFGR_DISCEN BIT(16)

#define ISR_LDORDY BIT(12)
#define ISR_OVR BIT(4)
#define ISR_EOS BIT(3)
#define ISR_EOC BIT(2)
#define ISR_EOSMP BIT(1)
#define ISR_ADRDY BIT(0)

#define IER_OVRIE BIT(4)
#define IER_EOSIE BIT(3)
#define IER_EOCIE BIT(2)
#define IER_EOSMPIE BIT(1)
#define IER_ADRDYIE BIT(0)

#define CCR_CH18SEL BIT(24) /* Vbat */
#define CCR_CH17SEL BIT(23) /* Temp */
#define CCR_VREFEN BIT(22)  /* Temp */
#define CCR_PRESC(_m_) (((_m_) & 0xf) << 18)
#define CCR_PRESC_1 0
#define CCR_PRESC_2 1
#define CCR_PRESC_4 2
#define CCR_PRESC_6 3
#define CCR_PRESC_8 4
#define CCR_PRESC_10 5
#define CCR_PRESC_12 6
#define CCR_PRESC_16 7
#define CCR_PRESC_32 8
#define CCR_PRESC_64 9
#define CCR_PRESC_128 10
#define CCR_PRESC_256 11
#define CCR_CKMODE(_m_) (((_m_) & 0x3) << 16)

#if STM32_U5XX
#define ADC_PRESC CCR_PRESC_4
#else
#define ADC_PRESC CCR_PRESC_1
#endif

typedef struct {
  unsigned short res[16];
  conv_done_f *conv_done;
  unsigned char tcount; /* sample count in each conversion */
  unsigned char count;  /* samples received so far */
  unsigned char flags;
  void *dma_buf;
  void *dma_bufh;
  unsigned int dma_buflen;
} adc_data_t;

#define ADC_DATA_FLAGS_CONV_ACTIVE BIT(0)

static adc_data_t adc_data;

static void adc_irq(void *arg)
{
  stm32_adc_t *a = ADC;
  unsigned int dr, isr;

  isr = a->isr;

  if (isr & ISR_OVR) {
    a->isr |= ISR_OVR;
    FAST_LOG('A', "adc overflow\n", 0, 0);
  }

  if (isr & ISR_EOC) {
    dr = a->dr;
    adc_data.res[adc_data.count++] = dr;
  }

  if (isr & ISR_EOS) {
    a->isr |= ISR_EOS;
    if (adc_data.conv_done)
      adc_data.conv_done(adc_data.res, adc_data.count, 0);
    adc_data.count = 0;

    adc_data.flags &= ~ADC_DATA_FLAGS_CONV_ACTIVE;
  }
}

#if ADC_USE_DMA
static void adc_dma_irq(void *data)
{
  unsigned int status;

  status = dma_irq_ack(DMA_NUM, DMA_CHAN);

  if (adc_data.flags & ADC_DATA_FLAGS_CONV_ACTIVE) {
    adc_data.conv_done(adc_data.res, adc_data.tcount, 0);
    adc_data.flags &= ~ADC_DATA_FLAGS_CONV_ACTIVE;
  } else {
    if (status & DMA_IRQ_STATUS_FULL) {
      adc_data.conv_done(adc_data.dma_bufh, adc_data.dma_buflen,
                         ADC_CONV_DONE_TYPE_FULL);
      FAST_LOG('A', "adc dma full\n", 0, 0);
    } else if (status & DMA_IRQ_STATUS_HALF) {
      adc_data.conv_done(adc_data.dma_buf, adc_data.dma_buflen,
                         ADC_CONV_DONE_TYPE_HALF);
      FAST_LOG('A', "adc dma half\n", 0, 0);
    }
  }
}
#endif

void stm32_adc_vbat(int en)
{
  stm32_adc_com_t *ac = ADC_COM;

  if (en)
    ac->ccr |= CCR_CH18SEL;
  else
    ac->ccr &= ~CCR_CH18SEL;
}

#if ADC_USE_DMA
static void _stm32_adc_dma_init(stm32_adc_t *a, int circ,
                                void *buf, unsigned int cnt)
{
  dma_attr_t attr;

  dma_en(DMA_NUM, DMA_CHAN, 0);

  dma_set_chan(DMA_NUM, DMA_CHAN, DMA_DEVID);

  attr.ssiz = DMA_SIZ_4;
  attr.dsiz = DMA_SIZ_2;
  attr.dir = DMA_DIR_FROM;
  attr.prio = 1;
  attr.sinc = 0;
  attr.dinc = 1;
  attr.irq = 1;

  if (circ) {
    attr.circ = 1;
    attr.irq_half = 1;
  }

  dma_trans(DMA_NUM, DMA_CHAN, (void *)&a->dr, buf, cnt, attr);
  dma_en(DMA_NUM, DMA_CHAN, 1);
}
#endif

static void _stm32_adc_trig_ev(void *base, int event)
{
  stm32_adc_t *a = (stm32_adc_t *)base;

  a->cfgr &= ~(CFGR_DISCEN | CFGR_CONT);

  reg_set_field(&a->cfgr, 5, 5, event); /* EXTSEL */
  reg_set_field(&a->cfgr, 2, 10, 1);    /* EXTEN positive edges */
}

void stm32_adc_trig_ev(int event)
{
  _stm32_adc_trig_ev(ADC, event);
}

#if STM32_U5XX
#define ADC_RES_BIT14 0
#define ADC_RES_BIT12 1
#define ADC_RES_BIT10 2
#define ADC_RES_BIT8 3
#else
#define ADC_RES_BIT12 0
#define ADC_RES_BIT10 1
#define ADC_RES_BIT8 2
#define ADC_RES_BIT6 3
#endif

#define ADC_RES_DEFAULT ADC_RES_BIT12

static void _stm32_adc_init(void *base, unsigned char *reg_seq,
                            unsigned int cnt, conv_done_f *conv_done)
{
  stm32_adc_t *a = (stm32_adc_t *)base;
  stm32_adc_com_t *ac = ADC_COM;
  unsigned int i;

  adc_data.conv_done = conv_done;

  cnt &= 0xf;

  /* CKMODE HCLK */
  ac->ccr = CCR_CH17SEL | CCR_VREFEN | \
            CCR_PRESC(ADC_PRESC) | CCR_CKMODE(ADC_CKMODE);

  a->cr &= ~CR_DEEPPWD;
  a->cr |= CR_ADVREGEN;
  /* 20us on L496 */
  hal_delay_us(20);
  a->cr &= ~CR_ADCALDIF;
  a->cr |= CR_ADCAL;

  reg_set_field(&a->cfgr, 2, 2, ADC_RES_DEFAULT);

  while (a->cr & CR_ADCAL)
    ;

  adc_data.tcount = cnt;

  for (i = 0; i < cnt; i++) {
    unsigned int n = (i + 1) % 5;
    unsigned int reg = (i + 1) / 5;

#if STM32_H5XX
    if (reg_seq[i] == 0) {
      a->or |= BIT(0); /* enable IN0 (via OP0) */
    }
#endif

#if STM32_U5XX
    a->pcsel |= BIT(reg_seq[i]);
#endif

    reg_set_field(&a->sqr[reg], 5, n * 6, reg_seq[i]);

    n = (reg_seq[i]) % 10;
    reg = (reg_seq[i]) / 10;

    reg_set_field(&a->smpr[reg], 3, n * 3, SAM_RATE_640_5);
  }

  reg_set_field(&a->sqr[0], 4, 0, cnt - 1);

  a->isr |= ISR_ADRDY;

  a->cr |= CR_ADEN;
  a->cr &= ~CR_ADSTART;

  while ((a->isr & ISR_ADRDY) == 0)
    ;

  a->isr = 0xffffffff;

  irq_register("adc1", adc_irq, 0, ADC1_IRQ);
#if ADC_USE_DMA
  irq_register("adc_dma", adc_dma_irq, a, DMA_IRQ);
  a->ier |= IER_OVRIE;
#else
  a->ier |= IER_OVRIE | IER_EOCIE | IER_EOSIE;
#endif
}

void stm32_adc_init(unsigned char *reg_seq, unsigned int cnt,
                    conv_done_f *conv_done)
{
  _stm32_adc_init(ADC, reg_seq, cnt, conv_done);
}

#if ADC_USE_DMA
static int _stm32_adc_conv(void *base)
{
  stm32_adc_t *a = (stm32_adc_t *)base;

  if (adc_data.flags & ADC_DATA_FLAGS_CONV_ACTIVE)
    return -1;

  if (a->cr & CR_ADSTART)
    return -1;

  _stm32_adc_dma_init(a, 0, &adc_data.res, adc_data.tcount);
  adc_data.flags |= ADC_DATA_FLAGS_CONV_ACTIVE;

  a->cfgr |= CFGR_DMAEN;
  a->cr |= CR_ADSTART;

  return 0;
}

void stm32_adc_init_dma(unsigned char *reg_seq, unsigned int cnt,
                        void *buf, unsigned int buflen, conv_done_f *conv_done)
{
  _stm32_adc_init(ADC, reg_seq, cnt, conv_done);

  adc_data.tcount = 0;
  adc_data.dma_buf = buf;
  /* no of samples in a half dma buffer */
  adc_data.dma_buflen = buflen / 4;
  adc_data.dma_bufh = (void *)((unsigned char *)buf + 2 * adc_data.dma_buflen);

  /* translate buffer length to number of samples */
  _stm32_adc_dma_init(ADC, 1, buf, 2 * adc_data.dma_buflen);

  stm32_adc_t *a = ADC;

  a->cfgr |= CFGR_DMAEN | CFGR_DMACFG;
}
#else
static int _stm32_adc_conv(void *base)
{
  stm32_adc_t *a = (stm32_adc_t *)base;

  if ((a->cr & CR_ADSTART))
    return -1;

  adc_data.count = 0;

  a->cr |= CR_ADSTART;

  return 0;
}
#endif

int stm32_adc_conv()
{
  return _stm32_adc_conv(ADC);
}

static int _stm32_adc_start(void *base)
{
  stm32_adc_t *a = (stm32_adc_t *)base;

  if (a->cr & CR_ADSTART)
    return -1;

  a->cr |= CR_ADSTART;

  return 0;
}

int stm32_adc_start()
{
  return _stm32_adc_start(ADC);
}
