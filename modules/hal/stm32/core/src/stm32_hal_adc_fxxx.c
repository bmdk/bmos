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
#include "stm32_timer.h"
#include "xassert.h"

#if STM32_F1XX || AT32_F4XX
#define DMA_NUM 0
#define DMA_CHAN 0
#define DMA_DEVID 0
#define DMA_IRQ 11
#else
#define DMA_NUM 1
#define DMA_CHAN 0
#define DMA_DEVID 0 /* 0 or 4 is ADC1 on F411 */
#define DMA_IRQ 56
#endif

typedef struct {
  reg32_t sr;
  reg32_t cr1;
  reg32_t cr2;
  reg32_t smpr[2];
  reg32_t jofr[4];
  reg32_t htr;
  reg32_t ltr;
  reg32_t sqr[3];
  reg32_t jsqr;
  reg32_t jdr[4];
  reg32_t dr;
} stm32_adc_t;

typedef struct {
  reg32_t pad0;
  reg32_t ccr;
} stm32_adc_com_t;

#if STM32_F1XX || AT32_F4XX
#define ADC_BASE (void *)0x40012400
#else
#define ADC_BASE (void *)0x40012000
#endif
#define ADC_COM_BASE (void *)(ADC_BASE + 0x300)

#define ADC_DMA_LEN 16

#define ADC_DATA_FLAGS_CONV_ACTIVE BIT(0)

typedef struct {
  unsigned short res[ADC_DMA_LEN];
  conv_done_f *conv_done;
  unsigned char tcount;
  unsigned char flags;
} adc_data_t;

static adc_data_t adc_data;

#define SR_OVR BIT(5)
#define SR_STRT BIT(4)
#define SR_JSTRT BIT(3)
#define SR_JEOC BIT(2)
#define SR_EOC BIT(1)
#define SR_AWD BIT(0)

#define CCR_TSVREFE BIT(23)
#define CCR_VBATE BIT(22)
#define CCR_ADCPRE(_div_) (((_div_) & 0x3) << 16)
#define CCR_ADCPRE_DIV2 CCR_ADCPRE(0)
#define CCR_ADCPRE_DIV4 CCR_ADCPRE(1)
#define CCR_ADCPRE_DIV6 CCR_ADCPRE(2)
#define CCR_ADCPRE_DIV8 CCR_ADCPRE(3)

#define CR1_OVRIE BIT(26)
#define CR1_RES(_v_) (((_v_) & 3) << 24)
#define CR1_RES_12 CR1_RES(0)
#define CR1_RES_10 CR1_RES(1)
#define CR1_RES_8 CR1_RES(2)
#define CR1_RES_6 CR1_RES(3)
#define CR1_DISCEN BIT(11)
#define CR1_SCAN BIT(8)
#define CR1_JEOCIE BIT(7)
#define CR1_AWDIE BIT(6)
#define CR1_EOCIE BIT(5)

#if STM32_F1XX || AT32_F4XX
#define CR2_TSVREFE BIT(23)
#define CR2_SWSTART BIT(22)
#define CR2_EXTSEL_SWSTART 7
#else
#define CR2_SWSTART BIT(30)
#endif
#define CR2_ALIGN BIT(11)
#define CR2_EOCS BIT(10)
#define CR2_DDS BIT(9)
#define CR2_DMA BIT(8)
#define CR2_CAL BIT(2)
#define CR2_CONT BIT(1)
#define CR2_ADON BIT(0)

static void adc_irq(void *arg)
{
  stm32_adc_t *a = (stm32_adc_t *)ADC_BASE;
  unsigned int sr;

  sr = a->sr;

  if (sr & SR_OVR) {
    FAST_LOG('A', "adc overflow\n", 0, 0);
    a->sr |= SR_OVR;
  }

  if (sr & SR_EOC)
    FAST_LOG('A', "adc EOC\n", 0, 0);
}

static void adc_dma_irq(void *data)
{
  unsigned int status;

  status = dma_irq_ack(DMA_NUM, DMA_CHAN);
  if (status & DMA_IRQ_STATUS_FULL)
    FAST_LOG('A', "adc dma full\n", 0, 0);
  if (status & DMA_IRQ_STATUS_HALF)
    FAST_LOG('A', "adc dma half\n", 0, 0);

  if (adc_data.flags & ADC_DATA_FLAGS_CONV_ACTIVE) {
    stm32_adc_t *a = (stm32_adc_t *)data;

    a->cr2 &= ~CR2_DMA;

    adc_data.conv_done(adc_data.res, adc_data.tcount, 0);
    adc_data.flags &= ~ADC_DATA_FLAGS_CONV_ACTIVE;
  }
}

