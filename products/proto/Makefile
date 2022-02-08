# Copyright (c) 2019-2021 Brian Thomas Murphy
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

ifeq ($(PROG),)

export BUILD_DIR = build
export ROOT ?= $(shell pwd)/../..

PROG += f030demo
PROG += f072n
PROG += f100d
PROG += f103deb
PROG += f103bp
PROG += f103n
PROG += f401bp
PROG += f401bp64
PROG += f411bp
PROG += f4d
PROG += f407deb
PROG += f407debm
PROG += f429
PROG += f469d
PROG += f746
PROG += f767
PROG += h723n
PROG += h735dk
PROG += h743wa
PROG += h743nucleo
PROG += h745nucleo
PROG += l432
PROG += l452
PROG += l496
PROG += l4r
PROG += g030deb
PROG += g0b1n
PROG += g474n
PROG += g431kbn
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

ifneq ($(TOOLCHAIN_ROOT),)
TOOLCHAIN_PFX = $(TOOLCHAIN_ROOT)/
endif

CONFIG_NEWLIB ?= n

CPP = cpp
XCC = $(TOOLCHAIN_PFX)arm-none-eabi-gcc
XOBJCOPY = $(TOOLCHAIN_PFX)arm-none-eabi-objcopy

BUILD_DIR ?= build

OBJDIR = $(BUILD_DIR)/obj-$(PROG)

all: $(BUILD_DIR)/$(PROG).bin

clean:
	rm -fr $(BUILD_DIR)

H7_BOARDS += h723n
H7_BOARDS += h735dk
H7_BOARDS += h743wa
H7_BOARDS += h743nucleo
H7_BOARDS += h745nucleo

ifneq (,$(findstring $(PROG),$(H7_BOARDS)))
CONFIG_STM32_H7 = 1
else
CONFIG_STM32_H7 = 0
endif

XCFLAGS.h735dk += -DSTM32_H7XX
XCFLAGS.h735dk += -DSTM32_H735DK
STACK_END.h735dk = 0x20020000
XCFLAGS.h735dk += -mcpu=cortex-m7
LWIP.h735dk = y

XCFLAGS.h743wa += -DSTM32_H7XX
XCFLAGS.h743wa += -DSTM32_H743WA
STACK_END.h743wa = 0x20020000
XCFLAGS.h743wa += -mcpu=cortex-m7

XCFLAGS.h743nucleo += -DSTM32_H7XX
STACK_END.h743nucleo = 0x20020000
XCFLAGS.h743nucleo += -mcpu=cortex-m7
LWIP.h743nucleo = y

XCFLAGS.h723n += -DSTM32_H7XX -DSTM32_H723N
STACK_END.h723n = 0x20020000
XCFLAGS.h723n += -mcpu=cortex-m7
LWIP.h723n = y

XCFLAGS.h745nucleo += -DSTM32_H7XX -DSTM32_H745N
STACK_END.h745nucleo = 0x20020000
XCFLAGS.h745nucleo += -mcpu=cortex-m7
LWIP.h745nucleo = y

XCFLAGS.l4r += -DSTM32_L4R
XCFLAGS.l4r += -mcpu=cortex-m4
STACK_END.l4r = 0x200A0000

XCFLAGS.f030demo += -DSTM32_F030 -DSTM32_F030DEMO -DSTM32_F0XX
XCFLAGS.f030demo += -DCONFIG_FAST_LOG_ENABLE=0
XCFLAGS.f030demo += -DCONFIG_N_INTS=32
XCFLAGS.f030demo += -DCONFIG_TIMER_16BIT=1
XCFLAGS.f030demo += -DCONFIG_BUTTON_INT=0
XCFLAGS.f030demo += -DCONFIG_SHELL_HIST=2
XCFLAGS.f030demo += -DCONFIG_SHELL_MAX_LINE_LEN=32
XCFLAGS.f030demo += -DCONFIG_SHELL_TAB_COMPLETE=0
XCFLAGS.f030demo += -DCONFIG_SHELL_HELP=0
XCFLAGS.f030demo += -DCONFIG_STM32_HAL_COMMANDS=0
XCFLAGS.f030demo += -DCONFIG_HAL_DMA_COMMANDS=1
XCFLAGS.f030demo += -DCONFIG_SIMPLE_EXCEPTION_HANDLER=0
XCFLAGS.f030demo += -DCONFIG_MALLOC_CHUNK_POW2=6
XCFLAGS.f030demo += -mcpu=cortex-m0
CONFIG_COPY_ISR.f030demo = 1
STACK_END.f030demo = 0x20001000
APPL_FLASH_BASE.f030demo = 0x8000000
OPTFLAGS.f030demo += -Os

