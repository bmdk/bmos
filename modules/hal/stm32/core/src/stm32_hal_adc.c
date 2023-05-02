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
#include "hal_int.h"
#include "hal_time.h"
#include "io.h"
#include "shell.h"
#include "stm32_hal.h"
#include "stm32_hal_adc.h"

typedef struct {
  reg32_t isr;
  reg32_t ier;
  reg32_t cr;
  reg32_t cfgr;
  reg32_t cfgr2;
  reg32_t smpr[2];
  reg32_t pad0;
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
#elif STM32_G4XX
#define ADC_BASE 0x50000000
#define ADC_COM_BASE (ADC_BASE + 0x300)
#define ADC_CKMODE 3
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
#define CCR_CKMODE(_m_) (((_m_) & 0x3) << 16)

typedef struct {
  unsigned short res[16];
  conv_done_f *conv_done;
  unsigned char count;
  unsigned char flags;
} adc_data_t;

static adc_data_t adc_data;

static void adc_irq(void *arg)
{
  stm32_adc_t *a = ADC;
  unsigned int dr, isr;

  isr = a->isr;

  if (isr & ISR_OVR)
    debug_printf("adc overflow");

  if (isr & ISR_EOC) {
    dr = a->dr;
    adc_data.res[adc_data.count++] = dr;
  }

  if (isr & ISR_EOS) {
    a->isr |= ISR_EOS;
    if (adc_data.conv_done)
      adc_data.conv_done(adc_data.res, adc_data.count, 0);
    adc_data.conv_done = 0;
    adc_data.count = 0;
  }
}

static void _stm32_adc_init(void *base, unsigned char *reg_seq,
                            unsigned int cnt, conv_done_f *conv_done)
{
  stm32_adc_t *a = (stm32_adc_t *)base;
  stm32_adc_com_t *ac = ADC_COM;
  unsigned int i;

  adc_data.conv_done = conv_done;

  cnt &= 0xf;

  /* CKMODE HCLK */
  ac->ccr = CCR_CH18SEL | CCR_CH17SEL | CCR_VREFEN | CCR_CKMODE(ADC_CKMODE);

  a->cr &= ~CR_DEEPPWD;
  a->cr |= CR_ADVREGEN;
  /* 20us on L496 */
  hal_delay_us(20);
  a->cr &= ~CR_ADCALDIF;
  a->cr |= CR_ADCAL;

  while (a->cr & CR_ADCAL)
    ;

  for (i = 0; i < cnt; i++) {
    unsigned int n = (i + 1) % 5;
    unsigned int reg = (i + 1) / 5;

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

  irq_register("adc", adc_irq, 0, 18);

  a->ier |= IER_OVRIE | IER_EOCIE | IER_EOSIE;
}

void stm32_adc_init(unsigned char *reg_seq, unsigned int cnt, conv_done_f *conv_done)
{
  _stm32_adc_init(ADC, reg_seq, cnt, conv_done);
}

static int _stm32_adc_conv(void *base)
{
  stm32_adc_t *a = (stm32_adc_t *)base;

  if ((a->cr & CR_ADSTART))
    return -1;

  a->cr |= CR_ADSTART;

  return 0;
}

int stm32_adc_conv()
{
  return _stm32_adc_conv(ADC);
}
