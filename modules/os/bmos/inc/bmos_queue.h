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

#ifndef BMOS_QUEUE_H
#define BMOS_QUEUE_H

typedef struct _bmos_queue_t bmos_queue_t;

#define QUEUE_TYPE_DRIVER 0
#define QUEUE_TYPE_TASK 1

bmos_queue_t *queue_create(const char *name, unsigned int type);

typedef void bmos_queue_put_f_t (void *data);

int queue_set_put_f(bmos_queue_t *queue, bmos_queue_put_f_t *f, void *f_data);

void queue_destroy(bmos_queue_t *queue);

bmos_queue_t *queue_lookup(const char *name);

const char *queue_get_name(bmos_queue_t *queue);

unsigned int queue_get_count(bmos_queue_t *queue);

#endif
