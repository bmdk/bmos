/* Copyright (c) 2019 Brian Thomas Murphy
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

#include <stdlib.h>

#include "hal_common.h"
#include "shell.h"
#include "io.h"

typedef struct {
  unsigned int ccr;
  unsigned int cndtr;
  unsigned int cpar;
  unsigned int cmar[2];
} stm32_bdma_chan_t;

typedef struct {
  unsigned int isr;
  unsigned int ifcr;
  stm32_bdma_chan_t chan[7];
  unsigned int pad0[5];
  unsigned int cselr;
} stm32_bdma_t;

#ifdef STM32_H7XX
#define BDMA1_BASE (void *)0x58025400
#else
#define BDMA1_BASE (void *)0x40020000
#define BDMA2_BASE (void *)0x40020400
#endif

#define CCR_CT BIT(16)
#define CCR_DBM BIT(15)
#define CCR_MEM2MEM BIT(14)
#define CCR_PL(_l_) (((_l_) & 0x3) << 12) /* prio */
#define CCR_MSIZ(_s_) (((_s_) & 0x3) << 10)
#define CCR_PSIZ(_s_) (((_s_) & 0x3) << 8)
#define CCR_MINC BIT(7)
#define CCR_PINC BIT(6)
#define CCR_CIRC BIT(5)
#define CCR_DIR BIT(4) /* 0 - from peripheral */
#define CCR_TEIE BIT(3)
#define CCR_HEIE BIT(2)
#define CCR_TCIE BIT(1)
#define CCR_EN BIT(0)

void stm32_bdma_set_chan(void *base, unsigned int chan, unsigned int devid)
{
  volatile stm32_bdma_t *d = base;

  reg_set_field(&d->cselr, 4, chan << 2, devid);
}

void stm32_bdma_memcpy(void *base, void *dst, void *src, unsigned int n)
{
  volatile stm32_bdma_t *d = base;
  volatile stm32_bdma_chan_t *c = &d->chan[0];

  c->cndtr = n;
  c->cpar = (unsigned int)src;
  c->cmar[0] = (unsigned int)dst;

  c->ccr = CCR_MEM2MEM | CCR_PL(0) | CCR_MSIZ(0) | CCR_PSIZ(0) | \
           CCR_MINC | CCR_PINC | CCR_EN;
}

int cmd_bdma(int argc, char *argv[])
{
  unsigned int chan, devid;
  unsigned int src, dst, n;

  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  case 'c':
    if (argc < 4)
      return -1;
    chan = atoi(argv[2]);
    devid = atoi(argv[3]);

    stm32_bdma_set_chan(BDMA1_BASE, chan, devid);
    break;
  case 'm':
    if (argc < 5)
      return -1;
    dst = strtoul(argv[2], 0, 16);
    src = strtoul(argv[3], 0, 16);
    n = strtoul(argv[4], 0, 0);

    xprintf("copy dst: %08x src: %08x cnt: %d\n", dst, src, n);
    stm32_bdma_memcpy(BDMA1_BASE, (void *)dst, (void *)src, n);
    break;
  }

  return 0;
}

SHELL_CMD(bdma, cmd_bdma);