XCFLAGS.f072n += -DSTM32_F072 -DSTM32_F072N -DSTM32_F0XX
XCFLAGS.f072n += -DCONFIG_FAST_LOG_SHIFT=4
XCFLAGS.f072n += -DCONFIG_N_INTS=32
XCFLAGS.f072n += -DCONFIG_TIMER_16BIT=1
XCFLAGS.f072n += -DCONFIG_BUTTON_INT=0
XCFLAGS.f072n += -mcpu=cortex-m0
CONFIG_COPY_ISR.f072n = 1
STACK_END.f072n = 0x20004000
APPL_FLASH_BASE.f072n = 0x8004000
OPTFLAGS.f072n += -Os

XCFLAGS.f100d += -DSTM32_F1XX
XCFLAGS.f100d += -DSTM32_F100D
XCFLAGS.f100d += -DCONFIG_SYSLOG_NUM_LINES=4
XCFLAGS.f100d += -DCONFIG_FAST_LOG_SHIFT=4
XCFLAGS.f100d += -DCONFIG_MALLOC_CHUNK_POW2=6
XCFLAGS.f100d += -DCONFIG_N_INTS=61
XCFLAGS.f100d += -DCONFIG_TIMER_16BIT=1
XCFLAGS.f100d += -mcpu=cortex-m3
STACK_END.f100d = 0x20002000

XCFLAGS.f103deb += -DSTM32_F1XX
XCFLAGS.f103deb += -DSTM32_F103DEB -DSTM32_F103
XCFLAGS.f103deb += -DCONFIG_FAST_LOG_ENABLE=0
XCFLAGS.f103deb += -DCONFIG_MALLOC_CHUNK_POW2=6
XCFLAGS.f103deb += -DCONFIG_N_INTS=68
XCFLAGS.f103deb += -DCONFIG_TIMER_16BIT=1
XCFLAGS.f103deb += -mcpu=cortex-m3
STACK_END.f103deb = 0x20002800
APPL_FLASH_BASE.f103deb = 0x8002800

XCFLAGS.f103bp += -DSTM32_F1XX
XCFLAGS.f103bp += -DSTM32_F103BP -DSTM32_F103
XCFLAGS.f103bp += -DCONFIG_SYSLOG_NUM_LINES=32
XCFLAGS.f103bp += -DCONFIG_FAST_LOG_SHIFT=4
XCFLAGS.f103bp += -DCONFIG_MALLOC_CHUNK_POW2=6
XCFLAGS.f103bp += -DCONFIG_N_INTS=68
XCFLAGS.f103bp += -DCONFIG_TIMER_16BIT=1
XCFLAGS.f103bp += -mcpu=cortex-m3
STACK_END.f103bp = 0x20005000

XCFLAGS.f103n += -DSTM32_F1XX
XCFLAGS.f103n += -DSTM32_F103N -DSTM32_F103
XCFLAGS.f103n += -DCONFIG_SYSLOG_NUM_LINES=32
XCFLAGS.f103n += -DCONFIG_FAST_LOG_SHIFT=4
XCFLAGS.f103n += -DCONFIG_MALLOC_CHUNK_POW2=6
XCFLAGS.f103n += -DCONFIG_N_INTS=68
XCFLAGS.f103n += -DCONFIG_TIMER_16BIT=1
XCFLAGS.f103n += -mcpu=cortex-m3
STACK_END.f103n = 0x20005000

XCFLAGS.f411bp += -DSTM32_F411 -DSTM32_F4XX
XCFLAGS.f411bp += -DSTM32_F411BP
XCFLAGS.f411bp += -mcpu=cortex-m4
STACK_END.f411bp = 0x20020000
#XCFLAGS.f411bp += -DWS2811
XCFLAGS.f411bp += -DI2C_DEMO
XCFLAGS.f411bp += -DDISP
XCFLAGS.f411bp += -DCFG_TUSB_MCU=OPT_MCU_STM32F4
XCFLAGS.f411bp += -DSTM32F411xE

