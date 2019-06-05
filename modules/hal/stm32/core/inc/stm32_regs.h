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

#ifndef STM32_REGS_H
#define STM32_REGS_H

typedef struct {
  unsigned int cr;
  unsigned int icscr;
  unsigned int cfgr;
  unsigned int pllcfgr;
  unsigned int pllsai1cfgr;
  unsigned int pllsai2cfgr;
  unsigned int cier;
  unsigned int cifr;
  unsigned int cicr;
  unsigned int pad0;
  unsigned int ahb1rstr;
  unsigned int ahb2rstr;
  unsigned int ahb3rstr;
  unsigned int pad1;
  unsigned int apb1rstr1;
  unsigned int apb1rstr2;
  unsigned int apb2rstr;
  unsigned int pad2;
  unsigned int ahb1enr;
  unsigned int ahb2enr;
  unsigned int ahb3enr;
  unsigned int pad3;
  unsigned int apb1enr1;
  unsigned int apb1enr2;
  unsigned int apb2enr;
  unsigned int pad4;
  unsigned int ahb1smenr;
  unsigned int ahb2smenr;
  unsigned int ahb3smenr;
  unsigned int pad5;
  unsigned int apb1smenr1;
  unsigned int apb1smenr2;
  unsigned int apb2smenr;
  unsigned int pad6;
  unsigned int ccipr;
  unsigned int pad7;
  unsigned int bdcr;
  unsigned int csr;
  unsigned int crrcr;
  unsigned int ccipr2;
} stm32_rcc_t;

typedef struct {
  unsigned int cr;
  unsigned int pllcfgr;
  unsigned int cfgr;
  unsigned int cir;
  unsigned int ahb1rstr;
  unsigned int ahb2rstr;
  unsigned int ahb3rstr;
  unsigned int pad1;
  unsigned int apb1rstr;
  unsigned int apb2rstr;
  unsigned int pad2[2];
  unsigned int ahb1enr;
  unsigned int ahb2enr;
  unsigned int ahb3enr;
  unsigned int pad3;
  unsigned int apb1enr;
  unsigned int apb2enr;
  unsigned int pad4[2];
  unsigned int ahb1lpenr;
  unsigned int ahb2lpenr;
  unsigned int ahb3lpenr;
  unsigned int pad5;
  unsigned int apb1lpenr;
  unsigned int apb2lpenr;
  unsigned int pad6[2];
  unsigned int bdcr;
  unsigned int csr;
  unsigned int pad7[2];
  unsigned int sscgr;
  unsigned int pll12scfgr;
  unsigned int pllsaicfgr;
  unsigned int dckcfgr1;
  unsigned int dckcfgr2;
} stm32_rcc_a_t;

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

typedef struct {
  unsigned int sr;
  unsigned int dr;
  unsigned int brr;
  unsigned int cr1;
  unsigned int cr2;
  unsigned int cr3;
  unsigned int gtpr;
} stm32_usart_a_t;

typedef struct {
  unsigned int mcr;
  unsigned int msr;
  unsigned int tsr;
  unsigned int rfr[2];
  unsigned int ier;
  unsigned int esr;
  unsigned int btr;
  unsigned int pad0[88];

  struct {
    unsigned int i;
    unsigned int dt;
    unsigned int d[2];
  } t[3];

  struct {
    unsigned int i;
    unsigned int dt;
    unsigned int d[2];
  } r[2];

  unsigned int pad1[12];

  unsigned int fmr;
  unsigned int fm1r;
  unsigned int pad2;
  unsigned int fs1r;
  unsigned int pad3;
  unsigned int ffa1r;
  unsigned int pad4;
  unsigned int fa1r;

  unsigned int pad5[8];

  struct {
    unsigned int id;
    unsigned int mask;
  } f[28];

} stm32_can_t;

typedef struct {
  unsigned int pad0[2];
  unsigned int sscr;
  unsigned int bpcr;
  unsigned int awcr;
  unsigned int twcr;
  unsigned int gcr;
  unsigned int pad1[2];
  unsigned int srcr;
  unsigned int bccr;
  unsigned int pad2[2];
  unsigned int ier;
  unsigned int isr;
  unsigned int icr;
  unsigned int lipcr;
  unsigned int cpsr;
  unsigned int cdsr;
  unsigned int pad3[14];
  unsigned int l1cr;
  unsigned int l1whpcr;
  unsigned int l1wvpcr;
  unsigned int l1ckcr;
  unsigned int l1pfcr;
  unsigned int l1cacr;
  unsigned int l1dccr;
  unsigned int l1bfcr;
  unsigned int pad4[2];
  unsigned int l1cfbar;
  unsigned int l1cfblr;
  unsigned int l1cfblnr;
  unsigned int pad5[3];
  unsigned int l1clutwr;
} stm32_lcd_t;

#endif
