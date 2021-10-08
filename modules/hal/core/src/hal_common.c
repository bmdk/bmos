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

#include <shell.h>
#include "io.h"
#include "common.h"
#include "xassert.h"

#include "hal_board.h"

extern unsigned int _fsdata, _rsdata, _redata, _sbss, _ebss, _stack_end;
extern unsigned int _flash_start, _flash_end, _ram_start, _end;

static unsigned int sbrk_next = (unsigned int)&_end;

void _data_init(void)
{
  unsigned int *si, *s, *e;

  si = &_fsdata;
  s = &_rsdata;

  /* copy initialized data from flash to ram */
  if (s != si) {
    e = &_redata;

    while (s < e)
      *s++ = *si++;
  }

  s = &_sbss;
  e = &_ebss;

  while (s < e)
    *s++ = 0;
}

static int cmd_mem(int argc, char *argv[])
{
  unsigned int data_start = (unsigned int)&_rsdata;
  unsigned int data_start_flash = (unsigned int)&_fsdata;
  unsigned int data_size = (unsigned int)&_redata - (unsigned int)&_rsdata;

  if (data_start == data_start_flash) {
    xprintf(" code: %08x - %08x: %6d\n", (unsigned int)&_flash_start,
            (unsigned int)data_start,
            (unsigned int)data_start - (unsigned int)&_flash_start);
  }

  xprintf("  ram: %08x - %08x: %6d\n", (unsigned int)&_ram_start,
          (unsigned int)&_stack_end,
          (unsigned int)&_stack_end - (unsigned int)&_ram_start);
  xprintf(" data: %08x - %08x: %6d\n", data_start,
          data_start + data_size, data_size);
  xprintf("  bss: %08x - %08x: %6d\n", (unsigned int)&_sbss,
          (unsigned int)&_ebss,
          (unsigned int)&_ebss - (unsigned int)&_sbss);
  xprintf(" heap: %08x - %08x: %6d\n", (unsigned int)&_end,
          sbrk_next, sbrk_next - (unsigned int)&_end);

  if (data_start != data_start_flash) {
    xprintf("\n");
    xprintf("flash: %08x - %08x: %6d\n", (unsigned int)&_flash_start,
            (unsigned int)&_flash_end,
            (unsigned int)&_flash_end - (unsigned int)&_flash_start);
    xprintf("fdata: %08x - %08x: %6d\n", data_start_flash,
            data_start_flash + data_size, data_size);
  }

  return 0;
}

SHELL_CMD(mem, cmd_mem);

static int cmd_hal(int argc, char *argv[])
{
  char cmd = 'c';

  if (argc > 1)
    cmd = argv[1][0];

  switch (cmd) {
  case 'c':
    xprintf("CPU clock: %d\n", hal_cpu_clock);
    break;
  }
  return 0;
}

SHELL_CMD(hal, cmd_hal);

#ifndef PRIVATE_SBRK
#define _sbrk sbrk
#endif

void *_sbrk(int count)
{
  unsigned int cur = sbrk_next;

  XASSERT(count >= 0);

  if (count > 0)
    /* align to next 8 byte boundary */
    sbrk_next += ALIGN(count, 3);

#if 0
  xprintf("%08x %d %d\n", cur, count, ALIGN(count, 3));
#endif
  return (void *)cur;
}
