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

#include "stm32_wdog.h"
#include "shell.h"
#include "io.h"

typedef struct {
  unsigned int kr;
  unsigned int pr;
  unsigned int rlr;
  unsigned int sr;
  unsigned int winr;
} stm32_iwdg_t;

#define KR_KICK 0xaaaa
#define KR_UNLOCK 0x5555
#define KR_START 0xcccc

static void _stm32_iwdg_init(void *base, unsigned int presc, unsigned int count)
{
  volatile stm32_iwdg_t *w = (volatile stm32_iwdg_t *)base;

  w->kr = KR_UNLOCK;

  w->pr = presc & 0x7;
  w->rlr = count & 0xfff;

  w->kr = KR_START;
}

static void _stm32_iwdg_kick(void *base)
{
  volatile stm32_iwdg_t *w = (volatile stm32_iwdg_t *)base;

  w->kr = KR_KICK;
}

void stm32_wdog_init(unsigned int presc, unsigned int count)
{
  _stm32_iwdg_init(IWDG_BASE, presc, count);
}

void stm32_wdog_kick(void)
{
  _stm32_iwdg_kick(IWDG_BASE);
}
