# Copyright (c) 2019-2022 Brian Thomas Murphy
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

BUILD_DIR = build

ifeq ($(PROG),)

PROG += at32f403bp
PROG += c031n
PROG += f030demo
PROG += f072n
PROG += f100d
PROG += f103bp
PROG += f103deb
PROG += f103n
PROG += f303rt
PROG += f401bp
PROG += f407deb
PROG += f407debm
PROG += f411bp
PROG += f429d
PROG += f469d
PROG += f4d
PROG += f746d
PROG += f767n
PROG += g030deb
PROG += g0b1n
PROG += g431kbn
PROG += g474n
PROG += h563n
PROG += h723n
PROG += h735dk
PROG += h743n
PROG += h743wa
PROG += h745n
PROG += l073n
PROG += l432n
PROG += l452n
PROG += l452np
PROG += l496n
PROG += l4rn
PROG += u083n
PROG += u575n
PROG += wb55n
PROG += wb55usb

PROG_TRG = $(addprefix board-, $(PROG))

all: $(PROG_TRG)

board-%:
	$(MAKE) PROG=$*

clean:
	rm -fr $(BUILD_DIR)

else

OPTFLAGS_DEFAULT = -Os

XCFLAGS.h563n += -DSTM32_H5XX
STACK_END.h563n = 0x200a0000
CPU.h563n = cortex-m33

XCFLAGS.h735dk += -DSTM32_H7XX
STACK_END.h735dk = 0x20020000

XCFLAGS.h723n += -DSTM32_H7XX
STACK_END.h723n = 0x20020000

XCFLAGS.h743n += -DSTM32_H7XX -DSTM32_VOS_HACK
STACK_END.h743n = 0x20020000

XCFLAGS.h743wa += -DSTM32_H7XX -DSTM32_VOS_HACK
STACK_END.h743wa = 0x20020000

XCFLAGS.h745n += -DSTM32_H7XX -DSTM32_VOS_HACK
STACK_END.h745n = 0x20020000

XCFLAGS.l073n += -DSTM32_L0XX -DSTM32_L073N
XCFLAGS.l073n += -DCONFIG_TIMER_16BIT=1
STACK_END.l073n = 0x20005000
CPU.l073n = cortex-m0plus

XCFLAGS.l4rn += -DSTM32_L4R
STACK_END.l4rn = 0x200A0000

XCFLAGS.c031n += -DSTM32_C031 -DSTM32_C0XX
XCFLAGS.c031n += -DCONFIG_FAST_LOG_ENABLE=0
XCFLAGS.c031n += -DCONFIG_TIMER_16BIT=1
XCFLAGS.c031n += -DCONFIG_N_INTS=0
XCFLAGS.c031n += -DCONFIG_SHELL_HIST=2
XCFLAGS.c031n += -DCONFIG_SHELL_MAX_LINE_LEN=32
XCFLAGS.c031n += -DCONFIG_SHELL_TAB_COMPLETE=0
XCFLAGS.c031n += -DCONFIG_SHELL_HELP=0
XCFLAGS.c031n += -DCONFIG_STM32_HAL_COMMANDS=0
XCFLAGS.c031n += -DCONFIG_SIMPLE_EXCEPTION_HANDLER=1
XCFLAGS.c031n += -DCONFIG_XMODEM_CRC_BITWISE=1
XCFLAGS.c031n += -DCONFIG_FLASH_NO_LOCK=1
XCFLAGS.c031n += -DCONFIG_NO_EXCEPTIONS=1
XCFLAGS.c031n += -DCONFIG_XPANIC_SMALL=1
XCFLAGS.c031n += -DCONFIG_SMALL=1
CONFIG_XMODEM_SMALL.c031n = y
CONFIG_SHELL.c031n = 0
CONFIG_RAMSIZE.c031n = 4
CONFIG_FLASHSIZE.c031n = 2
STACK_END.c031n = 0x20003000
CPU.c031n = cortex-m0plus