STACK_END.f401bp = 0x20018000
XCFLAGS.f401bp += -DSTM32_F401 -DSTM32_F4XX
XCFLAGS.f401bp += -DSTM32_F401BP
XCFLAGS.f401bp += -mcpu=cortex-m4
XCFLAGS.f401bp += -DWS2811
XCFLAGS.f401bp += -DWS2811_DISPLAY_32X8_CLOCK
XCFLAGS.f401bp += -DWS2811_STRING_GRB
XCFLAGS.f401bp += -DCFG_TUSB_MCU=OPT_MCU_STM32F4
XCFLAGS.f401bp += -DSTM32F401xC


STACK_END.f401bp64 = 0x20010000
XCFLAGS.f401bp64 = $(XCFLAGS.f401bp)

XCFLAGS.f4d += -DSTM32_F407
XCFLAGS.f4d += -DSTM32_F4XX
XCFLAGS.f4d += -DSTM32_F4D
XCFLAGS.f4d += -mcpu=cortex-m4
STACK_END.f4d = 0x20020000
LWIP.f4d = y

XCFLAGS.f407deb += -DSTM32_F407
XCFLAGS.f407deb += -DSTM32_F4XX
XCFLAGS.f407deb += -DSTM32_F407DEB
XCFLAGS.f407deb += -mcpu=cortex-m4
STACK_END.f407deb = 0x20020000
LWIP.f407deb = y

XCFLAGS.f407debm += -DSTM32_F407
XCFLAGS.f407debm += -DSTM32_F4XX
XCFLAGS.f407debm += -DSTM32_F407DEBM
XCFLAGS.f407debm += -mcpu=cortex-m4
STACK_END.f407debm = 0x20020000

XCFLAGS.f429 += -DSTM32_F429
XCFLAGS.f429 += -DSTM32_F4XX
XCFLAGS.f429 += -DSTM32_LCD
XCFLAGS.f429 += -mcpu=cortex-m4
STACK_END.f429 = 0x20030000

XCFLAGS.f469d += -DSTM32_F469D -DSTM32_F4XX
XCFLAGS.f469d += -mcpu=cortex-m4
STACK_END.f469d = 0x20030000

XCFLAGS.f746 += -DSTM32_F746 -DSTM32_F7XX -DSTM32_LCD
XCFLAGS.f746 += -mcpu=cortex-m7
STACK_END.f746 = 0x20050000
LWIP.f746 = y

XCFLAGS.f767 += -DSTM32_F767 -DSTM32_F7XX
XCFLAGS.f767 += -mcpu=cortex-m7
STACK_END.f767 = 0x20080000
LWIP.f767 = y

XCFLAGS.l432 += -DSTM32_L432 -DSTM32_L4XX
XCFLAGS.l432 += -mcpu=cortex-m4
STACK_END.l432 = 0x20010000
XCFLAGS.l432 += -DWS2811

XCFLAGS.l452 += -DSTM32_L452 -DSTM32_L4XX
XCFLAGS.l452 += -mcpu=cortex-m4
STACK_END.l452 = 0x20028000

XCFLAGS.l496 += -DSTM32_L496 -DSTM32_L4XX
XCFLAGS.l496 += -mcpu=cortex-m4
STACK_END.l496 = 0x20040000

XCFLAGS.g0b1n += -DSTM32_G0B1 -DSTM32_G0B1N -DSTM32_G0XX
XCFLAGS.g0b1n += -DCONFIG_N_INTS=32
XCFLAGS.g0b1n += -DCONFIG_BUTTON_INT=0
XCFLAGS.g0b1n += -mcpu=cortex-m0plus
STACK_END.g0b1n = 0x20024000

