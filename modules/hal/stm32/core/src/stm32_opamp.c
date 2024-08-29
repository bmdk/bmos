#include <stdlib.h>

#include "common.h"
#include "hal_common.h"

typedef struct {
  reg32_t csr[6];
  reg32_t tmcr[6];
} stm32_opamp_t;

#if STM32_G4XX
#define OPAMP_BASE 0x40010300
#else
#error define OPAMP_BASE for this target
#endif

static stm32_opamp_t *opamp = (stm32_opamp_t *)OPAMP_BASE;

#define OPAMP_CSR_PGA_GAIN(g) (((g) & 0x1f) << 14)
#define OPAMP_CSR_VM_SEL(v) (((v) & 0x3) << 5)
#define OPAMP_CSR_VP_SEL(v) (((v) & 0x3) << 2)
#define OPAMP_CSR_OPAINTOEN BIT(8)
#define OPAMP_CSR_FORCE_VP BIT(1)
#define OPAMP_CSR_OPAEN BIT(0)

void opamp_set_gain(unsigned int n, unsigned pga_gain)
{
  reg32_t *csr = &opamp->csr[n];

  if (n > 6)
    return;

  reg_set_field(csr, 5, 14, pga_gain);
}

void opamp_init(unsigned int n, unsigned int vm, unsigned int vp, int internal)
{
  reg32_t *csr = &opamp->csr[n];

  if (n > 6)
    return;

  *csr &= ~OPAMP_CSR_OPAEN;

  reg_set_field(csr, 2, 5, vm);
  reg_set_field(csr, 2, 2, vp);
  reg_set_field(csr, 5, 14, 0);

  if (internal)
    *csr |= OPAMP_CSR_OPAINTOEN;
  else
    *csr &= ~OPAMP_CSR_OPAINTOEN;

  *csr |= OPAMP_CSR_OPAEN;
}

void opamp_init_all()
{
#if 0
  /* follower mode DAC4 ch 1 - PB12 */
  opamp_init(3, 3, 3, 0);
  /* follower mode DAC4 ch 2 output to PA8 */
  opamp_init(4, 3, 3, 0);

  /* pins
     opamp5 out pa8
     opamp4 out pb12 */
#endif
}
