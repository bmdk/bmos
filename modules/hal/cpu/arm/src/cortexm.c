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

#include "common.h"
#include "cortexm.h"
#include "debug_ser.h"
#include "hal_cpu.h"
#include "hal_int.h"
#include "hal_board.h"
#include "io.h"
#include "shell.h"
#include "fast_log.h"

#define HSFR_FORCED BIT(30)
#define HSFR_VECTTBL BIT(1)

#define CFSR_ZERODIV BIT(9 + 16)
#define CFSR_UNALIGNED BIT(8 + 16)
#define CFSR_NOCP BIT(3 + 16)
#define CFSR_INVPC BIT(2 + 16)
#define CFSR_INVSTATE BIT(1 + 16)
#define CFSR_UNDEFINSTR BIT(0 + 16)

#define CFSR_BFAR_VALID BIT(7 + 8)
#define CFSR_BFAR_IMPRECISERR BIT(2 + 8)
#define CFSR_BFAR_PRECISERR BIT(1 + 8)
#define CFSR_BFAR_IBUSERR BIT(0 + 8)

#define CFSR_MMAR_VALID BIT(7)
#define CFSR_MLSPERR BIT(5)
#define CFSR_MSTKERR BIT(4)
#define CFSR_MUNSTKERR BIT(3)
#define CFSR_DACCVIOL BIT(1)
#define CFSR_IACCVIOL BIT(0)

#define SYSTICK ((systick_t *)SYSTICK_BASE)
#define NVIC ((nvic_t *)NVIC_BASE)

#if ARCH_PICO
#define VTOR 0x10000100
#elif RAM_APPL
#define VTOR 0x20000000
#elif APPL
#define VTOR APPL_FLASH_BASE
#else
#define VTOR 0x8000000
#endif

unsigned int orig_vtor;

#if FLOATING_POINT
static void fp_enable()
{
  reg32_t *cpacr = (reg32_t *)(CP_BASE);

  *cpacr |= (0xf << 20); /* enable CP10 + 11 */

  /* disable automatic floating point context saving
     - only one thread has access to floating point */
  FPC->fpccr &= ~(FPCCR_ASPEN | FPCCR_LSPEN);
}
#endif

void hal_cpu_init()
{
  unsigned int i;

  orig_vtor = SCB->vtor;

  /* set the correct exception vector */
  SCB->vtor = VTOR;

  /* enable 8 byte stack alignment
     unaligned access and divide by zero exception */
  SCB->ccr |= BIT(9) | BIT(4) | BIT(3);

#if FLOATING_POINT
  fp_enable();
#endif

#if 1
  /* set exception priorities */
  SCB->shpr[0] = 0x00000000;
  SCB->shpr[1] = 0x00000000;
  /* systick has highest priority, pendsv has lowest priority */
  SCB->shpr[2] = 0x00ff0000;
#endif

  for (i = 0; i < 8; i++)
    NVIC->iser[i] = 0;

/* initialize interupt priorities to the lowest possible by default */
  for (i = 0; i < 32; i++)
    NVIC->ip[i] = 0xffffffff;
}

void hal_cpu_fini()
{
  SCB->vtor = 0;
  SYSTICK->ctrl = 0;
}

#define SYSTICK_CTRL_ENABLE BIT(0)
#define SYSTICK_CTRL_TICKINT BIT(1)
/* Clock source 0 - AHB / 8, 1 - AHB */
#define SYSTICK_CTRL_CLKSOURCE BIT(2)

#ifndef HAL_CPU_CLOCK
#define HAL_CPU_CLOCK hal_cpu_clock
#endif

void systick_init()
{
  SYSTICK->ctrl = 0;
  SYSTICK->load = HAL_CPU_CLOCK / 1000 - 1;
  SYSTICK->val = 0;
  SYSTICK->ctrl = SYSTICK_CTRL_CLKSOURCE | SYSTICK_CTRL_TICKINT |
                  SYSTICK_CTRL_ENABLE;
}

void exception_handler(void) __attribute__((naked));

#ifndef CONFIG_SIMPLE_EXCEPTION_HANDLER
#define CONFIG_SIMPLE_EXCEPTION_HANDLER 0
#endif

#if !CONFIG_SIMPLE_EXCEPTION_HANDLER
static void dump_stack_frame(stack_frame_t *sf)
{
  debug_printf("  r0 : %08x r1 : %08x r2 : %08x r3 : %08x\n", sf->r0, sf->r1,
               sf->r2, sf->r3);
  debug_printf("  r12: %08x\n", sf->r12);
  debug_printf("  pc : %08x lr : %08x sr : %08x\n", sf->pc, sf->lr, sf->xpsr);
}

