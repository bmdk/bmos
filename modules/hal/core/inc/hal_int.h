/* Copyright (c) 2019-2021 Brian Thomas Murphy
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

#ifndef HAL_INT_H
#define HAL_INT_H

#include "hal_int_cpu.h"

void irq_enable(unsigned int n, int en);
void irq_ack(unsigned int n);
void irq_set_pri(unsigned int n, unsigned int pri);
void irq_set_pending(unsigned int n);

typedef void irq_handler_t (void *);

void irq_register(const char *name, irq_handler_t *handler,
                  void *data, unsigned int num);

void irq_call(unsigned int num);

#if 0
unsigned int interrupt_disable(void);
void interrupt_enable(unsigned int saved);
#define INTERRUPT_OFF() XXXX
#define INTERRUPT_ON() XXXX
#endif

#endif