XCFLAGS.g030deb += -DSTM32_G030 -DSTM32_G030DEB -DSTM32_G0XX
XCFLAGS.g030deb += -DCONFIG_N_INTS=32
XCFLAGS.g030deb += -DCONFIG_FAST_LOG_ENABLE=0
XCFLAGS.g030deb += -DCONFIG_TIMER_16BIT=1
XCFLAGS.g030deb += -DCONFIG_BUTTON_INT=1
XCFLAGS.g030deb += -DCONFIG_SHELL_HIST=2
XCFLAGS.g030deb += -DCONFIG_SHELL_MAX_LINE_LEN=32
XCFLAGS.g030deb += -DCONFIG_MALLOC_CHUNK_POW2=6
XCFLAGS.g030deb += -mcpu=cortex-m0plus
APPL_FLASH_BASE.g030deb = 0x8002800
STACK_END.g030deb = 0x20002000
OPTFLAGS.g030deb = -Os

XCFLAGS.g474n += -DSTM32_G4XX -DSTM32_G474N
XCFLAGS.g474n += -mcpu=cortex-m4
STACK_END.g474n = 0x20020000

XCFLAGS.g431kbn += -DSTM32_G4XX -DSTM32_G432KBN
XCFLAGS.g431kbn += -mcpu=cortex-m4
STACK_END.g431kbn = 0x20008000

XCFLAGS.u575n += -DSTM32_U575N -DSTM32_UXXX
XCFLAGS.u575n += -mcpu=cortex-m33
STACK_END.u575n = 0x200c0000

XCFLAGS.wb55n += -DSTM32_WBXX -DSTM32_WB55N
XCFLAGS.wb55n += -mcpu=cortex-m4
STACK_END.wb55n = 0x20030000

XCFLAGS.wb55usb += -DSTM32_WBXX -DSTM32_WB55USB
XCFLAGS.wb55usb += -mcpu=cortex-m4
STACK_END.wb55usb = 0x20030000

XCFLAGS += $(XCFLAGS.$(PROG))
STACK_END = $(STACK_END.$(PROG))

XCFLAGS += -DARCH_STM32
XCFLAGS += -mthumb
XCFLAGS += -Wall -Werror
XCFLAGS += -Wno-error=strict-overflow
XCFLAGS += -mno-unaligned-access
XCFLAGS += -ffunction-sections
# generate dependency information
XCFLAGS += -MD
XCFLAGS += -DAPPL
XCFLAGS += -DBMOS
XCFLAGS += -DCONFIG_POISON_STACK=1

ifeq ($(CONFIG_NEWLIB), y)
XCFLAGS += -DCONFIG_NEWLIB
XCFLAGS += -DPRIVATE_SBRK
endif

ifeq ($(CONFIG_NEWLIB), y)
XLDFLAGS += -lc -lnosys
XLDFLAGS += -specs=nano.specs
else
XLDFLAGS += -nostdlib
XLDFLAGS += -lgcc
endif
LD_SCRIPT = $(OBJDIR)/appl.ld
XLDFLAGS += -Wl,--defsym,_stack_end=$(STACK_END)
XLDFLAGS += -Wl,--gc-sections
XLDFLAGS += $(XCFLAGS)

MODULES += appl/prod/proto
MODULES += appl/shell
MODULES += appl/xmodem
MODULES += appl/xslog
MODULES += hal/board/stm32
MODULES += hal/core
MODULES += hal/cpu/arm
MODULES += hal/misc
MODULES += hal/stm32/core
MODULES += os/bmos
MODULES += std

MODULES += lib/graph/fb

MODULES += prot/usb/tusb/if
MODULES += prot/usb/tusb/tusb

ifneq ($(CONFIG_NEWLIB), y)
MODULES += lib/libc_min
endif

ifeq ($(LWIP.$(PROG)), y)
include Makefile.lwip
XCFLAGS += -DCONFIG_LWIP -DLWIP_DEBUG
FILES += lwip.o
FILES += $(FILES.lwip)
endif

XCFLAGS += $(addsuffix /inc, $(addprefix -I$(ROOT)/modules/, $(MODULES)))

# cmsis
XCFLAGS.cmsis += -I$(ROOT)/external/cmsis_5/CMSIS/Core/Include
XCFLAGS.cmsisf4 += -I$(ROOT)/external/st/cmsis_device_f4/Include
XCFLAGS.cmsish7 += -I$(ROOT)/external/st/cmsis_device_h7/Include

