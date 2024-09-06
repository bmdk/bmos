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

/* EXTI suport for C0, G0, H5, U5 */

#include "common.h"
#include "hal_common.h"
#include "stm32_exti.h"

#include "io.h"
#include "shell.h"

typedef struct {
  struct {
    reg32_t rtsr;
    reg32_t ftsr;
    reg32_t swier;
    reg32_t rpr;
    reg32_t fpr;
    reg32_t seccfgr;
    reg32_t privcfgr;
    reg32_t pad0;
  } r[2];
  unsigned int pad0[8];
  reg32_t cr[4];
  reg32_t lockr;
  unsigned int pad1[3];
  struct {
    reg32_t imr;
    reg32_t emr;
    unsigned int pad0[2];
  } cpu[2];
} stm32_exti_uxxx_t;

#if STM32_G0XX || STM32_C0XX || STM32_U0XX
#define EXTI_BASE 0x40021800
#elif STM32_H5XX
#define EXTI_BASE 0x44022000
#elif STM32_U5XX
#define EXTI_BASE 0x46022000
#else
#error No EXTI_BASE for this device.
#endif
#define EXTI ((volatile stm32_exti_uxxx_t*)EXTI_BASE)

void stm32_exti_irq_set_edge_rising(unsigned int n, int en)
{
  unsigned int reg = n / 32;

  n %= 32;

  if (reg > 1)
    return;

  bit_en(&EXTI->r[reg].rtsr, n, en);
}

void stm32_exti_irq_set_edge_falling(unsigned int n, int en)
{
  unsigned int reg = n / 32;

  n %= 32;

  if (reg > 1)
    return;

  bit_en(&EXTI->r[reg].ftsr, n, en);
}

void stm32_exti_irq_enable(unsigned int n, int en)
{
  unsigned int reg = n / 32;

  n %= 32;

  if (reg > 1)
    return;

  bit_en(&EXTI->cpu[reg].imr, n, en);
}

void stm32_exti_irq_ack(unsigned int n)
{
  unsigned int reg = n / 32;

  n %= 32;

  if (reg > 1)
    return;

  /* clear both falling and rising */
  EXTI->r[reg].fpr = BIT(n);
  EXTI->r[reg].rpr = BIT(n);
}

void stm32_exti_ev_enable(unsigned int n, int en)
{
  unsigned int reg = n / 32;

  n %= 32;

  if (reg > 1)
    return;

  bit_en(&EXTI->cpu[reg].emr, n, en);
}

void stm32_syscfg_set_exti(unsigned int v, unsigned int n)
{
  unsigned int reg, ofs;

  if (n > 15)
    return;

  reg = n / 4;
  ofs = n % 4;

  reg_set_field(&EXTI->cr[reg], 4, ofs << 3, v);
}

#if 0
static inline void _reg_set_clear(
  volatile unsigned int *reg, unsigned int clear, unsigned int set)
{
  unsigned int val;

  val = *reg;
  val &= ~clear;
  val |= set;
  xprintf("reg %p val: %08x\n", reg, val);
  *reg = val;
}

static inline void _reg_set_field(
  volatile unsigned int *reg, unsigned int width,
  unsigned int pos, unsigned int set)
{
  unsigned int mask = (BIT(width) - 1);

  _reg_set_clear(reg, mask << pos, (set & mask) << pos);
}

void _stm32_syscfg_set_exti(unsigned int v, unsigned int n)
{
  unsigned int reg, ofs;

  if (n > 15)
    return;

  reg = n / 4;
  ofs = n % 4;

  xprintf("reg %d ofs %d v: %d\n", reg, ofs, v);
  xprintf("reg %08x\n", &EXTI->exticr[reg]);

  _reg_set_field(&EXTI->exticr[reg], 4, ofs << 3, v);
}

int cmd_exti(int argc, char *argv[])
{
  stm32_syscfg_set_exti(2, 13);

  return 0;
}

SHELL_CMD(exti, cmd_exti);
#endif
