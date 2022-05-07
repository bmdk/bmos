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

#include <stdlib.h>
#include <stdio.h>
#include "common.h"

#include "hd44780.h"

#define REG BIT(0)
#define RW BIT(1)
#define E BIT(2)
#define L BIT(3)
#define DATA(v) (((v) & 0xf) << 4)

static void write_byte(hd44780_data_t *d, unsigned int b)
{
  d->write_byte(b | L);
}

static void write_reg(hd44780_data_t *d, unsigned int b)
{
  unsigned int c = DATA(b >> 4);

  write_byte(d, c | E);
  d->delay();
  write_byte(d, c);
  d->delay();

  c = DATA(b);
  write_byte(d, c | E);
  d->delay();
  write_byte(d, c);
  d->delay();
}

static void write_reg_h(hd44780_data_t *d, unsigned int b)
{
  unsigned int c = DATA(b);

  write_byte(d, c | E);
  d->delay();
  write_byte(d, c);
  d->delay();
}

static void write_data(hd44780_data_t *d, unsigned int b)
{
  unsigned int c = DATA(b >> 4) | REG;

  write_byte(d, c | E);
  d->delay();
  write_byte(d, c);
  d->delay();

  c = DATA(b) | REG;
  write_byte(d, c | E);
  d->delay();
  write_byte(d, c);
  d->delay();
}

static void _hd44780_write_str(hd44780_data_t *d, const char *str)
{
  char c;

  while ((c = *str++))
    write_data(d, c);
}

void hd44780_clear(hd44780_data_t *d)
{
  write_reg(d, 0x01);
}

/* reset cursor position to 0 */
void hd44780_home(hd44780_data_t *d)
{
  write_reg(d, 0x02);
}

/* display control */
void hd44780_disp(hd44780_data_t *d, int en, int cursor, int blink)
{
  unsigned int val = BIT(3);

  if (en)
    val |= BIT(2);
  if (cursor)
    val |= BIT(1);
  if (blink)
    val |= BIT(0);

  write_reg(d, val);
}

void hd44780_shift(hd44780_data_t *d, int cursor, int right)
{
  unsigned int val = BIT(4);

  if (!cursor)
    val |= BIT(3);
  if (right)
    val |= BIT(2);

  write_reg(d, val);
}

static void _hd44780_init(hd44780_data_t *d)
{
  write_reg_h(d, 0x3); /* 8bit mode */
  d->delay();
  write_reg_h(d, 0x3); /* 8bit mode */
  d->delay();
  write_reg_h(d, 0x3); /* 8bit mode */

  write_reg_h(d, 0x2); /* 4bit mode */

  write_reg(d, 0x28);  /* 4 bit mode, 2 display lines */

  hd44780_clear(d);
  hd44780_home(d);
  hd44780_disp(d, 1, 0, 0);
}

static void set_addr(hd44780_data_t *d, unsigned int addr)
{
  write_reg(d, 0x80 | (addr & 0x7f));
}

void hd44780_init(hd44780_data_t *d)
{
  _hd44780_init(d);
}

void hd44780_write_str(hd44780_data_t *d, unsigned int addr, const char *str)
{
  set_addr(d, addr);
  _hd44780_write_str(d, str);
}