XCFLAGS.f401bp += $(XCFLAGS.cmsis) $(XCFLAGS.cmsisf4)
XCFLAGS.f401bp64 += $(XCFLAGS.cmsis) $(XCFLAGS.cmsisf4)
XCFLAGS.f411bp += $(XCFLAGS.cmsis) $(XCFLAGS.cmsisf4)

XCFLAGS.h743wa += $(XCFLAGS.cmsis) $(XCFLAGS.cmsish7)

VPATH += $(addsuffix /src, $(addprefix $(ROOT)/modules/, $(MODULES)))

FILES += main.o
FILES += start.o
FILES += stm32_hal_gpio.o
FILES += shell.o
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
FILES += stm32_timer.o
FILES += circ_buf.o
FILES += hal_dma.o
FILES += stm32_dma_if.o

# BMOS
FILES += mutex.o
FILES += op_msg.o
FILES += queue.o
FILES += reg.o
FILES += sem.o
FILES += task.o

ifneq ($(CONFIG_NEWLIB), y)
FILES += libc.o
FILES += malloc.o
endif

FILES.h7xx += stm32_usart_b.o
FILES.h7xx += stm32_rcc_c.o
FILES.h7xx += stm32_hal_rtc.o
FILES.h7xx += stm32_flash_h7xx.o
FILES.h7xx += stm32_wdog.o
FILES.h7xx += stm32_exti_h7xx.o
FILES.h7xx += stm32_pwr_h7xx.o
FILES.h7xx += stm32_eth_h7xx.o
FILES.h7xx += stm32_hal_bdma.o
FILES.h7xx += stm32_hal_dma.o
FILES.h7xx += stm32_hal_dmamux.o
FILES.h7xx += stm32_hal_i2c.o
FILES.h7xx += font1.o
FILES.h7xx += ssd1306_fonts.o
FILES.h7xx += fb.o
FILES.h7xx += i2c_test.o
FILES.h7xx += hd44780.o

FILES.h723n += stm32_board_h723n.o
FILES.h723n += stm32_fdcan.o
FILES.h723n += fdcan.o

FILES.h735dk += stm32_lcd.o
FILES.h735dk += stm32_board_h735dk.o
FILES.h735dk += stm32_fdcan.o
FILES.h735dk += fdcan.o

FILES.h743wa += stm32_board_h743wa.o
FILES.h743wa += stm32_hal_spi_b.o
FILES.h743wa += spi_test.o
FILES.h743wa += st7735.o

FILES.h743nucleo += stm32_board_h743nucleo.o

FILES.h745nucleo += stm32_board_h745nucleo.o
FILES.h745nucleo += stm32_fdcan.o
FILES.h745nucleo += fdcan.o

FILES.f7xx += stm32_pwr_f7xx.o
FILES.f7xx += stm32_flash.o
FILES.f4xx += stm32_pwr_f4xx.o
FILES.f4xx += stm32_flash.o
FILES.fxxx += stm32_exti_fxxx.o

FILES.f0xx += stm32_usart_b.o
FILES.f0xx += stm32_rcc_f1.o
FILES.f0xx += stm32_flash_f1.o
FILES.f0xx += stm32_hal_bdma.o
FILES.f0xx += stm32_pwr_f0xx.o

FILES.f030demo += stm32_board_f030demo.o
XSLOG.f030demo += xslog_simple.o
FILES.f030demo += $(FILES.f0xx)

FILES.f072n += stm32_board_f072n.o
XSLOG.f072n += xslog_simple.o
FILES.f072n += $(FILES.f0xx)

FILES.f1xx += stm32_exti_fxxx.o
FILES.f1xx += stm32_flash_f1.o
FILES.f1xx += stm32_hal_bdma.o
FILES.f1xx += stm32_pwr_f1xx.o
FILES.f1xx += stm32_rcc_f1.o
FILES.f1xx += stm32_usart_a.o

#FILES.f100d += stm32_hal_rtc.o # Needs new driver
FILES.f100d += stm32_board_f100d.o
XSLOG.f100d += xslog_simple.o
FILES.f100d += $(FILES.f1xx)

FILES.f103deb += stm32_board_f103deb.o
FILES.f103deb += $(FILES.f1xx)
XSLOG.f103deb = xslog_simple.o

