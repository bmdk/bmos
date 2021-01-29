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

#ifndef STM32_PWR_FXXX
#define STM32_PWR_FXXX

typedef struct {
  unsigned int memrmp;
  unsigned int pmc;
  unsigned int exticr[4];
  unsigned int rsvd0;
  unsigned int cbr;
  unsigned int cmpcr;
} stm32_syscfg_t;

typedef struct {
  unsigned int cr;
  unsigned int csr;
} stm32_pwr_t;

#define SYSCFG ((volatile stm32_syscfg_t *)(0x40013800))
#define PWR ((volatile stm32_pwr_t *)(0x40007000))

#define PWR_CR1_DBP BIT(8)

/* choose between rmii and mii interface */
void stm32_syscfg_eth_phy(unsigned int rmii);

void stm32_pwr_vos(unsigned int vos);
int stm32_pwr_vos_rdy(void);

#endif
