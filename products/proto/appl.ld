ENTRY(reset_handler)

MEMORY
{
  RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 320K
  FLASH (rx)      : ORIGIN = 0x8008000, LENGTH = 256K
}

SECTIONS
{
  .isr_vector :
  {
    _flash_start = .;
    . = ALIGN(8);
    KEEP(*(.isr_vector))
    . = ALIGN(8);
  } >FLASH

  cmd_list_start = .;
  .shell_reg :
  {
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
