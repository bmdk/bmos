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

#include "hal_gpio.h"
#include "ds1302.h"

void ds1302_init(ds1302_t *d)
{
  gpio_set(d->ce, 0);
  gpio_set(d->clk, 0);
  gpio_set(d->dat, 0);

  gpio_init(d->ce, GPIO_OUTPUT);
  gpio_init(d->clk, GPIO_OUTPUT);
  gpio_init(d->dat, GPIO_OUTPUT);
}

static void _ds1302_write(ds1302_t *d, unsigned int v)
{
  unsigned int i;

  gpio_init(d->dat, GPIO_OUTPUT);

  for (i = 0; i < 8; i++) {
    gpio_set(d->dat, (v & 1));

    v >>= 1;

    gpio_set(d->clk, 1);
    d->delay();
    gpio_set(d->clk, 0);
    d->delay();
  }
}

static unsigned int _ds1302_read(ds1302_t *d)
{
  unsigned int i, v = 0;

  gpio_init(d->dat, GPIO_INPUT);
  d->delay();

  for (i = 0; i < 8; i++) {
    v |= gpio_get(d->dat) << i;

    gpio_set(d->clk, 1);
    d->delay();
    gpio_set(d->clk, 0);
    d->delay();
  }

  return v;
}

unsigned int ds1302_read(ds1302_t *d, unsigned int cmd)
{
  unsigned int v;

  gpio_set(d->ce, 1);
  d->delay();
  _ds1302_write(d, cmd);
  v = _ds1302_read(d);
  d->delay();
  gpio_set(d->ce, 0);

  return v;
}

void ds1302_write(ds1302_t *d, unsigned int cmd, unsigned int val)
{
  gpio_set(d->ce, 1);
  d->delay();
  _ds1302_write(d, cmd);
  _ds1302_write(d, val);
  d->delay();
  gpio_set(d->ce, 0);
}

unsigned int ds1302_read_reg(ds1302_t *d, unsigned int r)
{
  unsigned int cmd = ((r & 0x1f) << 1) | 0x81;

  return ds1302_read(d, cmd);
}

void ds1302_read_regs(ds1302_t *d, unsigned char *data, unsigned int len)
{
  unsigned int i;

  gpio_set(d->ce, 1);
  d->delay();
  _ds1302_write(d, 0xbf);
  for (i = 0; i < len; i++)
    data[i] = _ds1302_read(d);
  d->delay();
  gpio_set(d->ce, 0);
}

void ds1302_write_regs(ds1302_t *d, unsigned char *data, unsigned int len)
{
  unsigned int i;

  gpio_set(d->ce, 1);
  d->delay();
  _ds1302_write(d, 0xbe);
  for (i = 0; i < len; i++)
    _ds1302_write(d, data[i]);
  d->delay();
  gpio_set(d->ce, 0);
}

unsigned int ds1302_read_ram(ds1302_t *d, unsigned int r)
{
  unsigned int cmd = ((r & 0x1f) << 1) | 0xc1;

  return ds1302_read(d, cmd);
}

void ds1302_write_ram(ds1302_t *d, unsigned int r, unsigned int val)
{
  unsigned int cmd = ((r & 0x1f) << 1) | 0xc0;

  ds1302_write(d, cmd, val);
}

void ds1302_write_reg(ds1302_t *d, unsigned int r, unsigned int val)
{
  unsigned int cmd = ((r & 0x1f) << 1) | 0x80;

  ds1302_write(d, cmd, val);
}
