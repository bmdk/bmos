#include <stdlib.h>

#include "io.h"
#include "shell.h"
#include "common.h"
#include "hal_common.h"
#include "stm32_hrtim.h"

#define N_HRTIM_TIM 6

typedef struct {
  reg32_t set;
  reg32_t rst;
} stm32_hrtim_tim_output_t;

typedef struct {
  reg32_t cr;
  reg32_t isr;
  reg32_t icr;
  reg32_t dier;
  reg32_t cntr;
  reg32_t perr;
  reg32_t repr;
  reg32_t cmp1r;
  reg32_t cmp1cr;
  reg32_t cmp2r;
  reg32_t cmp3r;
  reg32_t cmp4r;
  reg32_t cpt1r;
  reg32_t cpt2r;
  reg32_t dtr;
  stm32_hrtim_tim_output_t out[2];
  reg32_t eef1r;
  reg32_t eef2r;
  reg32_t rstr;
  reg32_t chpr;
  reg32_t cpt1cr;
  reg32_t cpt2cr;
  reg32_t outr;
  reg32_t fltr;
  reg32_t timcr2;
  reg32_t eefr3;
  reg32_t pad0[3];
} stm32_hrtim_tim_t;

typedef struct {
  reg32_t mcr;
  reg32_t misr;
  reg32_t micr;
  reg32_t mdier;
  reg32_t mcntr;
  reg32_t mper;
  reg32_t mrep;
  reg32_t mcmp1r;
  reg32_t pad0;
  reg32_t mcmp2r;
  reg32_t mcmp3r;
  reg32_t mcmp4r;
  reg32_t pad1[20];
  stm32_hrtim_tim_t tim[6];
  reg32_t cr1;
  reg32_t cr2;
  reg32_t isr;
  reg32_t icr;
  reg32_t ier;
  reg32_t oenr;
  reg32_t odisr;
  reg32_t odsr;
  reg32_t bmcr;
  reg32_t bmtrg;
  reg32_t bmcmpr;
  reg32_t bmper;
  reg32_t eecr1;
  reg32_t eecr2;
  reg32_t eecr3;
  reg32_t adc[4];
  reg32_t dllcr;
  reg32_t fltinr1;
  reg32_t fltinr2;
  reg32_t bdmupdr;
  reg32_t bdtaupr;
  reg32_t bdtbupr;
  reg32_t bdtcupr;
  reg32_t bdtdupr;
  reg32_t bdteupr;
  reg32_t bdmadr;
  reg32_t bdtfupr;
  reg32_t adcer;
  reg32_t adcur;
  reg32_t adcps1;
  reg32_t adcps2;
  reg32_t flintr3;
  reg32_t flintr4;
} stm32_hrtim_t;

#if STM32_G4XX
#define HRTIM_BASE 0x40016800
#endif

#define HRTIM_MCR_CONT_NUM 3
#define HRTIM_MCR_CONT BIT(HRTIM_MCR_CONT_NUM) /* continuous */
#define HRTIM_MCR_RETRIG BIT(4)
#define HRTIM_MCR_MCEN BIT(16)
#define HRTIM_MCR_TxCEN(n) BIT(17 + (n))
#define HRTIM_MCR_TACEN BIT(17)
#define HRTIM_MCR_TBCEN BIT(18)
#define HRTIM_MCR_TCCEN BIT(19)
#define HRTIM_MCR_TDCEN BIT(20)
#define HRTIM_MCR_TECEN BIT(21)
#define HRTIM_MCR_TFCEN BIT(22)
#define HRTIM_MCR_PREEN BIT(27)

/* events for output set/reset registers */
#define HRTIM_TIMX_SET_SST BIT(0)
#define HRTIM_TIMX_SET_RESYNC BIT(1)
#define HRTIM_TIMX_SET_PER BIT(2)
#define HRTIM_TIMX_SET_CMPx(n) BIT(3 + (n))
#define HRTIM_TIMX_SET_CMP1 BIT(3)
#define HRTIM_TIMX_SET_CMP2 BIT(4)
#define HRTIM_TIMX_SET_CMP3 BIT(5)
#define HRTIM_TIMX_SET_CMP4 BIT(6)
#define HRTIM_TIMX_SET_MST_PER BIT(7)
#define HRTIM_TIMX_SET_MST_CMP1 BIT(8)
#define HRTIM_TIMX_SET_MST_CMP2 BIT(9)
#define HRTIM_TIMX_SET_MST_CMP3 BIT(10)
#define HRTIM_TIMX_SET_MST_CMP4 BIT(11)
#define HRTIM_TIMX_SET_UPDATE BIT(31)

/* output enable */
#define HRTIM_OENR_TA1OEN BIT(0)
#define HRTIM_OENR_TA2OEN BIT(1)
#define HRTIM_OENR_TB1OEN BIT(2)
#define HRTIM_OENR_TB2OEN BIT(3)
#define HRTIM_OENR_TC1OEN BIT(4)
#define HRTIM_OENR_TC2OEN BIT(5)
#define HRTIM_OENR_TD1OEN BIT(6)
#define HRTIM_OENR_TD2OEN BIT(7)
#define HRTIM_OENR_TE1OEN BIT(8)
#define HRTIM_OENR_TE2OEN BIT(9)
#define HRTIM_OENR_TF1OEN BIT(10)
#define HRTIM_OENR_TF2OEN BIT(11)

