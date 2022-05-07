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

#ifndef HD44780_H
#define HD44780_H

typedef void _hd44780_write_byte(unsigned int b);
typedef void _hd44780_delay(void);

typedef struct {
  _hd44780_write_byte *write_byte;
  _hd44780_delay *delay;
} hd44780_data_t;

void hd44780_init(hd44780_data_t *d);
void hd44780_write_str(hd44780_data_t *d, unsigned int addr, const char *str);
void hd44780_clear(hd44780_data_t *d);
void hd44780_home(hd44780_data_t *d);
void hd44780_disp(hd44780_data_t *d, int en, int cursor, int blink);
void hd44780_shift(hd44780_data_t *d, int cursor, int right);

#endif
