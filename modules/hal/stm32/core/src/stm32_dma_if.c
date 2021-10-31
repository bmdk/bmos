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
#elif STM32_F429 || STM32_F411 || STM32_F401 || \
  STM32_F4XX || STM32_F746 || STM32_F767
dma_cont_data_t dma_cont_data[] = {
  { &stm32_dma_controller, (void *)0x40026000 },
  { &stm32_dma_controller, (void *)0x40026400 },
};
#elif STM32_L4XX || STM32_L4R || STM32_G4XX || STM32_WBXX
dma_cont_data_t dma_cont_data[] = {
  { &stm32_bdma_controller, (void *)0x40020000 },
  { &stm32_bdma_controller, (void *)0x40020400 }
};
#elif STM32_UXXX
dma_cont_data_t dma_cont_data[] = {
  { &stm32_gpdma_controller, (void *)0x40020000 },
};
#else
#error Define dma controller configuration for this platform
#endif
/* *INDENT-ON* */

unsigned int dma_cont_data_len = ARRSIZ(dma_cont_data);