XCFLAGS.f030demo += -DSTM32_F030 -DSTM32_F0XX
XCFLAGS.f030demo += -DCONFIG_FAST_LOG_ENABLE=0
XCFLAGS.f030demo += -DCONFIG_TIMER_16BIT=1
XCFLAGS.f030demo += -DCONFIG_N_INTS=0
XCFLAGS.f030demo += -DCONFIG_SHELL_HIST=2
XCFLAGS.f030demo += -DCONFIG_SHELL_MAX_LINE_LEN=32
XCFLAGS.f030demo += -DCONFIG_SHELL_TAB_COMPLETE=0
XCFLAGS.f030demo += -DCONFIG_SHELL_HELP=0
XCFLAGS.f030demo += -DCONFIG_STM32_HAL_COMMANDS=0
XCFLAGS.f030demo += -DCONFIG_SIMPLE_EXCEPTION_HANDLER=1
XCFLAGS.f030demo += -DCONFIG_BUS_ENABLE_INLINE=1
XCFLAGS.f030demo += -DCONFIG_SIMPLE_EXCEPTION_HANDLER=1
XCFLAGS.f030demo += -DCONFIG_XMODEM_CRC_BITWISE=1
XCFLAGS.f030demo += -DCONFIG_FLASH_NO_LOCK=1
XCFLAGS.f030demo += -DCONFIG_NO_EXCEPTIONS=1
XCFLAGS.f030demo += -DCONFIG_XPANIC_SMALL=1
XCFLAGS.f030demo += -DCONFIG_SMALL=1
CONFIG_SHELL.f030demo = 0
CONFIG_RAMSIZE.f030demo = 4
CONFIG_FLASHSIZE.f030demo = 1
CONFIG_XMODEM_SMALL.f030demo = y
STACK_END.f030demo = 0x20001000
CPU.f030demo = cortex-m0

XCFLAGS.f072n += -DSTM32_F072 -DSTM32_F0XX
XCFLAGS.f072n += -DCONFIG_FAST_LOG_ENABLE=0
XCFLAGS.f072n += -DCONFIG_TIMER_16BIT=1
XCFLAGS.f072n += -DCONFIG_N_INTS=32
XCFLAGS.f072n += -DCONFIG_SHELL_HIST=2
XCFLAGS.f072n += -DCONFIG_SHELL_MAX_LINE_LEN=32
XCFLAGS.f072n += -DCONFIG_SHELL_TAB_COMPLETE=0
XCFLAGS.f072n += -DCONFIG_SHELL_HELP=0
XCFLAGS.f072n += -DCONFIG_STM32_HAL_COMMANDS=0
#XCFLAGS.f072n += -DCONFIG_SIMPLE_EXCEPTION_HANDLER=1
STACK_END.f072n = 0x20004000
CPU.f072n = cortex-m0

XCFLAGS.f100d += -DSTM32_F100 -DSTM32_F1XX
XCFLAGS.f100d += -DCONFIG_FAST_LOG_SHIFT=4
STACK_END.f100d = 0x20002000
CPU.f100d = cortex-m3

XCFLAGS.f103deb += -DSTM32_F103 -DSTM32_F1XX
XCFLAGS.f103deb += -DCONFIG_FAST_LOG_ENABLE=0
XCFLAGS.f103deb += -DCONFIG_N_INTS=31
XCFLAGS.f103deb += -DCONFIG_SHELL_HIST=2
XCFLAGS.f103deb += -DCONFIG_SHELL_MAX_LINE_LEN=32
XCFLAGS.f103deb += -DCONFIG_SHELL_TAB_COMPLETE=0
XCFLAGS.f103deb += -DCONFIG_SHELL_HELP=0
XCFLAGS.f103deb += -DCONFIG_STM32_HAL_COMMANDS=0
XCFLAGS.f103deb += -DCONFIG_SIMPLE_EXCEPTION_HANDLER=0
STACK_END.f103deb = 0x20002800
CPU.f103deb = cortex-m3

XCFLAGS.f103bp += -DSTM32_F103 -DSTM32_F1XX
STACK_END.f103bp = 0x20005000
CPU.f103bp = cortex-m3


XCFLAGS.f103n += -DSTM32_F103 -DSTM32_F1XX
STACK_END.f103n = 0x20005000
CPU.f103n = cortex-m3

XCFLAGS.at32f403bp += -DAT32_F403 -DAT32_F403BP -DAT32_F4XX
STACK_END.at32f403bp = 0x20018000

XCFLAGS.f303rt += -DSTM32_F303 -DSTM32_F3XX
STACK_END.f303rt = 0x2000A000

XCFLAGS.f411bp += -DSTM32_F411 -DSTM32_F4XX
STACK_END.f411bp = 0x20020000

XCFLAGS.f401bp += -DSTM32_F401 -DSTM32_F4XX
STACK_END.f401bp = 0x20010000

XCFLAGS.f4d += -DSTM32_F407 -DSTM32_F4XX
STACK_END.f4d = 0x20020000

