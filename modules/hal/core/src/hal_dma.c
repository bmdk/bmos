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

#include <stdlib.h>

#include "io.h"
#include "shell.h"

#include "hal_dma.h"
#include "hal_dma_if.h"
#include "xassert.h"

#define GET_CONT_DATA \
  dma_cont_data_t *cont_data; \
  dma_controller_t *cont; \
  XASSERT(num < dma_cont_data_len); \
  cont_data = &dma_cont_data[num]; \
  cont = cont_data->cont; \
  XASSERT(cont)


void dma_trans(unsigned int num, unsigned int chan,
               void *src, void *dst, unsigned int n, dma_attr_t flags)
{
  GET_CONT_DATA;
  cont->trans(cont_data->data, chan, src, dst, n, flags);
}

void dma_set_chan(unsigned int num, unsigned int chan, unsigned int devid)
{
  GET_CONT_DATA;
  cont->set_chan(cont_data->data, chan, devid);
}

void dma_en(unsigned int num, unsigned int chan, int en)
{
  GET_CONT_DATA;
  cont->en(cont_data->data, chan, en);
}

void dma_start(unsigned int num, unsigned int chan)
{
  GET_CONT_DATA;
  cont->start(cont_data->data, chan);
}

void dma_irq_ack(unsigned int num, unsigned int chan)
{
  GET_CONT_DATA;
  cont->irq_ack(cont_data->data, chan);
}

#if 0
int dma_controller_reg(unsigned int num, dma_controller_t *cont, void *data)
{
  XASSERT(num < dma_cont_data_len);

  dma_cont_data[num].cont = cont;
  dma_cont_data[num].data = data;

  return 0;
}
#endif

#ifndef CONFIG_HAL_DMA_COMMANDS
#define CONFIG_HAL_DMA_COMMANDS 1
#endif

#if CONFIG_HAL_DMA_COMMANDS
static void dma_memcpy(unsigned int num, void *src, void *dst, unsigned int n)
{
  dma_attr_t attr;

  attr.ssiz = DMA_SIZ_1;
  attr.dsiz = DMA_SIZ_1;
  attr.dir = DMA_DIR_TO;
  attr.prio = 0;
  attr.sinc = 1;
  attr.dinc = 1;
  attr.irq = 0;

  dma_trans(num, 0, src, dst, n, attr);

  dma_start(num, 0);
}

#define DMANUM 0

int cmd_dma(int argc, char *argv[])
{
  unsigned int chan, devid;
  unsigned int src, dst, n;
  dma_attr_t attr;

  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  case 'c':
    if (argc < 4)
      return -1;
    chan = atoi(argv[2]);
    devid = atoi(argv[3]);

    dma_set_chan(DMANUM, chan, devid);
    break;
  case 'm':
    if (argc < 5)
      return -1;
    src = strtoul(argv[2], 0, 16);
    dst = strtoul(argv[3], 0, 16);
    n = strtoul(argv[4], 0, 0);

    xprintf("copy src: %08x dst: %08x cnt: %d\n", src, dst, n);
    dma_memcpy(DMANUM, (void *)src, (void *)dst, n);
    break;
  case 'p':
    if (argc < 5)
      return -1;
    src = strtoul(argv[2], 0, 16);
    dst = strtoul(argv[3], 0, 16);
    n = strtoul(argv[4], 0, 0);

    attr.ssiz = DMA_SIZ_1;
    attr.dsiz = DMA_SIZ_1;
    attr.dir = DMA_DIR_TO;
    attr.prio = 0;

    dma_trans(DMANUM, 0, (void *)src, (void *)dst, n, attr);
    break;
  case 't':
    if (argc < 3)
      return -1;
    dma_start(DMANUM, atoi(argv[2]));
    break;
  case 'e':
    if (argc < 3)
      return -1;
    dma_en(DMANUM, atoi(argv[2]), 1);
    break;
  case 's':
    if (argc < 3)
      return -1;
    dma_en(DMANUM, atoi(argv[2]), 0);
    break;
#if 0
  case 'd':
    dma_chan_dump(DMANUM);
    break;
#endif
  }

  return 0;
}

SHELL_CMD(dma, cmd_dma);
#endif
