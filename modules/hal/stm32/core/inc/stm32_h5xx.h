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

#ifndef STM32_H5XX_H
#define STM32_H5XX_H

/*
 #define TIM1_BASE 0x40012C00
 #define TIM2_BASE 0x40000000
 */
#define EXTI_BASE 0x44022000
#define RTC_BASE 0x44007800
#define DMA1_BASE 0x40020000
#define DMA2_BASE 0x40021000
#define ADC_BASE 0x42028000
#define ETH_BASE 0x40028000
#define USB2_BASE 0x40016000
#define USART2_BASE 0x40004400
#define USART3_BASE 0x40004800
#define FDCAN1_BASE 0x4000a400
#define FDCAN2_BASE 0x4000a800

#define PWR_BASE 0x44020800
#define GPIO_BASE 0x42020000
#define RCC_BASE 0x44020C00
#define SBS_BASE 0x44000400

#endif