static void _exception_handler(sw_stack_frame_t *xsf, unsigned int exc_return)
{
  unsigned int e = SCB->icsr & 0xff;
  stack_frame_t *psf, *sf;
  const char *name = "unknown";

  sf = (stack_frame_t *)(xsf + 1);

  if (e == 3) {
    unsigned int hfsr = SCB->hfsr;

    name = "hard fault";
    debug_printf("Exception(%d) - %s\n", e, name);

    if (hfsr & HSFR_FORCED) {
      unsigned int cfsr = SCB->cfsr;
      if (cfsr & CFSR_UNDEFINSTR)
        debug_puts(" Undefined instruction.\n");
      if (cfsr & CFSR_UNALIGNED)
        debug_printf(" Unaligned access.\n");
      if (cfsr & CFSR_MMAR_VALID)
        debug_printf(" fault address(mm): %08x\n", SCB->mmar);
      if (cfsr & CFSR_BFAR_PRECISERR)
        debug_printf(" Bus error\n");
      if (cfsr & CFSR_BFAR_VALID)
        debug_printf(" fault address(bus): %08x\n", SCB->bfar);
      debug_printf(" cfsr: %08x\n", cfsr);
    } else if (hfsr & HSFR_VECTTBL)
      debug_puts(" Vector table error\n");
  } else
    debug_printf("Exception(%d) - %s\n", e, "unknown");

  /* bit 2 of exc_return(lr) indicates where the exception frame
     was saved - on process stack or main stack. */
  if ((exc_return & 0x4) == 0) {
    debug_printf("\nmain stack: %08x\n", sf);

    dump_stack_frame(sf);
  } else {
    /* get the process stack frame */
    asm volatile ("MRS %0, psp\n"  : "=r" (psf) );

    if (psf) {
      debug_printf("\nprocess stack: %08x\n", psf);
      dump_stack_frame(psf);
    }
  }

  debug_printf("\n");
  debug_printf("  r4 : %08x r5 : %08x r6 : %08x r7 : %08x\n", xsf->r4, xsf->r5,
               xsf->r6, xsf->r7);
  debug_printf("  r8 : %08x r9 : %08x r10: %08x r11: %08x\n", xsf->r8, xsf->r9,
               xsf->r10, xsf->r11);

  debug_printf("\nHALT\n");

  fast_log_dump(128, 1);

  hal_cpu_reset();
  for (;;)
    ;
}

void exception_handler()
{
  register sw_stack_frame_t *xsf;
  unsigned int exc_return;

/* *INDENT-OFF* */
  asm volatile (
#if __ARM_ARCH_6M__
                "push {r4-r7};\n"
                "mov r4, r8;\n"
                "mov r5, r9;\n"
                "mov r6, r10;\n"
                "mov r7, r11;\n"
                "push {r4-r7};\n"
#else
                "push {r4-r11};\n"
#endif
                "mov %0, sp;\n"
                "mov %1, r14;\n"
                : "=r" (xsf), "=r" (exc_return));
/* *INDENT-ON* */

#if 0
  debug_printf("exc_return: %08x\n", exc_return);
#endif

  FAST_LOG('I', "exception\n", 0, 0);

  _exception_handler(xsf, exc_return);
}
#else
void exception_handler()
{
  hal_cpu_reset();

  for (;;)
    ;
}
#endif

void interrupt_handler()
{
  unsigned int e;

  INTERRUPT_OFF();

  e = (SCB->icsr & 0xff) - 16;

  FAST_LOG('I', "irqS %d\n", e, 0);

  irq_call(e);

  FAST_LOG('I', "irqE %d\n", e, 0);

  INTERRUPT_ON();
}

void irq_enable(unsigned int n)
{
  unsigned int bit, reg;

  reg = n / 32;
  bit = n % 32;

  NVIC->iser[reg] |= BIT(bit);
}

void irq_disable(unsigned int n)
{
  unsigned int bit, reg;

  reg = n / 32;
  bit = n % 32;

  NVIC->icer[reg] |= BIT(bit);
}

void irq_set_pending(unsigned int n)
{
  unsigned int bit, reg;

  reg = n / 32;
  bit = n % 32;

  NVIC->ispr[reg] |= BIT(bit);
}

void irq_ack(unsigned int n)
{
  unsigned int bit, reg;

  reg = n / 32;
  bit = n % 32;

  NVIC->icpr[reg] = BIT(bit);
}

void irq_set_pri(unsigned int n, unsigned int pri)
{
  unsigned char *base = (unsigned char *)&NVIC->ip[0];

  base[n] = (unsigned char)(pri << 4);
}

#if 0
int cmd_nvic(int argc, char *argv[])
{
  unsigned int i;

  for (i = 0; i < 4; i++)
    xprintf("iser[%d]: %08x\n", i, NVIC->iser[i]);
  for (i = 0; i < 4; i++)
    xprintf("icer[%d]: %08x\n", i, NVIC->icer[i]);
  for (i = 0; i < 4; i++)
    xprintf("ispr[%d]: %08x\n", i, NVIC->ispr[i]);
  for (i = 0; i < 4; i++)
    xprintf("icpr[%d]: %08x\n", i, NVIC->icpr[i]);

  for (i = 0; i < 16; i++)
    xprintf("ip[%d]: %08x\n", i, NVIC->ip[i]);

  for (i = 0; i < 3; i++)
    xprintf("shp[%d]: %08x\n", i, SCB->shp[i]);

  return 0;
}

SHELL_CMD(nvic, cmd_nvic);
#endif

#define SCB_SCR_SLEEPDEEP BIT(2)

void set_low_power(int en)
{
  if (en)
    SCB->scr |= SCB_SCR_SLEEPDEEP;
  else
    SCB->scr &= ~SCB_SCR_SLEEPDEEP;
}

#define SCB_AIRCR_KEY (0x05faUL << 16)
#define SCB_AIRCR_SYSRESETREQ BIT(2)

void hal_cpu_reset()
{
  SCB->aircr = SCB_AIRCR_KEY | SCB_AIRCR_SYSRESETREQ;
}

#if 0
int cmd_ccr(int argc, char *argv[])
{
  xprintf("ccr: %08x\n", SCB->ccr);
  return 0;
}

SHELL_CMD(ccr, cmd_ccr);
#endif

#define DWT_CTRL_CYCCNTENA BIT(0)

#define DEBUG_DEMCR ((reg32_t *)0xE000EDFC)

#define DEBUG_DEMCR_TRCENA BIT(24)

void dwt_init()
{
  *DEBUG_DEMCR |= DEBUG_DEMCR_TRCENA;
  DWT->ctrl |= DWT_CTRL_CYCCNTENA;
}
