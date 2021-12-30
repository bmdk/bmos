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

#ifndef DS1302_H
#define DS1302_H

#include "hal_gpio.h"

typedef void delay_t (void);

typedef struct {
  gpio_handle_t clk;
  gpio_handle_t dat;
  gpio_handle_t ce;
  delay_t *delay;
} ds1302_t;

void ds1302_init(ds1302_t *d);
unsigned int ds1302_read_reg(ds1302_t *d, unsigned int r);
unsigned int ds1302_read_ram(ds1302_t *d, unsigned int r);
void ds1302_write_ram(ds1302_t *d, unsigned int r, unsigned int val);
void ds1302_write_reg(ds1302_t *d, unsigned int r, unsigned int val);
void ds1302_read_regs(ds1302_t *d, unsigned char *data, unsigned int len);
void ds1302_write_regs(ds1302_t *d, unsigned char *data, unsigned int len);

#endif
