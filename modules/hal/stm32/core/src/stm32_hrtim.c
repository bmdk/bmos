#include <stdlib.h>

#include "io.h"
#include "shell.h"
#include "common.h"

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
  reg32_t set1r;
  reg32_t rst1r;
  reg32_t set2r;
  reg32_t rst2r;
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
  reg32_t adc1r;
  reg32_t adc2r;
  reg32_t adc3r;
  reg32_t adc4r;
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

#define HRTIM_MCR_CONT BIT(3) /* continuous */
#define HRTIM_MCR_RETRIG BIT(4)
#define HRTIM_MCR_MCEN BIT(16)
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

#define HRTIM_TIMX_CR_CONT BIT(3)

static stm32_hrtim_t *hrtim = (stm32_hrtim_t *)HRTIM_BASE;

void hrtim_set_compare_pct(unsigned int timer,
                           unsigned int compare, unsigned int pct)
{
  unsigned int cval;
  reg32_t *cmpx;

  if (pct > 100)
    pct = 100;

  cval = (0xffffU * pct + 50) / 100;

  switch (compare) {
  default:
  case 0:
    cmpx = &hrtim->tim[timer].cmp1r;
    break;
  case 1:
    cmpx = &hrtim->tim[timer].cmp2r;
    break;
  case 3:
    cmpx = &hrtim->tim[timer].cmp3r;
    break;
  case 4:
    cmpx = &hrtim->tim[timer].cmp4r;
    break;
  }

  *cmpx = cval & 0xffffU;
}

int hrtim_init()
{
  hrtim->mcr &= ~(HRTIM_MCR_MCEN | HRTIM_MCR_TACEN | HRTIM_MCR_TBCEN);

  hrtim->oenr = (HRTIM_OENR_TA1OEN | HRTIM_OENR_TA2OEN)
  hrtim->mcntr = 0;

  hrtim->tim[0].cntr = 0;

  hrtim->tim[0].cr |= HRTIM_TIMX_CR_CONT;

  hrtim_set_compare_pct(0, 0, 0);

  hrtim->tim[0].set1r = 0;
  hrtim->tim[0].rst1r = 0;

  hrtim->tim[0].set2r = 0;
  hrtim->tim[0].rst2r = 0;

#if 0
  hrtim->tim[1].cntr = 0x8000;
  hrtim->tim[1].cr |= HRTIM_TIMX_CR_CONT;

  hrtim_set_compare_pct(1, 0, 50);

  hrtim->tim[1].set1r = HRTIM_TIMX_SET_PER;
  hrtim->tim[1].rst1r = HRTIM_TIMX_SET_CMP1;

  hrtim->tim[1].set2r = HRTIM_TIMX_SET_CMP1;
  hrtim->tim[1].rst2r = HRTIM_TIMX_SET_PER;
#endif

  hrtim->mcr |= HRTIM_MCR_TACEN;
  return 0;
}

int cmd_hrtim(int argc, char *argv[])
{
  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  default:
  case 'g':
    xprintf("mcr: %x\n", hrtim->mcr);
    xprintf("mcnt: %x\n", hrtim->mcntr);
    xprintf("mper: %x\n", hrtim->mper);
    xprintf("isr0: %08x\n", hrtim->tim[0].isr);
    xprintf("cnt0: %04x\n", hrtim->tim[0].cntr);
    xprintf("per0: %04x\n", hrtim->tim[0].perr);
  case 'p':
    {
      unsigned int pct, compare;

      if (argc < 3)
        return -1;

      pct = atoi(argv[2]);
      if (pct > 100)
        pct = 100;

      compare = (0xffffU * pct + 50) / 100;

      hrtim->tim[0].cmp1r = compare & 0xffffU;
    }
    break;
  case 's':
    if (argc < 3)
      return -1;

    speed = atoi(argv[2]);
    if (speed == 0) {
      hrtim->tim[0].set1r = 0;
      hrtim->tim[0].rst1r = 0;
      hrtim->tim[0].set2r = 0;
      hrtim->tim[0].rst2r = 0;
    } else if (speed > 0) {
      hrtim->tim[0].set1r = HRTIM_TIMX_SET_PER;
      hrtim->tim[0].rst1r = HRTIM_TIMX_SET_CMP1;
      hrtim->tim[0].set2r = 0;
      hrtim->tim[0].rst2r = 0;
      hrtim_set_compare_pct(0, 0, speed);
    } else {
      hrtim->tim[0].set1r = 0;
      hrtim->tim[0].rst1r = 0;
      hrtim->tim[0].set2r = HRTIM_TIMX_SET_PER;
      hrtim->tim[0].rst2r = HRTIM_TIMX_SET_CMP1;
      hrtim_set_compare_pct(0, 0, -speed);
    }
  break;
  }

  return 0;
}

SHELL_CMD(hrtim, cmd_hrtim);