FILES.f103bp += stm32_board_f103bp.o
FILES.f103bp += $(FILES.f1xx)

FILES.f103n += stm32_board_f103n.o
FILES.f103n += $(FILES.f1xx)

FILES.f411bp += stm32_hal_rtc.o
FILES.f411bp += stm32_usart_a.o
FILES.f411bp += stm32_rcc_a.o
FILES.f411bp += stm32_board_f411bp.o
FILES.f411bp += stm32_hal_dma.o
FILES.f411bp += stm32_hal_i2c_b.o
FILES.f411bp += font1.o
FILES.f411bp += fb.o
FILES.f411bp += ssd1306_fonts.o
FILES.f411bp += i2c_test.o
FILES.f411bp += ws2811.o

FILES.tusb += stm32_usbd.o
FILES.tusb += tusb_cdc.o
FILES.tusb += cdc_device.o
FILES.tusb += dcd_synopsys.o
FILES.tusb += tusb.o
FILES.tusb += tusb_fifo.o
FILES.tusb += usb_descriptors.o
FILES.tusb += usbd.o
FILES.tusb += usbd_control.o

FILES.f411bp += $(FILES.tusb)
FILES.f411bp += $(FILES.f4xx)
FILES.f411bp += $(FILES.fxxx)

FILES.f401bp += stm32_hal_rtc.o
FILES.f401bp += stm32_usart_a.o
FILES.f401bp += stm32_rcc_a.o
FILES.f401bp += stm32_board_f401bp.o
FILES.f401bp += stm32_hal_dma.o
FILES.f401bp += ws2811.o
FILES.f401bp += font1.o
FILES.f401bp += fb.o
FILES.f401bp += $(FILES.tusb)
FILES.f401bp += $(FILES.f4xx)
FILES.f401bp += $(FILES.fxxx)

FILES.f401bp64 += $(FILES.f401bp)

FILES.f4d += stm32_hal_rtc.o
FILES.f4d += stm32_usart_a.o
FILES.f4d += stm32_rcc_a.o
FILES.f4d += stm32_board_f4d.o
FILES.f4d += stm32_hal_dma.o
FILES.f4d += stm32_eth.o
FILES.f4d += $(FILES.f4xx)
FILES.f4d += $(FILES.fxxx)

FILES.f407deb += stm32_hal_rtc.o
FILES.f407deb += stm32_usart_a.o
FILES.f407deb += stm32_rcc_a.o
FILES.f407deb += stm32_board_f407deb.o
FILES.f407deb += stm32_hal_dma.o
FILES.f407deb += stm32_eth.o
FILES.f407deb += $(FILES.f4xx)
FILES.f407deb += $(FILES.fxxx)

FILES.f407debm += stm32_hal_rtc.o
FILES.f407debm += stm32_usart_a.o
FILES.f407debm += stm32_rcc_a.o
FILES.f407debm += stm32_board_f407debm.o
FILES.f407debm += stm32_hal_dma.o
FILES.f407debm += $(FILES.f4xx)
FILES.f407debm += $(FILES.fxxx)

FILES.f429 += stm32_usart_a.o
FILES.f429 += stm32_rcc_a.o
FILES.f429 += stm32_board_f429.o
FILES.f429 += stm32_hal_spi.o
FILES.f429 += stm32_hal_dma.o
FILES.f429 += stm32_lcd.o
FILES.f429 += $(FILES.f4xx)
FILES.f429 += $(FILES.fxxx)

FILES.f469d += stm32_usart_a.o
FILES.f469d += stm32_rcc_a.o
FILES.f469d += stm32_board_f469d.o
FILES.f469d += stm32_hal_dma.o
FILES.f469d += $(FILES.f4xx)
FILES.f469d += $(FILES.fxxx)

FILES.f746 += stm32_hal_dma.o
FILES.f746 += stm32_usart_b.o
FILES.f746 += stm32_rcc_a.o
FILES.f746 += stm32_hal_rtc.o
FILES.f746 += stm32_board_f746.o
FILES.f746 += stm32_eth.o
FILES.f746 += stm32_lcd.o
FILES.f746 += $(FILES.f7xx)
FILES.f746 += $(FILES.fxxx)

