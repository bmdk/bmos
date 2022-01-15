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
#if __ARM_ARCH_6M__
	ldr	r0, =_stack_end
	mov	sp, r0
#else
	ldr	sp, =_stack_end
#endif
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

.word svc_handler
.word exception_handler
.word exception_handler
.word pendsv_handler
.word systick_handler

#ifdef CONFIG_N_INTS
#define N_INTS CONFIG_N_INTS
#else
#define N_INTS 128
#endif

.rept N_INTS
.word interrupt_handler
.endr

.type   pendsv_handler, %function
pendsv_handler:
      cpsid i

      mrs r0, psp
#if __ARM_ARCH_6M__
      mov r1, sp
      mov sp, r0

      push {r4-r7}
      mov r4, r8
      mov r5, r9
      mov r6, r10
      mov r7, r11
      push {r4-r7}

      mov r0, sp
      mov sp, r1
#else
      stmdb r0!, {r4-r11}
#endif

      bl _pendsv_handler

#if __ARM_ARCH_6M__
      mov r1, sp
      mov sp, r0

      pop {r4-r7}
      mov r8, r4
      mov r9, r5
      mov r10, r6
      mov r11, r7
      pop {r4-r7}

      mov r0, sp
      mov sp, r1
#else
      ldmfd r0!, {r4-r11}
#endif
      msr psp, r0

      ldr r0, =0xfffffffd

      cpsie i

      bx r0
