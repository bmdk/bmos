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

#ifndef STM32_TIMER_H
#define STM32_TIMER_H

#if STM32_G4XX || STM32_H5XX || STM32_C0XX || STM32_F0XX || \
    STM32_F1XX || STM32_G0XX
#define TIM1_BASE ((void *)0x40012C00)
#elif STM32_H7XX || STM32_F4XX
#define TIM1_BASE ((void *)0x40010000)
#elif AT32_F4XX
#define TIM1_BASE ((void *)0x40010000)
#endif

#define TIM2_BASE ((void *)0x40000000)
#define TIM3_BASE ((void *)0x40000400)
#define TIM4_BASE ((void *)0x40000800)
#define TIM5_BASE ((void *)0x40000c00)
#define TIM6_BASE ((void *)0x40001000)
#define TIM7_BASE ((void *)0x40001400)

/* from G4XX */
#define TIM15_BASE ((void *)0x40014000)
#define TIM16_BASE ((void *)0x40014400)
#define TIM17_BASE ((void *)0x40014C00)
#define TIM20_BASE ((void *)0x40015000)

void timer_init(void *base, unsigned int presc);
unsigned int timer_get(void *base);
unsigned int timer_get_cap(void *base, unsigned int unit);

void timer_init_dma(void *base, unsigned int presc, unsigned int max,
                    const unsigned int *compare, unsigned int compare_len,
                    int update_en);

void timer_init_pwm(void *base, unsigned int presc, unsigned int max,
                    const unsigned int *compare, unsigned int compare_len);

void timer_stop(void *base);

void timer_init_pwm(void *base, unsigned int presc, unsigned int max,
                    const unsigned int *compare, unsigned int compare_len);
void timer_set_compare(void *base, unsigned int num, unsigned int v);

void timer_init_encoder(void *base, unsigned int presc,
                        unsigned int max, unsigned int filter);

void timer_init_capture(void *base, unsigned int presc, unsigned int trig_src);

#define TIM_MMS_RESET 0
#define TIM_MMS_ENABLE 1
#define TIM_MMS_UPDATE 2
#define TIM_MMS_COMPARE_PULSE 3
#define TIM_MMS_COMPARE1 4
#define TIM_MMS_COMPARE2 5
#define TIM_MMS_COMPARE3 6
#define TIM_MMS_COMPARE4 7
#define TIM_MMS_ENCODER 8

/* set master output function to a slave timer (tim_trgo) */
void timer_mms(void *base, unsigned int mode);

unsigned int timer_get_clear_status(void *base);

#endif
