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

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

typedef unsigned int gpio_handle_t;

#if ARCH_AVR
#define GPIO(bank, pin) (((bank & 0xf) << 4) | ((pin) & 0xf))

#define GPIO_BANK(gpio) ((gpio >> 4) & 0xf)
#define GPIO_PIN(gpio) ((gpio) & 0xf)
#else
#define GPIO(bank, pin) (((bank & 0xffff) << 16) | ((pin) & 0xffff))

#define GPIO_BANK(gpio) ((gpio >> 16) & 0xffff)
#define GPIO_PIN(gpio) ((gpio) & 0xffff)
#endif

#define GPIO_INPUT 0
#define GPIO_OUTPUT 1

void gpio_init(gpio_handle_t gpio, unsigned int type);

void gpio_set(gpio_handle_t gpio, int val);

int gpio_get(gpio_handle_t gpio);

void gpio_init_attr(gpio_handle_t gpio, unsigned int attr);

#endif