FILES.f767 += stm32_hal_dma.o
FILES.f767 += stm32_usart_b.o
FILES.f767 += stm32_rcc_a.o
FILES.f767 += stm32_hal_rtc.o
FILES.f767 += stm32_eth.o
FILES.f767 += stm32_board_f767.o
FILES.f767 += $(FILES.f7xx)
FILES.f767 += $(FILES.fxxx)

FILES.h723n += $(FILES.h7xx)
FILES.h735dk += $(FILES.h7xx)
FILES.h743wa += $(FILES.h7xx)
FILES.h743nucleo += $(FILES.h7xx)
FILES.h745nucleo += $(FILES.h7xx)

FILES.g0xx += stm32_flash.o
FILES.g0xx += stm32_hal_bdma.o
FILES.g0xx += stm32_hal_dmamux.o
FILES.g0xx += stm32_hal_rtc.o
FILES.g0xx += stm32_pwr_g0xx.o
FILES.g0xx += stm32_rcc_g0.o
FILES.g0xx += stm32_usart_b.o

FILES.g0b1n += $(FILES.g0xx)
FILES.g0b1n += stm32_board_g0b1n.o

FILES.g030deb += $(FILES.g0xx)
FILES.g030deb += stm32_exti_uxxx.o
FILES.g030deb += stm32_board_g030deb.o
XSLOG.g030deb += xslog_simple.o

FILES.g4xx += crc_ccitt16.o
FILES.g4xx += stm32_exti_lxxx.o
FILES.g4xx += stm32_flash.o
FILES.g4xx += stm32_hal_bdma.o
FILES.g4xx += stm32_hal_dmamux.o
FILES.g4xx += stm32_pwr_lxxx.o
FILES.g4xx += stm32_rcc_b.o
FILES.g4xx += stm32_usart_b.o
FILES.g4xx += stm32_wdog.o
FILES.g4xx += xmodem.o
FILES.g4xx += stm32_fdcan.o

FILES.wbxx += crc_ccitt16.o
FILES.wbxx += stm32_exti_lxxx.o
FILES.wbxx += stm32_flash.o
FILES.wbxx += stm32_hal_bdma.o
FILES.wbxx += stm32_pwr_lxxx.o
FILES.wbxx += stm32_rcc_b.o
FILES.wbxx += stm32_usart_b.o
FILES.wbxx += stm32_wdog.o
FILES.wbxx += xmodem.o

FILES.l4xx += stm32_usart_b.o
FILES.l4xx += stm32_rcc_b.o
FILES.l4xx += stm32_flash.o
FILES.l4xx += crc_ccitt16.o
FILES.l4xx += xmodem.o
FILES.l4xx += stm32_wdog.o
FILES.l4xx += stm32_pwr_lxxx.o
FILES.l4xx += stm32_exti_lxxx.o

FILES.l432 += stm32_hal_bdma.o
FILES.l432 += stm32_hal_rtc.o
FILES.l432 += stm32_hal_spi.o
FILES.l432 += stm32_board_l432.o
FILES.l432 += ws2811.o

FILES.l452 += stm32_hal_bdma.o
FILES.l452 += stm32_hal_rtc.o
FILES.l452 += stm32_board_l452.o

FILES.l496 += stm32_hal_bdma.o
FILES.l496 += stm32_hal_spi.o
FILES.l496 += stm32_hal_adc.o
FILES.l496 += stm32_board_l496.o

FILES.l432 += $(FILES.l4xx)
FILES.l452 += $(FILES.l4xx)
FILES.l496 += $(FILES.l4xx)

#FILES.l496 += ds1302.o
#FILES.l496 += ds1302_test.o
FILES.l496 += stm32_hal_rtc.o

FILES.l4r += stm32_hal_bdma.o
FILES.l4r += stm32_usart_b.o
FILES.l4r += stm32_rcc_b.o
FILES.l4r += stm32_board_l4r.o
FILES.l4r += stm32_wdog.o
FILES.l4r += stm32_pwr_lxxx.o
FILES.l4r += stm32_exti_lxxx.o
FILES.l4r += stm32_flash.o

