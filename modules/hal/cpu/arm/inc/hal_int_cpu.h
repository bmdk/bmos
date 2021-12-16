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

#ifndef HAL_INT_CPU_H
#define HAL_INT_CPU_H

#define INTERRUPT_OFF() asm volatile ("cpsid i")
#define INTERRUPT_ON() asm volatile ("cpsie i")

#define __ISB() asm volatile ("isb")
#define __DSB() asm volatile ("dsb")

#undef __WFI
#define __WFI() asm volatile ("wfi")

static inline void set_psp(void *psp)
{
  asm volatile ("msr psp, %0\n" : : "r" (psp));
}

static inline void set_msp(void *msp)
{
  asm volatile ("msr msp, %0\n" : : "r" (msp));
}

static inline void set_basepri(unsigned int pri)
{
  asm volatile ("msr basepri, %0\n" : : "r" (pri));
}

static inline void set_faultmask(unsigned int msk)
{
  asm volatile ("msr faultmask, %0\n" : : "r" (msk));
}

static inline unsigned int interrupt_disable()
{
  unsigned int ret;

  asm volatile ("mrs %0, primask\n"
                "cpsid i\n"
                : "=r" (ret));
  return ret;
}

static inline void interrupt_enable(unsigned int saved)
{
  asm volatile ("msr primask, %0\n"
                : : "r" (saved));
}

#endif
