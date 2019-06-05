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

#include <stdlib.h>
#include <string.h>

#include "hal_time.h"
#include "io.h"
#include "shell.h"

static int cmd_dm(int argc, char *argv[])
{
  unsigned int width = 4, ipl = 4, addr, count = 1, i, j;

  if (argc < 2)
    return -1;

  addr = strtoul(argv[1], 0, 16);

  if (strlen(argv[0]) > 2) {
    switch (argv[0][2]) {
    case 'h':
      width = 2;
      ipl = 8;
      break;
    case 'b':
      width = 1;
      ipl = 16;
      break;
    default:
      break;
    }
  }

  if (argc > 2)
    count = strtoul(argv[2], 0, 0);


  for (i = 0; i < count; i += ipl) {
    unsigned int left = count - i;
    if (left > ipl)
      left = ipl;
    xprintf("%08x: ", addr);
    for (j = 0; j < left; j++) {
      switch (width) {
      case 4:
        xprintf("%08x ", *(unsigned int *)addr);
        break;
      case 2:
        xprintf("%04x ", *(unsigned short *)addr);
        break;
      case 1:
        xprintf("%02x ", *(unsigned char *)addr);
        break;
      }
      addr += width;
    }
    xprintf("\n");
  }

  return 0;
}

SHELL_CMD(dm, cmd_dm);
SHELL_CMD(dmb, cmd_dm);
SHELL_CMD(dmh, cmd_dm);

static int cmd_sm(int argc, char *argv[])
{
  unsigned int width = 4, addr, count = 1, i;

  if (argc < 3)
    return -1;

  addr = strtoul(argv[1], 0, 16);

  if (strlen(argv[0]) > 2) {
    switch (argv[0][2]) {
    case 'h':
      width = 2;
      break;
    case 'b':
      width = 1;
      break;
    default:
      break;
    }
  }

  argv += 2;
  count = argc - 2;

  for (i = 0; i < count; i++, argv++) {
    unsigned int val = strtoul(*argv, 0, 0);
    switch (width) {
    case 4:
      *(unsigned int *)addr = val;
      break;
    case 2:
      *(unsigned short *)addr = (unsigned short)val;
      break;
    case 1:
      *(unsigned char *)addr = (unsigned char)val;
      break;
    }
    addr += width;
  }

  return 0;
}

SHELL_CMD(sm, cmd_sm);
SHELL_CMD(smb, cmd_sm);
SHELL_CMD(smh, cmd_sm);

static int cmd_mf(int argc, char *argv[])
{
  unsigned int width = 4, addr, saddr, count, i, j, val, start, rpt = 1;

  if (argc < 4)
    return -1;

  saddr = strtoul(argv[1], 0, 16);
  val = strtoul(argv[2], 0, 0);
  count = strtoul(argv[3], 0, 0);

  if (argc > 4)
    rpt = strtoul(argv[4], 0, 0);

  if (strlen(argv[0]) > 2) {
    switch (argv[0][2]) {
    case 'h':
      width = 2;
      break;
    case 'b':
      width = 1;
      break;
    default:
      break;
    }
  }

  start = hal_time_us();

  for (j = 0; j < rpt; j++) {
    addr = saddr;

    for (i = 0; i < count; i++) {
      switch (width) {
      case 4:
        *(unsigned int *)addr = val;
        break;
      case 2:
        *(unsigned short *)addr = (unsigned short)val;
        break;
      case 1:
        *(unsigned char *)addr = (unsigned char)val;
        break;
      }
      addr += width;
    }
  }

  xprintf("addr %08x val %x count %d rpt %d time %d\n", saddr, val, count, rpt,
          hal_time_us() - start);

  return 0;
}

SHELL_CMD(mf, cmd_mf);
SHELL_CMD(mfb, cmd_mf);
SHELL_CMD(mfh, cmd_mf);
