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

ENTRY(reset_handler)

MEMORY
{
  RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 256k
  FLASH (rx)      : ORIGIN = 0x10000000, LENGTH = 2048k
}

SECTIONS
{
  .flash_begin : {
    __flash_binary_start = .;
  } > FLASH

  .boot2 : {
    __boot2_start__ = .;
    KEEP (*(.boot2))
      __boot2_end__ = .;
  } > FLASH

  ASSERT(__boot2_end__ - __boot2_start__ == 256,
      "ERROR: Pico second stage bootloader must be 256 bytes in size")

  _flash_start = LOADADDR(.isr_vector);

  .isr_vector :
  {
    _isr_start = .;
    . = ALIGN(8);
    KEEP(*(.isr_vector))
    . = ALIGN(8);
    _isr_end = .;
  } >FLASH

  .shell_reg :
  {
    PROVIDE(cmd_list_start = .);
    . = ALIGN(4);
    KEEP(*(SORT(.shell_reg.*)))
    . = ALIGN(4);
    PROVIDE(cmd_list_end = .);
  } >FLASH

  .text :
  {
    . = ALIGN(8);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(8);
    _etext = .;
  } >FLASH

  .rodata :
  {
    . = ALIGN(8);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(8);
  } >FLASH

  .ARM.extab :
  {
    . = ALIGN(8);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(8);
  } >FLASH

  .ARM : {
    . = ALIGN(8);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(8);
  } >FLASH

  .init_array :
  {
    . = ALIGN(8);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(8);
  } >FLASH

  .fini_array :
  {
    . = ALIGN(8);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(8);
  } >FLASH

  _fsdata = LOADADDR(.data);

  .data :
  {
    PROVIDE ( _ram_start = . );
    . = ALIGN(8);
    _rsdata = .;
    *(.data)
    *(.data*)

    . = ALIGN(8);
    _redata = .;
  } >RAM AT> FLASH

  _flash_end = LOADADDR(.data) + SIZEOF(.data);

  . = ALIGN(4);
  .bss :
  {
    _sbss = .;
    __bss_start__ = .;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = .;
  } >RAM

  . = ALIGN(8);
  _end = .;
  PROVIDE(end = .);
  PROVIDE(__end__ = .);

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
