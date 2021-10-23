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

#ifndef COMMON_H
#define COMMON_H

#define BIT(_x_) (1U << (_x_))

#define ARRSIZ(arr) (sizeof(arr) / sizeof(arr[0]))

#define SWAP32(_x_) ((((unsigned int)(_x_) << 24) & 0xff000000U) | \
                     (((unsigned int)(_x_) <<  8) & 0x00ff0000U) | \
                     (((unsigned int)(_x_) >>  8) & 0x0000ff00U) | \
                     (((unsigned int)(_x_) >> 24) & 0x000000ffU))

#define SWAP16(_x_) ((((unsigned short)_x_ <<  8) & 0xff00) | \
                     (((unsigned short)_x_ >>  8) & 0x00ff))

#define ALIGN(_num_, _n_) (((_num_) + (1U << (_n_)) - 1) & ~((1U << (_n_)) - 1))

#define WEAK __attribute__((__weak__))

#endif
