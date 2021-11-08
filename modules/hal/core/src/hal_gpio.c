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

#include <stdlib.h>

#include "hal_gpio.h"
#include "shell.h"
#include "io.h"

int cmd_gpio(int argc, char *argv[])
{
  unsigned int bank, pin, val;

  if (argc < 4)
    return -1;

  bank = atoi(argv[2]);
  pin = atoi(argv[3]);

  switch (argv[1][0]) {
  case 'd':
    if (argc < 5)
      return -1;

    val = atoi(argv[4]);
    switch (argv[4][0]) {
    case 'i':
      xprintf("gpio %d:%d i(nput)\n", bank, pin);
      gpio_init(GPIO(bank, pin), GPIO_INPUT);
      break;
    case 'o':
      xprintf("gpio %d:%d o(utput)\n", bank, pin);
      gpio_init(GPIO(bank, pin), GPIO_OUTPUT);
      break;
    default:
      xprintf("Invalid direction. Choose o(utput) or i(nput)\n");
      return 0;
    }

    break;

  case 'r':
    val = gpio_get(GPIO(bank, pin));
    xprintf("gpio %d:%d = %d\n", bank, pin, val);
    break;

  case 'w':
    if (argc < 5)
      return -1;

    val = atoi(argv[4]);
    gpio_set(GPIO(bank, pin), val);

    break;
  default:
    xprintf("Invalid subcommand '%c'. \n", argv[1][0]);
    break;
  }

  return 0;
}

SHELL_CMD_H(gpio, cmd_gpio,
            "configure and control GPIOs\n\n"
            "gpio w <bank> <pin> <0|1>: write value to gpio\n"
            "gpio r <bank> <pin>      : read value from gpio\n"
            "gpio d <bank> <pin> <i|o>: configure gpio direction"
            );
