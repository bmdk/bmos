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

#ifndef CORTEXM_H
#define CORTEXM_H

#include "common.h"

#define DWT_BASE 0xE0001000UL
#define SCS_BASE 0xE000E000UL
#define SYSTICK_BASE (SCS_BASE + 0x10UL)
#define NVIC_BASE (SCS_BASE + 0x100UL)
#define SCB_BASE (SCS_BASE + 0xD00UL)
#define CP_BASE (SCS_BASE + 0xD88UL)
#define FPU_BASE (SCS_BASE + 0xF34UL)

typedef struct {
  unsigned int r0;
  unsigned int r1;
  unsigned int r2;
  unsigned int r3;
  unsigned int r12;
  unsigned int lr;
  unsigned int pc;
  unsigned int xpsr;
} stack_frame_t;

typedef struct {
  unsigned int r4;
  unsigned int r5;
  unsigned int r6;
  unsigned int r7;
  unsigned int r8;
  unsigned int r9;
  unsigned int r10;
  unsigned int r11;
} sw_stack_frame_t;

typedef struct {
  reg32_t ctrl;
  reg32_t load;
  reg32_t val;
  reg32_t calib;
} systick_t;

typedef struct {
  reg32_t cpuid;
  reg32_t icsr;
  reg32_t vtor;
  reg32_t aircr;
  reg32_t scr;
  reg32_t ccr;
  reg32_t shpr[3];
  reg32_t shcsr;
  reg32_t cfsr;
  reg32_t hfsr;
  reg32_t pad[1];
  reg32_t mmfar;
  reg32_t bfar;
  reg32_t afsr;
} scb_t;

typedef struct {
  reg32_t fpccr;
  reg32_t fpcar;
  reg32_t fpdscr;
} fpu_t;

#define FPCCR_LSPEN BIT(30)
#define FPCCR_ASPEN BIT(31)

typedef struct {
  reg32_t iser[8];
  reg32_t pad0[24];
  reg32_t icer[8];
  reg32_t pad1[24];
  reg32_t ispr[8];
  reg32_t pad2[24];
  reg32_t icpr[8];
  reg32_t pad3[24];
  reg32_t iabr[8];
  reg32_t pad4[56];
  reg32_t ip[240];
  reg32_t pad5[644];
  reg32_t stir;
} nvic_t;

typedef struct {
  reg32_t ctrl;
  reg32_t cyccnt;
  reg32_t cpicnt;
  reg32_t exccnt;
  reg32_t sleepcnt;
  reg32_t lsucnt;
  reg32_t foldcnt;
  reg32_t pcsr;
  struct {
    reg32_t comp;
    reg32_t mask;
    reg32_t function;
    reg32_t pad;
  } event[4];
} dwt_t;

#define DWT ((dwt_t *)DWT_BASE)
#define SCB ((scb_t *)SCB_BASE)
#define FPC ((fpu_t *)FPU_BASE)

void set_low_power(int en);

void systick_init();

static inline void trigger_pendsv()
{
  SCB->icsr = BIT(28);
}

static inline void clear_pendsv()
{
  SCB->icsr = BIT(27);
}

static inline unsigned int cycle_cnt()
{
  return DWT->cyccnt;
}

void dwt_init(void);

#endif
