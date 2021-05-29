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

#include <stdlib.h>
#include <string.h>

#include "hal_int.h"
#include "io.h"
#include "shell.h"
#include "xassert.h"

#if 0
int cmd_irqe(int argc, char *argv[])
{
  unsigned int n = 0;

  if (argc > 1)
    n = atoi(argv[1]);

  irq_enable(n, 1);

  return 0;
}

SHELL_CMD(irqe, cmd_irqe);

int cmd_irqp(int argc, char *argv[])
{
  unsigned int n = 0;

  if (argc > 1)
    n = atoi(argv[1]);

  irq_set_pending(n);

  return 0;
}

SHELL_CMD(irqp, cmd_irqp);
#endif

typedef struct irq_handler_data {
  irq_handler_t *handler;
  void *data;
  const char *name;
  unsigned int count;
} irq_handler_data_t;

#define N_INTS 128
static irq_handler_data_t interrupts[N_INTS];

void irq_stats_reset(unsigned int num)
{
  irq_handler_data_t *c;

  XASSERT(num < N_INTS);

  c = &interrupts[num];
  c->count = 0;
}

void irq_register(const char *name, irq_handler_t *handler, void *data,
                  unsigned int num)
{
  irq_handler_data_t *c;

  XASSERT(num < N_INTS);

  c = &interrupts[num];
  c->handler = handler;
  c->data = data;
  c->name = name;
  irq_stats_reset(num);

  irq_enable(num, 1);
}

void irq_call(unsigned int num)
{
  irq_handler_data_t *c;

  XASSERT(num < N_INTS);

  c = &interrupts[num];

  if (c->handler == 0)
    xpanic("unhandled interrupt %d\n", num);

  c->handler(c->data);
  c->count++;
}

int cmd_irq(int argc, char *argv[])
{
  irq_handler_data_t *c;
  unsigned int i;

  for (i = 0; i < N_INTS; i++) {
    c = &interrupts[i];

    if (c->handler)
      xprintf("%3d: %6d %s\n", i, c->count, c->name);
  }

  return 0;
}

SHELL_CMD(irq, cmd_irq);