XCFLAGS.f407deb += -DSTM32_F407 -DSTM32_F4XX
STACK_END.f407deb = 0x20020000

XCFLAGS.f407debm += -DSTM32_F407 -DSTM32_F4XX
STACK_END.f407debm = 0x20020000

XCFLAGS.f429d += -DSTM32_F429 -DSTM32_F4XX
XCFLAGS.f429d += -DSTM32_LCD
STACK_END.f429d = 0x20030000

XCFLAGS.f469d += -DSTM32_F4XX
STACK_END.f469d = 0x20030000

XCFLAGS.f746d += -DSTM32_F746 -DSTM32_F7XX
XCFLAGS.f746d += -DSTM32_LCD
STACK_END.f746d = 0x20050000

XCFLAGS.f767n += -DSTM32_F767 -DSTM32_F7XX
STACK_END.f767n = 0x20080000

XCFLAGS.l432n += -DSTM32_L432 -DSTM32_L4XX
STACK_END.l432n = 0x20010000

XCFLAGS.l452n += -DSTM32_L452 -DSTM32_L4XX
STACK_END.l452n = 0x20028000

XCFLAGS.l452np += -DSTM32_L452 -DSTM32_L4XX
STACK_END.l452np = 0x20028000

XCFLAGS.l496n += -DSTM32_L496 -DSTM32_L4XX
STACK_END.l496n = 0x20040000

XCFLAGS.g0b1n += -DSTM32_G0B1 -DSTM32_G0XX
XCFLAGS.g0b1n += -DCONFIG_FAST_LOG_ENABLE=0
XCFLAGS.g0b1n += -DCONFIG_TIMER_16BIT=1
XCFLAGS.g0b1n += -DCONFIG_N_INTS=0
XCFLAGS.g0b1n += -DCONFIG_SIMPLE_EXCEPTION_HANDLER=1
XCFLAGS.g0b1n += -DCONFIG_XMODEM_CRC_BITWISE=1
XCFLAGS.g0b1n += -DCONFIG_FLASH_NO_LOCK=1
XCFLAGS.g0b1n += -DCONFIG_NO_EXCEPTIONS=1
XCFLAGS.g0b1n += -DCONFIG_XPANIC_SMALL=1
XCFLAGS.g0b1n += -DCONFIG_SMALL=1
CONFIG_XMODEM_SMALL.g0b1n = y
CONFIG_SHELL.g0b1n = 0
STACK_END.g0b1n = 0x20024000
CPU.g0b1n = cortex-m0plus

XCFLAGS.g030deb += -DSTM32_G030 -DSTM32_G0XX
XCFLAGS.g030deb += -DCONFIG_FAST_LOG_ENABLE=0
XCFLAGS.g030deb += -DCONFIG_TIMER_16BIT=1
XCFLAGS.g030deb += -DCONFIG_N_INTS=0
XCFLAGS.g030deb += -DCONFIG_SIMPLE_EXCEPTION_HANDLER=1
XCFLAGS.g030deb += -DCONFIG_XMODEM_CRC_BITWISE=1
XCFLAGS.g030deb += -DCONFIG_FLASH_NO_LOCK=1
XCFLAGS.g030deb += -DCONFIG_NO_EXCEPTIONS=1
XCFLAGS.g030deb += -DCONFIG_XPANIC_SMALL=1
XCFLAGS.g030deb += -DCONFIG_SMALL=1
CONFIG_XMODEM_SMALL.g030deb = y
CONFIG_SHELL.g030deb = 0
STACK_END.g030deb = 0x20002000
CPU.g030deb = cortex-m0plus

XCFLAGS.g474n += -DSTM32_G4XX
STACK_END.g474n = 0x20020000

XCFLAGS.g431kbn += -DSTM32_G4XX
STACK_END.g431kbn = 0x20008000

XCFLAGS.u083n += -DSTM32_U0XX
STACK_END.u083n = 0x2000a000
CPU.u083n = cortex-m0plus

XCFLAGS.u575n += -DSTM32_U5XX
STACK_END.u575n = 0x200c0000
CPU.u575n = cortex-m33

XCFLAGS.wb55n += -DSTM32_WBXX
STACK_END.wb55n = 0x20030000

XCFLAGS.wb55usb += -DSTM32_WBXX
STACK_END.wb55usb = 0x20030000

XCFLAGS += -DARCH_STM32
XCFLAGS += -DBOOT

