# bmos
An Operating System for ARM Cortex-M specifically running on a bunch of STM32 boards:
NUCLEO H743, F767, L496, L452, L432, L4R5 and the discovery boards F429 and F746.

There is also a limited HAL included: pinmux, uart drivers, flash support and ethernet.

The network stack LwIP is integrated and working on F746 and H743.

There is a simple bootloader with xmodem download over the serial port. The serial port
configured by default is the ST-Link virtual serial port.

A simple main with a blink application and a shell is also included.

To initialize external modules run:

    git submodule update --init

Ensure you have installed an arm-none-eabi toolchain and that it is in your PATH or set the path to the toolchain with

    TOOLCHAIN_ROOT=<path to toolchain> make

Run make at the top level to build everything. Results are available under each product in the build directory.
