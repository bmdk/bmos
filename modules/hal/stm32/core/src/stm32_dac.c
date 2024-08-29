#include <stdlib.h>

#include "common.h"
#include "hal_common.h"
#include "io.h"
#include "shell.h"

typedef struct {
  reg32_t cr;
  reg32_t swtrgr;
  reg32_t dhr12r1;
  reg32_t dhr12l1;
  reg32_t dhr8r1;
  reg32_t dhr12r2;
  reg32_t dhr12l2;
  reg32_t dhr8r2;
  reg32_t dhr12rd;
  reg32_t dhr12ld;
  reg32_t dhr8rd;
  reg32_t dor1;
  reg32_t dor2;
  reg32_t sr;
  reg32_t ccr;
  reg32_t mcr;
  reg32_t shsr1;
  reg32_t shsr2;
  reg32_t shhr;
  reg32_t shrr;
  reg32_t str1;
  reg32_t str2;
  reg32_t stmodr;
} stm32_dac_t;

#if STM32_G4XX
#define N_DACS 4
#define DAC1_BASE 0x50000800
#define DAC2_BASE 0x50000C00
#define DAC3_BASE 0x50001000
#define DAC4_BASE 0x50001400

static stm32_dac_t *dac[N_DACS] = {
  (stm32_dac_t *)DAC1_BASE,
  (stm32_dac_t *)DAC2_BASE,
  (stm32_dac_t *)DAC3_BASE,
  (stm32_dac_t *)DAC4_BASE
};
#else
#error missing DAC config for this target
#endif

#define STM32_DAC_CR_EN(n) BIT(16 * (n))
#define STM32_DAC_CR_EN2 STM32_DAC_CR_EN(1)
#define STM32_DAC_CR_EN1 STM32_DAC_CR_EN(0)

#define STM32_DAC_SR_DACRDY(n) BIT(11 + (n) * 16)

#define STM32_DAC_SR_BWST2 BIT(31)
#define STM32_DAC_SR_DAC2RDY STM32_DAC_SR_DACRDY(1)

#define STM32_DAC_SR_BWST1 BIT(15)
#define STM32_DAC_SR_DAC1RDY STM32_DAC_SR_DACRDY(0)

#define DAC_COUNT 4

void dac_init(unsigned int dac_no, unsigned int channel, unsigned int cfg)
{
  stm32_dac_t *d;

  if (dac_no >= DAC_COUNT || channel >= 2)
    return;

  d = dac[dac_no];

  /* high speed */
  reg_set_field(&d->mcr, 2, 14, 2);

  reg_set_field(&d->mcr, 3, channel * 16, cfg);

  d->cr |= STM32_DAC_CR_EN(channel);
}

void dac_set_val(unsigned int dac_no, unsigned int channel, unsigned int val)
{
  stm32_dac_t *d;
  reg32_t *r;

  if (dac_no >= DAC_COUNT || channel >= 2)
    return;

  d = dac[dac_no];

  while ((d->sr & STM32_DAC_SR_DACRDY(channel)) == 0)
    ;

  if (channel == 0)
    r = &d->dhr12r1;
  else
    r = &d->dhr12r2;

  *r = val & 0xfff;
}

int dacs_init()
{
  dac_init(0, 0, 0);
  //dac_init(0, 1, 0);

  dac_set_val(0, 0, 0xfff * 250 / 330);

  return 0;
}

int cmd_dac(int argc, char *argv[])
{
  unsigned int val;

  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  default:
  case 'v':
    if (argc < 3)
      return -1;
    val = strtoul(argv[2], NULL, 0);
    dac_set_val(0, 0, val);
    break;
  case 'g':
    xprintf("dac1: do1: %x do2: %x\n",  dac[0]->dor1 & 0xfff,
            dac[0]->dor2 & 0xfff);
    xprintf("dac3: do1: %x do2: %x\n",  dac[2]->dor1 & 0xfff,
            dac[2]->dor2 & 0xfff);
    xprintf("dac4: do1: %x do2: %x\n",  dac[3]->dor1 & 0xfff,
            dac[3]->dor2 & 0xfff);
    break;
  }

  return 0;
}

SHELL_CMD(dac, cmd_dac);
