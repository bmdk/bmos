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

#ifndef STM32_GPIO_H
#define STM32_GPIO_H

#include "common.h"

/* F1XX stuff */
#define GPIO_ATTR_STM32F1(cnf, mode) \
  ((((unsigned int)(cnf) & 0x3) << 2) | \
   ((unsigned int)mode & 0x3))

#define MODE_INPUT 0
#define MODE_OUTPUT_LOW 1
#define MODE_OUTPUT_MED 2
#define MODE_OUTPUT_HIG 2

#define CNF_INP_ANA 0
#define CNF_INP_FLO 1
/* pull up or down is set with the output register */
#define CNF_INP_PUD 2

#define CNF_OUT_PP 0
#define CNF_OUT_OD 1
#define CNF_ALT_PP 2
#define CNF_ALT_OD 3

/* the rest */
#define GPIO_ALT 2
#define GPIO_ANALOG 3

#define GPIO_SPEED_LOW 0
#define GPIO_SPEED_MED 1
#define GPIO_SPEED_HIG 2
#define GPIO_SPEED_VHI 3

#define GPIO_FLAG_PULL_NONE 0
#define GPIO_FLAG_PULL_PU 1
#define GPIO_FLAG_PULL_PD 2
#define GPIO_FLAG_OPEN_DRAIN BIT(2)

#define GPIO_ATTR_STM32(flags, speed, alt, type) \
  ((((flags) & 0xf) << 8) | ((alt & 0xf) << 4) | \
   (((speed) & 0x3) << 2) | ((type) & 0x3))

#define GPIO_ATTR_STM32_ALT(attr) (((attr) >> 4) & 0xf)
#define GPIO_ATTR_STM32_FLAGS(attr) (((attr) >> 8) & 0xf)
#define GPIO_ATTR_STM32_SPEED(attr) (((attr) >> 2) & 0x3)
#define GPIO_ATTR_STM32_TYPE(attr) ((attr) & 0x3)

#if STM32_F1XX
typedef struct {
  reg32_t cr[2];
  reg32_t idr;
  reg32_t odr;
  reg32_t bsrr;
  reg32_t brr;
  reg32_t lckr;
} stm32_gpio_t;
#else
typedef struct {
  reg32_t moder;
  reg32_t otyper;
  reg32_t ospeedr;
  reg32_t pupdr;
  reg32_t idr;
  reg32_t odr;
  reg32_t bsrr;
  reg32_t lckr;
  reg32_t afrl;
  reg32_t afrh;
  reg32_t brr;
} stm32_gpio_t;
#endif

#if STM32_L4R || STM32_L4XX || STM32_G4XX || STM32_WBXX || STM32_F0XX
#define GPIO_BASE 0x48000000
#elif STM32_G0XX
#define GPIO_BASE 0x50000000
#elif STM32_H7XX
#define GPIO_BASE 0x58020000
#elif STM32_UXXX
#define GPIO_BASE 0x42020000
#elif STM32_F1XX
#define GPIO_BASE 0x40010800
#else
#define GPIO_BASE 0x40020000
#endif

#define STM32_GPIO(port) ((stm32_gpio_t *)(GPIO_BASE + (0x400 * (port))))

#endif
