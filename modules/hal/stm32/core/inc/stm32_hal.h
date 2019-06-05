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

#ifndef STM32_HAL_H
#define STM32_HAL_H

void delay(unsigned int cycles);
void led_set(unsigned int n, unsigned int v);

#define STM32_UART_LP BIT(0)

void debug_uart_init(void *base, unsigned int baud,
                     unsigned int clock, unsigned int flags);
void clock_init(void);
void led_init(void);

void enable_ahb1(unsigned int dev);
void enable_ahb2(unsigned int dev);
void enable_ahb3(unsigned int dev);
void enable_ahb4(unsigned int dev);
void enable_apb1(unsigned int dev);
void enable_apb2(unsigned int dev);
void enable_apb4(unsigned int dev);

void disable_ahb1(unsigned int dev);
void disable_ahb2(unsigned int dev);
void disable_ahb3(unsigned int dev);
void disable_ahb4(unsigned int dev);
void disable_apb1(unsigned int dev);
void disable_apb2(unsigned int dev);
void disable_apb4(unsigned int dev);

void clock_init_ls(void);

#endif
