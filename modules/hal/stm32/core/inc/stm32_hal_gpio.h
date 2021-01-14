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

#ifndef STM32_GPIO_H
#define STM32_GPIO_H

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

typedef struct {
  unsigned int moder;
  unsigned int otyper;
  unsigned int ospeedr;
  unsigned int pupdr;
  unsigned int idr;
  unsigned int odr;
  unsigned int bsrr;
  unsigned int lckr;
  unsigned int afrl;
  unsigned int afrh;
  unsigned int brr;
} stm32_gpio_t;

#if STM32_L4R || STM32_L4XX || STM32_G4XX
#define GPIO_BASE 0x48000000
#elif STM32_H7XX
#define GPIO_BASE 0x58020000
#else
#define GPIO_BASE 0x40020000
#endif

#define STM32_GPIO(port) ((volatile stm32_gpio_t *)(GPIO_BASE + \
                                                    (0x400 * (port))))

#endif