static void adc_seq(stm32_adc_t *a, unsigned int num, unsigned int chan)
{
  unsigned int n = num % 6;
  unsigned int reg = 2 - (num / 6);

  reg_set_field(&a->sqr[reg], 5, n * 5, chan);
}

static void adc_srate(stm32_adc_t *a, unsigned int chan, unsigned int rate)
{
  unsigned int n, reg;

  n = chan % 10;
  reg = 1 - chan / 10;

  reg_set_field(&a->smpr[reg], 3, n * 3, rate);
}

/* external trigger selection */
#define ADC_EXT_TYPE_DISABLED 0
#define ADC_EXT_TYPE_RISING 1
#define ADC_EXT_TYPE_FALLING 2
#define ADC_EXT_TYPE_BOTH 3
static void adc_ext_type(stm32_adc_t *a, unsigned int type)
{
  reg_set_field(&a->cr2, 2, 28, type);
}

/* select external trigger source */
static void adc_ext_sel(stm32_adc_t *a, unsigned int sel)
{
  reg_set_field(&a->cr2, 4, 24, sel);
}

static void _stm32_adc_init(void *base, unsigned char *reg_seq,
                            unsigned int cnt)
{
  stm32_adc_t *a = (stm32_adc_t *)base;

#if !STM32_F1XX && !AT32_F4XX
  stm32_adc_com_t *ac = (stm32_adc_com_t *)ADC_COM_BASE;
#endif
  unsigned int i;

  cnt &= 0xf;

  a->cr2 = CR2_ADON;
  hal_delay_us(2);

#if STM32_F1XX || AT32_F4XX
  a->cr1 = CR1_SCAN;

  reg_set_field(&a->cr2, 3, 17, CR2_EXTSEL_SWSTART);

  a->cr2 |= CR2_TSVREFE | CR2_CAL;

  while (a->cr2 & CR2_CAL)
    ;
#else
  a->cr1 = CR1_OVRIE | CR1_RES_12 | CR1_SCAN;
  ac->ccr = CCR_TSVREFE | CCR_ADCPRE_DIV8;

  adc_ext_type(a, ADC_EXT_TYPE_DISABLED);
  adc_ext_sel(a, 0);
#endif

  for (i = 0; i < cnt; i++) {
    adc_seq(a, i, reg_seq[i]);
    /* 3 + 12 clocks */
    adc_srate(a, reg_seq[i], 0);
  }

  reg_set_field(&a->sqr[0], 4, 20, cnt - 1);

  irq_register("adc", adc_irq, 0, 18);
  irq_register("adc_dma", adc_dma_irq, a, DMA_IRQ);
}

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

void stm32_adc_init(unsigned char *reg_seq, unsigned int cnt, conv_done_f *conv_done)
{
  XASSERT(cnt <= ADC_DMA_LEN);

  _stm32_adc_init(ADC_BASE, reg_seq, cnt);
  adc_data.tcount = cnt;
  adc_data.flags &= ~ADC_DATA_FLAGS_CONV_ACTIVE;
}

void stm32_adc_init_dma(unsigned char *reg_seq, unsigned int cnt,
                        void *buf, unsigned int buflen, conv_done_f *conv_done)
{
  unsigned int compare[] = { 10 };

  _stm32_adc_init(ADC_BASE, reg_seq, cnt);

  adc_data.tcount = 0;

  stm32_adc_t *a = (stm32_adc_t *)ADC_BASE;

  adc_ext_type(a, ADC_EXT_TYPE_RISING);
  adc_ext_sel(a, 0); /* timer 1 cc1 */

  _stm32_adc_dma_init(a, 1, buf, buflen / 2);

  a->cr2 |= CR2_DDS | CR2_DMA;

  timer_init_pwm(TIM1_BASE, 96, 19, compare, 1);
}

static int _stm32_adc_conv(void *base)
{
  stm32_adc_t *a = (stm32_adc_t *)base;

  if (adc_data.flags & ADC_DATA_FLAGS_CONV_ACTIVE)
    return -1;

  a->cr2 &= ~(CR2_DMA | CR2_SWSTART);

  _stm32_adc_dma_init(a, 0, &adc_data.res, adc_data.tcount);
  adc_data.flags |= ADC_DATA_FLAGS_CONV_ACTIVE;

  a->cr2 |= (CR2_DMA | CR2_SWSTART);

  return 0;
}

int stm32_adc_conv()
{
  return _stm32_adc_conv(ADC_BASE);
}
