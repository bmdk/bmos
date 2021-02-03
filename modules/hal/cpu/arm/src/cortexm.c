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

#define SYSTICK ((volatile systick_t *)SYSTICK_BASE)
#define SCB ((volatile scb_t *)SCB_BASE)
#define NVIC ((volatile nvic_t *)NVIC_BASE)

static void dump_stack_frame(stack_frame_t *sf)
{
  debug_printf("  r0 : %08x r1 : %08x r2 : %08x r3 : %08x\n", sf->r0, sf->r1,
               sf->r2, sf->r3);
  debug_printf("  r12: %08x\n", sf->r12);
  debug_printf("  pc : %08x lr : %08x sr : %08x\n", sf->pc, sf->lr, sf->xpsr);
}

void hal_cpu_init()
{
  unsigned int i;

  /* reset the exception vector to flash */
#if APPL
#if STM32_H7XX
  SCB->vtor = 0x8020000;
#else
#if RAM_APPL
  SCB->vtor = 0x20000000;
#else
  SCB->vtor = 0x8008000;
#endif
#endif
#else
  SCB->vtor = 0x8000000;
#endif

  /* enable 8 byte stack alignment
     unaligned access and divide by zero exception */
  SCB->ccr |= BIT(9) | BIT(4) | BIT(3);

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

void systick_init()
{
  SYSTICK->load = CLOCK / 1000 - 1;
  SYSTICK->val = 0;
  SYSTICK->ctrl = (1 << 2) | (1 << 1) | (1 << 0);
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
        debug_printf(" fault address(mm): %08x\n", SCB->mmfar);
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

void exception_handler(void) __attribute__((naked));
void exception_handler()
{
  register sw_stack_frame_t *xsf;
  unsigned int exc_return;

  asm volatile ("push {r4-r11};"
                "mov %0, sp;\n"
                "mov %1, r14;\n"
                : "=r" (xsf), "=r" (exc_return));

#if 0
  debug_printf("exc_return: %08x\n", exc_return);
#endif

  FAST_LOG('I', "exception\n", 0, 0);

  _exception_handler(xsf, exc_return);
}

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

void irq_enable(unsigned int n, int en)
{
  unsigned int bit, reg;

  reg = n / 32;
  bit = n % 32;

  if (en)
    NVIC->iser[reg] |= BIT(bit);
  else
    NVIC->iser[reg] &= ~BIT(bit);
}

void irq_set_pending(unsigned int n, int en)
{
  unsigned int bit, reg;

  reg = n / 32;
  bit = n % 32;

  if (en)
    NVIC->ispr[reg] |= BIT(bit);
  else
    NVIC->ispr[reg] &= ~BIT(bit);
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

void set_low_power(int en)
{
  if (en)
    SCB->scr |= BIT(2);
  else
    SCB->scr &= ~BIT(2);
}

void hal_cpu_reset()
{
  SCB->aircr = (0x5fa << 16) | BIT(2);
}

#if 0
int cmd_ccr(int argc, char *argv[])
{
  xprintf("ccr: %08x\n", SCB->ccr);
  return 0;
}

SHELL_CMD(ccr, cmd_ccr);
#endif

void trigger_pendsv()
{
  SCB->icsr = BIT(28);
}

void clear_pendsv()
{
  SCB->icsr = BIT(27);
}
