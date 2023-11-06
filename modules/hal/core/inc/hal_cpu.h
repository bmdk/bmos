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

#ifndef HAL_CPU_H
#define HAL_CPU_H

/* define HAL_CPU_CLOCK here for special platforms - for instance to save
   space in cortex m0 code where integer division is implemented in software */

#if BOOT
#if BOARD_F030DEMO
#define HAL_CPU_CLOCK 8000000
#define DEBUG_USART_CLOCK HAL_CPU_CLOCK
#define DEBUG_USART_BAUD 115200
#elif BOARD_G030DEB
#define HAL_CPU_CLOCK 16000000
#define DEBUG_USART_CLOCK HAL_CPU_CLOCK
#define DEBUG_USART_BAUD 115200
#elif BOARD_G0B1N
#define HAL_CPU_CLOCK 16000000
#define DEBUG_USART_CLOCK HAL_CPU_CLOCK
#define DEBUG_USART_BAUD 115200
#elif BOARD_C031N
#define HAL_CPU_CLOCK 12000000
#define DEBUG_USART_CLOCK HAL_CPU_CLOCK
#define DEBUG_USART_BAUD 115200
#endif
#endif

#ifndef HAL_CPU_CLOCK
#define HAL_CPU_CLOCK hal_cpu_clock
#endif

void hal_cpu_reset(void);

#endif
