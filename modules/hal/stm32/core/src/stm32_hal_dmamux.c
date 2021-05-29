/* Copyright (c) 2019-2021 Brian Thomas Murphy
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
#include "stm32_hal_dmamux.h"

typedef struct {
  unsigned int cr[16];
  unsigned int pad1[16];
  unsigned int sr;
  unsigned int fr;
  unsigned int pad2[30];
  unsigned int gcr[4];
  unsigned int gsr;
  unsigned int gfr;
} stm32_dmamux_t;

#ifdef STM32_H7XX
#define DMAMUX1_BASE 0x40020800
#define DMAMUX2_BASE 0x58025800
static volatile stm32_dmamux_t *dmamux[] = { (void *)DMAMUX1_BASE,
                                             (void *)DMAMUX2_BASE };
#else
#error Define dmamux for this platform
#endif

#define DMAMUX_CR_ID(_v_) ((_v_) & 0x7f)
#define DMAMUX_CR_EGE BIT(9)

void stm32_dmamux_req(unsigned int dmamux_num,
                      unsigned int chan, unsigned int req)
{
  volatile stm32_dmamux_t *d;

  if (dmamux_num >= ARRSIZ(dmamux))
    return;

  d = dmamux[dmamux_num];
  chan &= 0xf;

  d->cr[chan] &= ~DMAMUX_CR_EGE;
  d->cr[chan] = DMAMUX_CR_EGE | DMAMUX_CR_ID(req);
}
