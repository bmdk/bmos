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

#ifndef STM32_HAL_ADC_H
#define STM32_HAL_ADC_H

#define ADC_CONV_DONE_TYPE_NONE 0
#define ADC_CONV_DONE_TYPE_HALF 1
#define ADC_CONV_DONE_TYPE_FULL 2

#define SAM_RATE_2_5 0
#define SAM_RATE_6_5 1
#define SAM_RATE_12_5 2
#define SAM_RATE_24_5 3
#define SAM_RATE_47_5 4
#define SAM_RATE_92_5 5
#define SAM_RATE_247_5 6
#define SAM_RATE_640_5 7

typedef void conv_done_f (unsigned short *adc_dat, unsigned int count,
                          unsigned int type);

void stm32_adc_init(unsigned int inst,
                    unsigned char *reg_seq, unsigned int cnt, int rate,
                    conv_done_f *conv_done);
void stm32_adc_init_dma(unsigned int inst,
                        unsigned char *reg_seq, unsigned int cnt, int rate,
                        void *buf, unsigned int buflen, conv_done_f *conv_done);
int stm32_adc_conv(unsigned int inst);

/* enable vbat resistor divider */
void stm32_adc_vbat(unsigned int inst, int en);

/* start a conversion from software */
int stm32_adc_start(unsigned int inst);

/* configure a trigger event */
void stm32_adc_trig_ev(unsigned int inst, int event);

#endif
