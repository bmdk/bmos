#include "common.h"
#include "stm32_wwdg.h"

typedef struct {
  reg32_t cr;
  reg32_t cfr;
  reg32_t sr;
} stm32_wwdg_t;

#if STM32_H7XX
#define WWDG1_BASE 0x50003000
#define WWDG2_BASE 0x40002C00

#define WWDG2 ((stm32_wwdg_t *)WWDG2_BASE)

#define WWDG_CR_WDGA BIT(7)
void wwdg_reset_cpu2()
{
  WWDG2->cfr = 0;
  WWDG2->cr = WWDG_CR_WDGA;
}
#endif