LD_SCRIPT = $(OBJDIR)/boot.ld

MODULES += appl/prod/proto-boot
MODULES += appl/shell
MODULES += appl/xmodem
MODULES += appl/xslog
MODULES += hal/board/stm32
MODULES += hal/core
MODULES += hal/cpu/arm
MODULES += hal/stm32/core
MODULES += std
MODULES += lib/libc_min

FILES += start.o
FILES += stm32_hal_gpio.o
FILES += cortexm.o
FILES += xtime.o
FILES += hal_common.o
FILES += hal_gpio.o
FILES += hal_int.o
FILES += io.o
FILES += mem.o
FILES += misc.o
FILES += fast_log.o
FILES += stm32_hal.o
FILES += crc_ccitt16.o
FILES += stm32_timer.o
FILES += xslog_simple.o
FILES += xlib.o

ifneq ($(CONFIG_NEWLIB), y)
FILES += libc.o
endif

FILES.c0xx += stm32_rcc_g0.o
FILES.c0xx += stm32_usart_b.o
FILES.c0xx += stm32_flash.o
FILES.c0xx += stm32_pwr_g0xx.o

FILES.c031n += $(FILES.c0xx)
MAIN.c031n = main_small.o

FILES.h7xx += stm32_usart_b.o
FILES.h7xx += stm32_rcc_h7.o
FILES.h7xx += stm32_hal_rtc.o
FILES.h7xx += stm32_flash_h7xx.o
FILES.h7xx += stm32_pwr_h7xx.o

FILES.h723n += $(FILES.h7xx)

FILES.h743wa += $(FILES.h7xx)

FILES.h743n += $(FILES.h7xx)

FILES.h745n += stm32_wwdg.o
FILES.h745n += $(FILES.h7xx)

FILES.h735dk += $(FILES.h7xx)

FILES.f7xx += stm32_pwr_f7xx.o
FILES.f7xx += stm32_flash.o

FILES.f4xx += stm32_pwr_f4xx.o
FILES.f4xx += stm32_flash.o

FILES.fxxx += stm32_exti_fxxx.o

FILES.f0xx += stm32_usart_b.o
FILES.f0xx += stm32_rcc_f1.o
FILES.f0xx += stm32_flash_f1.o

FILES.f030demo += $(FILES.f0xx)
MAIN.f030demo = main_small.o

FILES.f072n += $(FILES.f0xx)

FILES.f100d += stm32_usart_a.o
FILES.f100d += stm32_rcc_f1.o
FILES.f100d += stm32_flash_f1.o

FILES.f103deb += stm32_usart_a.o
FILES.f103deb += stm32_rcc_f1.o
FILES.f103deb += stm32_flash_f1.o

FILES.f103bp += stm32_usart_a.o
FILES.f103bp += stm32_rcc_f1.o
FILES.f103bp += stm32_flash_f1.o

FILES.f103n += stm32_usart_a.o
FILES.f103n += stm32_rcc_f1.o
FILES.f103n += stm32_flash_f1.o

FILES.at32f403bp += stm32_usart_a.o
FILES.at32f403bp += stm32_rcc_f1.o
FILES.at32f403bp += stm32_flash_f1.o
BOARD.at32f403bp += at32_board_f403bp.o

FILES.f303rt += stm32_usart_b.o
FILES.f303rt += stm32_rcc_f1.o
FILES.f303rt += stm32_flash_f1.o

FILES.f411bp += stm32_usart_a.o
FILES.f411bp += stm32_rcc_a.o
FILES.f411bp += $(FILES.f4xx)
FILES.f411bp += $(FILES.fxxx)

FILES.f401bp += stm32_usart_a.o
FILES.f401bp += stm32_rcc_a.o
FILES.f401bp += $(FILES.f4xx)
FILES.f401bp += $(FILES.fxxx)

FILES.f4d += stm32_usart_a.o
FILES.f4d += stm32_rcc_a.o
FILES.f4d += $(FILES.f4xx)
FILES.f4d += $(FILES.fxxx)

FILES.f407deb += stm32_usart_a.o
FILES.f407deb += stm32_rcc_a.o
FILES.f407deb += $(FILES.f4xx)
FILES.f407deb += $(FILES.fxxx)

FILES.f407debm += stm32_usart_a.o
FILES.f407debm += stm32_rcc_a.o
FILES.f407debm += $(FILES.f4xx)
FILES.f407debm += $(FILES.fxxx)

