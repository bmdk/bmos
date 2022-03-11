#include "hal_common.h"
#include "rp2040_hal_resets.h"

#define RESETS_BASE 0x4000c000

typedef struct {
  reg32_t reset;
  reg32_t wdsel;
  reg32_t reset_done;
} rp2040_resets_t;

#define RESETS ((rp2040_resets_t*)RESETS_BASE)

void rp2040_reset_clr(unsigned int n)
{
  unsigned int mask = BIT(n);

  RESETS->reset &= ~mask;

  while (!(RESETS->reset_done & mask))
    ;
}

