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

#ifndef COMMON_H
#define COMMON_H

#define BIT(_x_) (1UL << (_x_))
#define WBIT(_x_) (1ULL << (_x_))

#define ARRSIZ(arr) (sizeof(arr) / sizeof(arr[0]))

#define FFMASK(_n_) (0xffULL << ((_n_) << 3))

#define SWAP64(_v_) ((((uint64_t)(_v_) >> 56) & FFMASK(0)) | \
                     (((uint64_t)(_v_) >> 40) & FFMASK(1)) | \
                     (((uint64_t)(_v_) >> 24) & FFMASK(2)) | \
                     (((uint64_t)(_v_) >> 8)  & FFMASK(3)) | \
                     (((uint64_t)(_v_) << 8)  & FFMASK(4)) | \
                     (((uint64_t)(_v_) << 24) & FFMASK(5)) | \
                     (((uint64_t)(_v_) << 40) & FFMASK(6)) | \
                     (((uint64_t)(_v_) << 56) & FFMASK(7)))

#define SWAP32(_v_) ((((unsigned int)(_v_) << 24) & FFMASK(3)) | \
                     (((unsigned int)(_v_) <<  8) & FFMASK(2)) | \
                     (((unsigned int)(_v_) >>  8) & FFMASK(1)) | \
                     (((unsigned int)(_v_) >> 24) & FFMASK(0)))

#define SWAP16(_v_) ((((unsigned short)(_v_) <<  8) & FFMASK(1)) | \
                     (((unsigned short)(_v_) >>  8) & FFMASK(0)))

#define ALIGN(_num_, _n_) (((_num_) + (1U << (_n_)) - 1) & ~((1U << (_n_)) - 1))

#define WEAK __attribute__((__weak__))

typedef volatile unsigned int reg32_t;
typedef volatile unsigned short reg16_t;
typedef volatile unsigned char reg8_t;

#define RAMFUNC __attribute__((section(".ramfunc"), __long_call__, \
                               __noinline__))

#define _STRIFY(x) # x
#define STRIFY(_x_) _STRIFY(_x_)

#endif
