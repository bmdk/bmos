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

#include "common.h"
#include "hal_dma_if.h"

dma_controller_t stm32_bdma_controller;
dma_controller_t stm32_dma_controller;
dma_controller_t stm32_gpdma_controller;

/* *INDENT-OFF* */
#ifdef STM32_H7XX
dma_cont_data_t dma_cont_data[] = {
  { &stm32_dma_controller,  (void *)0x40020000 },
  { &stm32_dma_controller,  (void *)0x40020400 },
  { &stm32_bdma_controller, (void *)0x58025400 },
};
#elif STM32_F4XX || STM32_F7XX
dma_cont_data_t dma_cont_data[] = {
  { &stm32_dma_controller, (void *)0x40026000 },
  { &stm32_dma_controller, (void *)0x40026400 },
};
#elif STM32_L4XX || STM32_L4R || STM32_G0XX || \
      STM32_G4XX || STM32_WBXX || STM32_F1XX || STM32_F3XX || \
      AT32_F4XX
dma_cont_data_t dma_cont_data[] = {
  { &stm32_bdma_controller, (void *)0x40020000 },
  { &stm32_bdma_controller, (void *)0x40020400 }
};
#elif STM32_UXXX
dma_cont_data_t dma_cont_data[] = {
  { &stm32_gpdma_controller, (void *)0x40020000 },
};
#elif STM32_F0XX
dma_cont_data_t dma_cont_data[] = {
  { &stm32_bdma_controller, (void *)0x40020000 },
};
#elif STM32_L0XX
dma_cont_data_t dma_cont_data[] = {
  { &stm32_bdma_controller, (void *)0x40020000 },
};
#else
#error Define dma controller configuration for this platform
#endif
/* *INDENT-ON* */

unsigned int dma_cont_data_len = ARRSIZ(dma_cont_data);
