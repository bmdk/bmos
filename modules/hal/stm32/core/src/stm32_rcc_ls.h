#ifndef STM32_RCC_LS_H
#define STM32_RCC_LS_H

typedef struct {
  reg32_t bdcr;
  reg32_t csr;
} rcc_ls_t;

void rcc_clock_init_ls(rcc_ls_t *rcc_ls);

const char *rcc_clock_type_ls_str(rcc_ls_t *rcc_ls);

#endif
