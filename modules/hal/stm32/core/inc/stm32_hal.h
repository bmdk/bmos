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

#ifndef STM32_HAL_H
#define STM32_HAL_H

#include "hal_gpio.h"


static inline void delay(unsigned int count)
{
  unsigned int i;

  for (i = 0; i < count; i++)
    asm volatile ("nop");
}

//void delay(unsigned int cycles);
void led_set(unsigned int n, unsigned int v);

#define STM32_UART_LP BIT(0)
#define STM32_UART_FIFO BIT(1)
#define STM32_UART_SINGLE_WIRE BIT(2)

void led_init(const gpio_handle_t *led_list, unsigned int _nleds);

#define LED_FLAG_INV BIT(0)

typedef unsigned char led_flag_t;

void led_init_flags(const gpio_handle_t *led_list,
                    const led_flag_t *led_flags, unsigned int _nleds);

void stm32_get_udid(void *buf, unsigned int len);

#if !CONFIG_BUS_ENABLE_INLINE
void enable_ahb1(unsigned int dev);
void enable_ahb2(unsigned int dev);
void enable_ahb3(unsigned int dev);
void enable_ahb4(unsigned int dev);
void enable_apb1(unsigned int dev);
void enable_apb2(unsigned int dev);
void enable_apb3(unsigned int dev);
void enable_apb4(unsigned int dev);

void disable_ahb1(unsigned int dev);
void disable_ahb2(unsigned int dev);
void disable_ahb3(unsigned int dev);
void disable_ahb4(unsigned int dev);
void disable_apb1(unsigned int dev);
void disable_apb2(unsigned int dev);
void disable_apb3(unsigned int dev);
void disable_apb4(unsigned int dev);
#endif

void clock_init_ls(int internal);
const char *clock_ls_name(void);

#endif
