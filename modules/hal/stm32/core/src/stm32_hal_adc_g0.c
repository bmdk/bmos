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
#include "hal_common.h"
#include "hal_dma.h"
#include "hal_int.h"
#include "hal_time.h"
#include "io.h"
#include "shell.h"
#include "stm32_hal.h"
#include "stm32_hal_adc.h"
#include "xassert.h"

#define DMA_NUM 0
#define DMA_CHAN 2
#define DMA_DEVID 5

typedef struct {
  reg32_t isr;
  reg32_t ier;
  reg32_t cr;
  reg32_t cfgr1;
  reg32_t cfgr2;
  reg32_t smpr;
  reg32_t pad0[2];
  reg32_t tr[2];
  reg32_t chselr;
  reg32_t tr3;
  reg32_t pad1[4];
  reg32_t dr;
} stm32_adc_t;

typedef struct {
  reg32_t pad0[2];
  reg32_t ccr;
  reg32_t pad1;
} stm32_adc_com_t;

#define CFGR2_CKMODE_ADCCLK 0
#define CFGR2_CKMODE_PCLK_2 1
#define CFGR2_CKMODE_PCLK_4 2
#define CFGR2_CKMODE_PCLK 3

#define REG_ADC_CFGR1_DMAEN BIT(0)
#define REG_ADC_CFGR1_CHSELRMOD BIT(21)

#if STM32_G0XX || STM32_C0XX
#define ADC_BASE 0x40012400
#define ADC_COM_BASE (ADC_BASE + 0x300)
#else
#error Define ADC_BASE for this platform
#endif

#define ADC (stm32_adc_t *)ADC_BASE
#define ADC_COM (stm32_adc_com_t *)ADC_COM_BASE

#define CR_ADCAL BIT(31)
#define CR_ADVREGEN BIT(28)
#define CR_ADSTP BIT(4)
#define CR_ADSTART BIT(2)
#define CR_ADDIS BIT(1)
#define CR_ADEN BIT(0)

#define ISR_CCRDY BIT(13)
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

#define ADC_IRQ_EN_MSK (IER_OVRIE)

#define CCR_VBATEN BIT(24) /* Vbat */
#define CCR_TSEN BIT(23)   /* Temp */
#define CCR_VREFEN BIT(22) /* Vref */
#define CCR_PRESC(_m_) (((_m_) & 0xf) << 18)

#define MAX_SAMPLES 16

typedef struct {
  unsigned short res[MAX_SAMPLES];
  conv_done_f *conv_done;
  unsigned char count;
  unsigned char tcount;
  unsigned char flags;
} adc_data_t;

static adc_data_t adc_data;

static void adc_dma_irq(void *data)
{
  stm32_adc_t *a = (stm32_adc_t *)data;

  a->cfgr1 &= ~REG_ADC_CFGR1_DMAEN;

  dma_irq_ack(DMA_NUM, DMA_CHAN);

  if (adc_data.conv_done) {
    adc_data.conv_done(adc_data.res, adc_data.tcount, 0);
    adc_data.conv_done = 0;
  }
}

static void adc_irq(void *arg)
{
  stm32_adc_t *a = ADC;
  unsigned int isr;

  isr = a->isr;

  if (isr & ISR_OVR)
    debug_printf("adc overflow\n");

  a->isr = isr;
}

static void _stm32_adc_dma_init(stm32_adc_t *a)
{
  dma_attr_t attr;

  dma_en(DMA_NUM, DMA_CHAN, 0);

  dma_set_chan(DMA_NUM, DMA_CHAN, DMA_DEVID);

  attr.ssiz = DMA_SIZ_2;
  attr.dsiz = DMA_SIZ_2;
  attr.dir = DMA_DIR_FROM;
  attr.prio = 1;
  attr.sinc = 0;
  attr.dinc = 1;
  attr.irq = 1;

  dma_trans(DMA_NUM, DMA_CHAN, (void *)&a->dr, adc_data.res,
            adc_data.tcount, attr);
  dma_en(DMA_NUM, DMA_CHAN, 1);
}

#define CHSELR_ALT 1

#if CHSELR_ALT
static void adc_seq(stm32_adc_t *a, unsigned int num, unsigned int chan)
{
  reg_set_field(&a->chselr, 4, num * 4, chan);
}
#endif

static void _stm32_adc_init(void *base, unsigned char *reg_seq,
                            unsigned int rate,
                            unsigned int cnt, conv_done_f *_conv_done)
{
  stm32_adc_t *a = (stm32_adc_t *)base;
  stm32_adc_com_t *ac = ADC_COM;
  unsigned int i;

  reg_set_field(&a->cfgr2, 2, 30, CFGR2_CKMODE_PCLK_2);
  ac->ccr = CCR_VBATEN | CCR_TSEN | CCR_VREFEN | CCR_PRESC(1);

  /* start voltage regulator - 20us delay from datasheet for c031 */
  a->cr = CR_ADVREGEN;
  hal_delay_us(20);
  /* start calibration */
  a->cr |= CR_ADCAL;

  /* wait for calibration done */
  /* FIXME timeout? */
  while (a->cr & CR_ADCAL)
    ;

#if CHSELR_ALT
  a->cfgr1 |= REG_ADC_CFGR1_CHSELRMOD;
#else
  a->cfgr1 &= ~REG_ADC_CFGR1_CHSELRMOD;
#endif

  reg_set_field(&a->smpr, 3, 0, rate);

#if CHSELR_ALT
  if (cnt > 8)
    cnt = 8;
#endif
  adc_data.tcount = cnt;
  adc_data.conv_done = _conv_done;

  for (i = 0; i < cnt; i++) {
    unsigned int seq = reg_seq[i];
    if (seq >= 22)
      continue;
    a->smpr &= ~BIT(8 + seq);
#if CHSELR_ALT
    adc_seq(a, i, seq);
#else
    a->chselr |= BIT(seq);
#endif
  }
#if CHSELR_ALT
  if (i < 7)
    adc_seq(a, i, 0xf);
#endif

  while ((a->isr & ISR_CCRDY) == 0)
    ;

  a->isr |= ISR_ADRDY;

  a->cr |= CR_ADEN;
  a->cr &= ~CR_ADSTART;

  while ((a->isr & ISR_ADRDY) == 0)
    ;

  a->isr = 0xffffffff;

  irq_register("adc", adc_irq, 0, 12);
  irq_register("adc_dma", adc_dma_irq, 0, 10);

  a->ier = ADC_IRQ_EN_MSK;
}

void stm32_adc_init(unsigned int inst,
                    unsigned char *reg_seq, unsigned int cnt, int rate,
                    conv_done_f *_conv_done)
{
  XASSERT(inst == 0);
  _stm32_adc_init(ADC, reg_seq, cnt, rate, _conv_done);
}

static int _stm32_adc_conv(void *base)
{
  stm32_adc_t *a = (stm32_adc_t *)base;

  if ((a->cr & CR_ADSTART))
    return -1;

  a->cfgr1 &= ~REG_ADC_CFGR1_DMAEN;

  _stm32_adc_dma_init(a);

  a->cfgr1 |= REG_ADC_CFGR1_DMAEN;

  a->cr |= CR_ADSTART;

  return 0;
}

int stm32_adc_conv(unsigned int inst)
{
  XASSERT(inst == 0);
  return _stm32_adc_conv(ADC);
}
