#include "common.h"
#include "hal_common.h"

#include "stm32_pwr.h"
#include "stm32_pwr_f0xx.h"

typedef struct {
  reg32_t cfgr1;
  reg32_t rsvd0;
  reg32_t exticr[4];
  reg32_t cfgr2;
} stm32_syscfg_t;

typedef struct {
  reg32_t cr;
  reg32_t csr;
} stm32_pwr_t;

#define SYSCFG ((stm32_syscfg_t *)(0x40010000))
#define PWR ((stm32_pwr_t *)(0x40007000))

void stm32_memmap(unsigned int val)
{
  reg_set_field(&SYSCFG->cfgr1, 2, 0, val);
}

#define PWR_CR_DBP BIT(8)

void backup_domain_protect(int on)
{
  if (on)
    PWR->cr &= ~PWR_CR_DBP;
  else
    PWR->cr |= PWR_CR_DBP;
}
