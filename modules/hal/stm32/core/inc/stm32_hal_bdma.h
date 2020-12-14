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

#ifndef STM32_HAL_BDMA_H
#define STM32_HAL_BDMA_H

#ifdef STM32_H7XX
#define BDMA1_BASE (void *)0x58025400
#else
#define BDMA1_BASE (void *)0x40020000
#define BDMA2_BASE (void *)0x40020400
#endif

#define CCR_CT BIT(16)
#define CCR_DBM BIT(15)
#define CCR_MEM2MEM BIT(14)
#define CCR_PL(_l_) (((_l_) & 0x3) << 12) /* prio */
#define CCR_MSIZ(_s_) (((_s_) & 0x3) << 10)
#define CCR_PSIZ(_s_) (((_s_) & 0x3) << 8)
#define CCR_MINC BIT(7)
#define CCR_PINC BIT(6)
#define CCR_CIRC BIT(5)
#define CCR_DIR BIT(4) /* 0 - from peripheral */
#define CCR_TEIE BIT(3)
#define CCR_HEIE BIT(2)
#define CCR_TCIE BIT(1)
#define CCR_EN BIT(0)

void stm32_bdma_trans(void *base, unsigned int chan,
                      void *src, void *dst, unsigned int n,
                      unsigned int flags);

void stm32_bdma_set_chan(void *base, unsigned int chan, unsigned int devid);
void stm32_bdma_en(void *base, unsigned int chan, int en);
void stm32_bdma_sw_trig(void *base, unsigned int chan);

#define IER_TEIF BIT(3)
#define IER_HTIF BIT(2)
#define IER_TCIF BIT(1)
#define IER_GIF BIT(0)

void stm32_bdma_irq_ack(void *base, unsigned int chan, unsigned int flags);

#endif
