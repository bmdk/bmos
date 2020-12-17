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

#define DMA_CHANNELS 8

#define DMA_CR_CHSEL(_l_) (((_l_) & 0x7) << 25)
#define DMA_CR_MBURST(_l_) (((_l_) & 0x3) << 23)
#define DMA_CR_MBURST_SINGLE CR_MBURST(0)
#define DMA_CR_MBURST_INCR4 CR_MBURST(1)
#define DMA_CR_MBURST_INCR8 CR_MBURST(2)
#define DMA_CR_MBURST_INCR16 CR_MBURST(3)
#define DMA_CR_PBURST(_l_) (((_l_) & 0x3) << 21)
#define DMA_CR_PBURST_SINGLE CR_PBURST(0)
#define DMA_CR_PBURST_INCR4 CR_PBURST(1)
#define DMA_CR_PBURST_INCR8 CR_PBURST(2)
#define DMA_CR_PBURST_INCR16 CR_PBURST(3)
#define DMA_CR_TRBUFF BIT(20)
#define DMA_CR_CT BIT(19)
#define DMA_CR_DBM BIT(18)
#define DMA_CR_PL(_l_) (((_l_) & 0x3) << 16) /* prio */
#define DMA_CR_PINCOS BIT(15)
#define DMA_CR_MSIZ(_s_) (((_s_) & 0x3) << 13)
#define DMA_CR_PSIZ(_s_) (((_s_) & 0x3) << 11)
#define DMA_CR_MINC BIT(10)
#define DMA_CR_PINC BIT(9)
#define DMA_CR_CIRC BIT(8)
#define DMA_CR_DIR(_s_) (((_s_) & 0x3) << 6)
#define DMA_CR_DIR_P2M DMA_CR_DIR(0)
#define DMA_CR_DIR_M2P DMA_CR_DIR(1)
#define DMA_CR_DIR_M2M DMA_CR_DIR(2)
#define DMA_CR_PFCTRL BIT(5)
#define DMA_CR_TCIE BIT(4)
#define DMA_CR_HTIE BIT(3)
#define DMA_CR_TEIE BIT(2)
#define DMA_CR_DMEIE BIT(1)
#define DMA_CR_EN BIT(0)

#define DMA_FCR_DMDIS BIT(2)

void stm32_dma_trans(unsigned int num, unsigned int chan,
                     void *src, void *dst, unsigned int n,
                     unsigned int flags);

void stm32_dma_set_chan(unsigned int num, unsigned int chan,
                        unsigned int devid);
void stm32_dma_en(unsigned int num, unsigned int chan, int en);
void stm32_dma_sw_trig(unsigned int num, unsigned int chan);

#endif