FILES.g474n += $(FILES.g4xx)
FILES.g474n += stm32_hal_rtc.o
FILES.g474n += stm32_board_g474n.o
FILES.g474n += stm32_hal_i2c.o
FILES.g474n += font1.o
FILES.g474n += fb.o
FILES.g474n += ssd1306_fonts.o
FILES.g474n += i2c_test.o
FILES.g474n += fdcan.o

FILES.g431kbn += $(FILES.g4xx)
FILES.g431kbn += stm32_hal_rtc.o
FILES.g431kbn += stm32_board_g431kbn.o
FILES.g431kbn += fdcan.o

FILES.u575n += stm32_hal_rtc.o
FILES.u575n += stm32_exti_uxxx.o
FILES.u575n += stm32_pwr_uxxx.o
FILES.u575n += stm32_rcc_d.o
FILES.u575n += stm32_usart_b.o
FILES.u575n += stm32_hal_gpdma.o
FILES.u575n += stm32_fdcan.o
FILES.u575n += fdcan.o
FILES.u575n += stm32_board_u575n.o
FILES.u575n += stm32_flash.o

FILES.wb55n += $(FILES.wbxx)
FILES.wb55n += stm32_hal_rtc.o
FILES.wb55n += stm32_board_wb55n.o

FILES.wb55usb += $(FILES.wbxx)
FILES.wb55usb += stm32_hal_rtc.o
FILES.wb55usb += stm32_board_wb55usb.o
CPU.f100d = cortex-m3

ifdef APPL_FLASH_BASE.$(PROG)
APPL_FLASH_BASE = $(APPL_FLASH_BASE.$(PROG))
else
ifeq ($(CONFIG_STM32_H7), 1)
APPL_FLASH_BASE = 0x8020000
else
APPL_FLASH_BASE = 0x8008000
endif
endif

APPL_RAM_SIZE=256K
APPL_FLASH_SIZE=256K

XCFLAGS += -DAPPL_FLASH_BASE=$(APPL_FLASH_BASE)

ifdef XSLOG.$(PROG)
FILES += $(XSLOG.$(PROG))
else
FILES += xslog.o
endif

ifdef OPTFLAGS.$(PROG)
OPTFLAGS = $(OPTFLAGS.$(PROG))
else
OPTFLAGS = -O2
endif

XCFLAGS += $(OPTFLAGS)

ifdef CONFIG_COPY_ISR.$(PROG)
CONFIG_COPY_ISR = $(CONFIG_COPY_ISR.$(PROG))
else
CONFIG_COPY_ISR = 0
endif

XCFLAGS += -D CONFIG_COPY_ISR=$(CONFIG_COPY_ISR)

FILES += $(FILES.$(PROG))

DEPFILES = $(addprefix $(OBJDIR)/,$(patsubst %.o,%.d,$(FILES)))

# include dependency information
-include $(DEPFILES)

$(BUILD_DIR) $(OBJDIR):
	mkdir -p $@

OFILES = $(addprefix $(OBJDIR)/,$(FILES))
$(OFILES): | $(OBJDIR)

$(OBJDIR)/appl.ld: appl.ld.tem | $(OBJDIR)
	 $(CPP) -P -D __APPL_FLASH_SIZE=$(APPL_FLASH_SIZE) \
	        -D __APPL_RAM_SIZE=$(APPL_RAM_SIZE) \
	        -D CONFIG_COPY_ISR=$(CONFIG_COPY_ISR) \
	        -D __APPL_FLASH_BASE=$(APPL_FLASH_BASE) \
	        -D CONFIG_STM32_H7=$(CONFIG_STM32_H7) $< -o $@

$(BUILD_DIR)/$(PROG).elf: $(OFILES) $(LD_SCRIPT) |$(BUILD_DIR)
	 $(XCC) -o $@ $(OFILES) $(XLDFLAGS) -T $(LD_SCRIPT)

$(BUILD_DIR)/$(PROG).bin: $(BUILD_DIR)/$(PROG).elf |$(BUILD_DIR)
	 $(XOBJCOPY) -O binary -R .eth -R .framebuf $^ $@

$(OBJDIR)/%.o: %.S
	 $(XCC) -c $(XCFLAGS) -o $@ $<

$(OBJDIR)/%.o: %.c
	 $(XCC) -c $(XCFLAGS) -D__S_FILE__=\"$(notdir $<)\" -o $@ $<

endif # PROG