FILES.f429d += stm32_usart_a.o
FILES.f429d += stm32_rcc_a.o
FILES.f429d += stm32_hal_spi.o
FILES.f429d += $(FILES.f4xx)
FILES.f429d += $(FILES.fxxx)

FILES.f469d += stm32_usart_a.o
FILES.f469d += stm32_rcc_a.o
FILES.f469d += $(FILES.f4xx)
FILES.f469d += $(FILES.fxxx)

FILES.f746d += stm32_usart_b.o
FILES.f746d += stm32_rcc_a.o
FILES.f746d += stm32_hal_rtc.o
FILES.f746d += $(FILES.f7xx)
FILES.f746d += $(FILES.fxxx)

FILES.f767n += stm32_usart_b.o
FILES.f767n += stm32_rcc_a.o
FILES.f767n += stm32_hal_rtc.o
FILES.f767n += $(FILES.f7xx)
FILES.f767n += $(FILES.fxxx)

FILES.l0xx += stm32_usart_b.o
FILES.l0xx += stm32_rcc_l0.o
FILES.l0xx += stm32_flash_l0.o
FILES.l0xx += stm32_pwr_l0xx.o

FILES.l073n += $(FILES.l0xx)

FILES.l4xx += stm32_usart_b.o
FILES.l4xx += stm32_rcc_b.o
FILES.l4xx += stm32_hal_rtc.o
FILES.l4xx += stm32_pwr_lxxx.o
FILES.l4xx += stm32_exti_lxxx.o
FILES.l4xx += stm32_flash.o

FILES.l432n += $(FILES.l4xx)
FILES.l432n += stm32_hal_spi.o

FILES.l452n += $(FILES.l4xx)
FILES.l452np += $(FILES.l4xx)

FILES.l496n += $(FILES.l4xx)
FILES.l496n += stm32_hal_spi.o

FILES.l4rn += stm32_usart_b.o
FILES.l4rn += stm32_rcc_b.o
FILES.l4rn += stm32_pwr_lxxx.o
FILES.l4rn += stm32_flash.o

FILES.h5xx += stm32_usart_b.o
FILES.h5xx += stm32_rcc_h5.o
#FILES.h5xx += stm32_exti_h5xx.o
FILES.h5xx += stm32_pwr_h5xx.o
FILES.h5xx += stm32_flash.o

FILES.h563n += $(FILES.h5xx)

FILES.u0xx += stm32_usart_b.o
FILES.u0xx += stm32_rcc_g0.o
#FILES.u0xx += stm32_exti_uxxx.o
#FILES.u0xx += stm32_pwr_uxxx.o
#FILES.u0xx += stm32_hal_rtc.o
FILES.u0xx += stm32_flash.o

FILES.u083n += $(FILES.u0xx)

FILES.u5xx += stm32_usart_b.o
FILES.u5xx += stm32_rcc_u5.o
FILES.u5xx += stm32_exti_uxxx.o
FILES.u5xx += stm32_pwr_uxxx.o
FILES.u5xx += stm32_hal_rtc.o
FILES.u5xx += stm32_flash.o
#FILES.u5xx += stm32_pwr_lxxx.o

FILES.u575n += $(FILES.u5xx)

FILES.wbxx += stm32_usart_b.o
FILES.wbxx += stm32_rcc_b.o
FILES.wbxx += stm32_hal_rtc.o
FILES.wbxx += stm32_pwr_lxxx.o
FILES.wbxx += stm32_exti_lxxx.o
FILES.wbxx += stm32_flash.o

FILES.wb55n += $(FILES.wbxx)

FILES.wb55usb += $(FILES.wbxx)

FILES.g0xx += stm32_rcc_g0.o
FILES.g0xx += stm32_usart_b.o
FILES.g0xx += stm32_flash.o
FILES.g0xx += stm32_pwr_g0xx.o

FILES.g0b1n += $(FILES.g0xx)
MAIN.g0b1n += main_small.o

FILES.g030deb += $(FILES.g0xx)
MAIN.g030deb += main_small.o

FILES.g4xx += stm32_usart_b.o
FILES.g4xx += stm32_rcc_b.o
FILES.g4xx += stm32_hal_rtc.o
FILES.g4xx += stm32_pwr_lxxx.o
FILES.g4xx += stm32_exti_lxxx.o
FILES.g4xx += stm32_flash.o

FILES.g474n += $(FILES.g4xx)

FILES.g431kbn += $(FILES.g4xx)

include ../Makefile.rules

endif # PROG
