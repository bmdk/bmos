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
#include <stdio.h>
#include "common.h"
#include "shell.h"
#include "io.h"
#include "hal_time.h"
#include "hal_int.h"

#include "stm32_hal_i2c.h"

#define REG BIT(0)
#define RW BIT(1)
#define E BIT(2)
#define L BIT(3)
#define DATA(v) (((v) & 0xf) << 4)

static void hd44780_write_byte(unsigned int b)
{
  unsigned char c = b | L;

  i2c_write_buf((void *)I2C2_BASE, 0x27, &c, 1);
}

#define DELAY 500

static void write_reg(unsigned int b)
{
  unsigned int c = DATA(b >> 4);

  hd44780_write_byte(c | E);
  hal_delay_us(DELAY);
  hd44780_write_byte(c);
  hal_delay_us(DELAY);

  c = DATA(b);
  hd44780_write_byte(c | E);
  hal_delay_us(DELAY);
  hd44780_write_byte(c);
  hal_delay_us(DELAY);
}

static void write_reg_h(unsigned int b)
{
  unsigned int c = DATA(b);

  hd44780_write_byte(c | E);
  hal_delay_us(DELAY);
  hd44780_write_byte(c);
  hal_delay_us(DELAY);
}

static void write_data(unsigned int b)
{
  unsigned int c = DATA(b >> 4) | REG;

  hd44780_write_byte(c | E);
  hal_delay_us(DELAY);
  hd44780_write_byte(c);
  hal_delay_us(DELAY);

  c = DATA(b) | REG;
  hd44780_write_byte(c | E);
  hal_delay_us(DELAY);
  hd44780_write_byte(c);
  hal_delay_us(DELAY);
}

static void _hd44780_write_str(const char *str)
{
  char c;

  while ((c = *str++))
    write_data(c);
}

static void _hd44780_init()
{
  unsigned int status;

  status = interrupt_disable();

  write_reg_h(0x3); /* 8bit mode */
  hal_delay_us(4100);
  write_reg_h(0x3); /* 8bit mode */
  hal_delay_us(100);
  write_reg_h(0x3); /* 8bit mode */

  write_reg_h(0x2); /* 4bit mode */

  write_reg(0x28);  /* 4 bit mode, 2 display lines */

  write_reg(0x01);  /* clear display */
  write_reg(0x02);  /* reset to address 0 */
  write_reg(0x0c);  /* display on */
  interrupt_enable(status);
}

static void set_addr(unsigned int addr)
{
  write_reg(0x80 | (addr & 0x7f));
}

static void hd44780_init()
{
  _hd44780_init();
}

static void hd44780_write_str(unsigned int addr, const char *str)
{
  set_addr(addr);
  _hd44780_write_str(str);
}

static int disp_cmd(int argc, char *argv[])
{
  switch (argv[1][0]) {
  case 'i':
    hd44780_init();
    break;

  case 'w':
    if (argc < 4)
      return -1;
    hd44780_write_str(strtoul(argv[2], 0, 0), argv[3]);
    break;
  }

  return 0;
}

SHELL_CMD(disp, disp_cmd);
