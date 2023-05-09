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

#include <stdarg.h>
#include <stdlib.h>

#include "fast_log.h"
#include "hal_cpu.h"
#include "hal_int.h"
#include "hal_time.h"
#include "io.h"
#include "shell.h"
#include "xassert.h"

#if BMOS
void sem_info(char opt, int debug);
void pool_info(char opt, int debug);
void queue_info(char opt, int debug);
void task_info(char opt, int debug);
#endif

void xdump(void)
{
  fast_log_dump(128, 1);

#if BMOS
  debug_printf("\nTASKS\n");
  task_info(0, 1);

  debug_printf("\nSEMAPHORES\n");
  sem_info('d', 1);

  debug_printf("\nPOOLS\n");
  pool_info('d', 1);

  debug_printf("\nQUEUES\n");
  queue_info('d', 1);
#endif
}

#if CONFIG_XPANIC_SMALL
void xpanic(const char *fmt, ...)
{
  INTERRUPT_OFF();
  for (;;)
    ;
}
#else
void xpanic(const char *fmt, ...)
{
  va_list ap;

  INTERRUPT_OFF();

  va_start(ap, fmt);
  debug_vprintf(fmt, ap);
  va_end(ap);

  xdump();

  hal_cpu_reset();

  for (;;)
    ;
}
#endif

int cmd_reset(int argc, char *argv[])
{
#if !ARCH_PICO
  unsigned int delay = 3;
#endif

  INTERRUPT_OFF();

#if !ARCH_PICO
  if (argc > 1)
    delay = atoi(argv[1]);

  hal_delay_us(1000000 * delay);
#endif

  hal_cpu_reset();
  return 0;
}

SHELL_CMD(reset, cmd_reset);