#define HRTIM_TIMX_CR_CONT_NUM 3
#define HRTIM_TIMX_CR_CONT BIT(HRTIM_TIMX_CR_CONT_NUM)

static stm32_hrtim_t *hrtim = (stm32_hrtim_t *)HRTIM_BASE;

void hrtim_adc_trig_en(unsigned int adc_trig, unsigned int bit, int en)
{
  reg_set_field(&hrtim->adc[adc_trig], 1, bit, en);
}

void hrtim_set_compare(unsigned int timer,
                       unsigned int compare, unsigned int val)
{
  reg32_t *cmpx;

  switch (compare) {
  default:
  case 0:
    cmpx = &hrtim->tim[timer].cmp1r;
    break;
  case 1:
    cmpx = &hrtim->tim[timer].cmp2r;
    break;
  case 2:
    cmpx = &hrtim->tim[timer].cmp3r;
    break;
  case 3:
    cmpx = &hrtim->tim[timer].cmp4r;
    break;
  }

  *cmpx = val & 0xffffU;
}

void hrtim_mst_set_compare(unsigned int compare, unsigned int val)
{
  reg32_t *cmpx;

  switch (compare) {
  default:
  case 0:
    cmpx = &hrtim->mcmp1r;
    break;
  case 1:
    cmpx = &hrtim->mcmp2r;
    break;
  case 2:
    cmpx = &hrtim->mcmp3r;
    break;
  case 3:
    cmpx = &hrtim->mcmp4r;
    break;
  }

  *cmpx = val & 0xffffU;
}

void hrtim_set_output(unsigned int timer, unsigned int o,
                      unsigned int set, unsigned int rst)
{
  stm32_hrtim_tim_output_t *out;

  if (timer >= N_HRTIM_TIM || o >= 2)
    return;

  out = &hrtim->tim[timer].out[o];

  out->set = set;
  out->rst = rst;
}

void hrtim_set_output_on(unsigned int timer, unsigned int o, int on)
{
  if (on)
    hrtim_set_output(timer, o, HRTIM_TIMX_SET_PER, 0);
  else
    hrtim_set_output(timer, o, 0, HRTIM_TIMX_SET_PER);
}

void hrtim_set_output_compare(unsigned int timer, unsigned int o,
                              unsigned int compare, int on)
{
  unsigned int on_flags, off_flags;

  if (compare >= 4)
    return;

  if (on) {
    on_flags = HRTIM_TIMX_SET_CMPx(compare);
    off_flags = HRTIM_TIMX_SET_PER;
  } else {
    on_flags = HRTIM_TIMX_SET_PER;
    off_flags = HRTIM_TIMX_SET_CMPx(compare);
  }

  hrtim_set_output(timer, o, on_flags, off_flags);
}

void hrtim_tim_init(unsigned int tim, unsigned int cntr,
                    unsigned int div_pow_2, unsigned int flags)
{
  stm32_hrtim_tim_t *timn = &hrtim->tim[tim];

  if (tim >= N_HRTIM_TIM)
    return;

  hrtim->mcr &= ~HRTIM_MCR_TxCEN(tim);

  reg_set_field(&hrtim->oenr, 1, 2 * tim, flags >> HRTIM_TIM_FLG_O1EN_NUM);
  reg_set_field(&hrtim->oenr, 1, 2 * tim + 1, flags >> HRTIM_TIM_FLG_O2EN_NUM);

  reg_set_field(&timn->cr, 1, HRTIM_TIMX_CR_CONT_NUM,
                flags >> HRTIM_TIM_FLG_CONT_NUM);

  /* divider to 2^n */
  reg_set_field(&timn->cr, 3, 0, div_pow_2);

  timn->cntr = cntr;
  timn->perr = HRTIM_TIMX_PERR_MAX;

  hrtim_set_output(tim, 0, 0, 0);
  hrtim_set_output(tim, 1, 0, 0);
  hrtim_set_compare(0, 0, 0);

  hrtim->mcr |= HRTIM_MCR_TxCEN(tim);
}

void hrtim_mst_init(unsigned int cntr,
                    unsigned int div_pow_2, unsigned int flags)
{
  hrtim->mcr &= ~HRTIM_MCR_MCEN;

  reg_set_field(&hrtim->mcr, 1, HRTIM_MCR_CONT_NUM,
                flags >> HRTIM_TIM_FLG_CONT_NUM);

  hrtim->mcntr = cntr;

  /* divider to 2^n */
  reg_set_field(&hrtim->mcr, 3, 0, div_pow_2);

  hrtim->mper = HRTIM_TIMX_PERR_MAX;

  hrtim_mst_set_compare(0, 0);

  hrtim->mcr |= HRTIM_MCR_MCEN;
}
