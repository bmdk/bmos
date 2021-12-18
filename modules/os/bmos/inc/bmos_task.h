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

#ifndef BMOS_TASK_H
#define BMOS_TASK_H

typedef struct _bmos_task_t bmos_task_t;
typedef void task_fun_t (void *arg);

void task_start(void);
void task_delay(int time);
void task_wake(bmos_task_t *t);
bmos_task_t *task_init(task_fun_t *tf, void *ta,
                       const char *name, unsigned int prio,
                       void *stack, unsigned int stack_size);

#define TLS_IND_STDOUT 0

void *task_get_tls(unsigned int n);
void task_set_tls(unsigned int n, void *data);
bmos_task_t *task_get_current(void);

void systick_hook();

#endif
