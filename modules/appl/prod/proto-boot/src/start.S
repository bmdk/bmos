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

.syntax unified
.thumb
.section .text.reset_handler
.globl	reset_handler
.type   reset_handler, %function
reset_handler:
	ldr	sp, =_stack_end
	bl	_data_init
#if CONFIG_NEWLIB
	bl	__libc_init_array
#endif
	bl	main

forever:
	b forever

.section .isr_vector,"a",%progbits
vector_table:
.word _stack_end
.word reset_handler

.rept 9
.word exception_handler
.endr

.word exception_handler
.word exception_handler
.word exception_handler
.word exception_handler
.word systick_handler

#ifdef CONFIG_N_INTS
#define N_INTS CONFIG_N_INTS
#else
#define N_INTS 128
#endif

.rept N_INTS
.word interrupt_handler
.endr
