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

#ifndef STM32_TIMER_H
#define STM32_TIMER_H

#if STM32_H7XX
#define TIM1_BASE ((void *)0x40010000)
#define TIM2_BASE ((void *)0x40000000)
#define TIM3_BASE ((void *)0x40000400)
#define TIM4_BASE ((void *)0x40000800)
#define TIM5_BASE ((void *)0x40000c00)
#define TIM6_BASE ((void *)0x40001000)
#define TIM7_BASE ((void *)0x40001400)
#else
#if STM32_F411 || STM32_F401
#define TIM1_BASE ((void *)0x40010000)
#else
#define TIM1_BASE ((void *)0x40012C00)
#endif
#define TIM2_BASE ((void *)0x40000000)
#define TIM3_BASE ((void *)0x40000400)
#endif

void timer_init(void *base, unsigned int presc);
unsigned int timer_get(void *base);

void timer_init_dma(void *base, unsigned int presc, unsigned int max,
                    const unsigned int *compare, unsigned int compare_len,
                    int update_en);

void timer_stop(void *base);

#endif
