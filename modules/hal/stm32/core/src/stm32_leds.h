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

#ifndef STM32_LEDS_H
#define STM32_LEDS_H

#ifdef STM32_F767
#define STM32_LEDS GPIO(1, 14), GPIO(1, 7)
#elif STM32_F746
#define STM32_LEDS GPIO(8, 1)
#elif STM32_L496
#define STM32_LEDS GPIO(1, 14), GPIO(1, 7)
#elif STM32_L452
#define STM32_LEDS GPIO(1, 13)
#elif STM32_L432
#define STM32_LEDS GPIO(1, 3)
#elif STM32_L4R
#define STM32_LEDS GPIO(2, 7)
#elif STM32_F429
#define STM32_LEDS GPIO(6, 13), GPIO(6, 14)
#elif STM32_H7XX
#define STM32_LEDS GPIO(1, 0), GPIO(1, 14), GPIO(1, 7)
#else
#define STM32_LEDS GPIO(1, 3), GPIO(1, 4)
#endif

#endif
