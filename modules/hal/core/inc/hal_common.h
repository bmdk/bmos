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

#ifndef HAL_COMMON_H
#define HAL_COMMON_H

#include "common.h"

static inline void reg_set_clear(
  volatile unsigned int *reg, unsigned int clear, unsigned int set)
{
  unsigned int val;

  val = *reg;
  val &= ~clear;
  val |= set;
  *reg = val;
}

static inline void reg_set_clear_shift(
  volatile unsigned int *reg, unsigned int clear,
  unsigned int set, unsigned int shift)
{
  reg_set_clear(reg, clear << shift, set << shift);
}

static inline void reg_set_field(
  volatile unsigned int *reg, unsigned int width,
  unsigned int pos, unsigned int set)
{
  unsigned int mask = (BIT(width) - 1);

  reg_set_clear(reg, mask << pos, (set & mask) << pos);
}

static inline unsigned int reg_get_field(
  volatile unsigned int *reg, unsigned int width, unsigned int pos)
{
  unsigned int mask = (BIT(width) - 1);

  return ((*reg) >> pos) & mask;
}

static inline void bit_en(volatile unsigned int *reg, unsigned int n, int en)
{
  if (n >= 32)
    return;

  if (en)
    *reg |= BIT(n);
  else
    *reg &= ~BIT(n);
}

int bl_enter(void);

void hal_init();

unsigned int hal_flash_size(void);

#endif
