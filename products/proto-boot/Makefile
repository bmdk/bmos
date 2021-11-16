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

PROG += f100d
PROG += f103bp
PROG += f411bp
PROG += f401bp
PROG += f407deb
PROG += f407debm
PROG += h723n
PROG += h743wa
PROG += h743nucleo
PROG += h745nucleo
PROG += h735dk
PROG += l4r
PROG += f767
PROG += f429
PROG += f469d
PROG += f746
PROG += l432
PROG += l452
PROG += l496
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

XCC = $(TOOLCHAIN_PFX)arm-none-eabi-gcc
XOBJCOPY = $(TOOLCHAIN_PFX)arm-none-eabi-objcopy

BUILD_DIR ?= build

OBJDIR = $(BUILD_DIR)/obj-$(PROG)

all: $(BUILD_DIR)/$(PROG).bin

clean:
	rm -fr $(BUILD_DIR)

XCFLAGS.h735dk += -DSTM32_H7XX
STACK_END.h735dk = 0x20020000

XCFLAGS.h723n += -DSTM32_H7XX
STACK_END.h723n = 0x20020000

XCFLAGS.h743nucleo += -DSTM32_H7XX
STACK_END.h743nucleo = 0x20020000

XCFLAGS.h743wa += -DSTM32_H7XX
STACK_END.h743wa = 0x20020000

XCFLAGS.h745nucleo += -DSTM32_H7XX
STACK_END.h745nucleo = 0x20020000

XCFLAGS.l4r += -DSTM32_L4R
STACK_END.l4r = 0x200A0000

XCFLAGS.f100d += -DSTM32_F100 -DSTM32_F100D -DSTM32_F1XX
XCFLAGS.f100d += -DCONFIG_FAST_LOG_SHIFT=4
STACK_END.f100d = 0x20002000
CPU.f100d = cortex-m3

XCFLAGS.f103bp += -DSTM32_F103 -DSTM32_F103BP -DSTM32_F1XX
STACK_END.f103bp = 0x20005000
CPU.f103bp = cortex-m3

XCFLAGS.f411bp += -DSTM32_F411 -DSTM32_F4XX
STACK_END.f411bp = 0x20020000

XCFLAGS.f401bp += -DSTM32_F401 -DSTM32_F4XX
STACK_END.f401bp = 0x20018000

XCFLAGS.f407deb += -DSTM32_F407 -DSTM32_F4XX
STACK_END.f407deb = 0x20020000

XCFLAGS.f407debm += -DSTM32_F407 -DSTM32_F4XX
STACK_END.f407debm = 0x20020000

XCFLAGS.f429 += -DSTM32_F429 -DSTM32_F4XX
XCFLAGS.f429 += -DSTM32_LCD
STACK_END.f429 = 0x20030000

XCFLAGS.f469d += -DSTM32_F469D -DSTM32_F4XX
STACK_END.f469d = 0x20030000

XCFLAGS.f746 += -DSTM32_F746 -DSTM32_F7XX
XCFLAGS.f746 += -DSTM32_LCD
STACK_END.f746 = 0x20050000

XCFLAGS.f767 += -DSTM32_F767 -DSTM32_F7XX
STACK_END.f767 = 0x20080000

XCFLAGS.l432 += -DSTM32_L432 -DSTM32_L4XX
STACK_END.l432 = 0x20010000

XCFLAGS.l452 += -DSTM32_L452 -DSTM32_L4XX
STACK_END.l452 = 0x20028000

XCFLAGS.l496 += -DSTM32_L496 -DSTM32_L4XX
STACK_END.l496 = 0x20040000

XCFLAGS.g474n += -DSTM32_G474N -DSTM32_G4XX
STACK_END.g474n = 0x20020000

XCFLAGS.g431kbn += -DSTM32_G431KBN -DSTM32_G4XX
STACK_END.g431kbn = 0x20008000

XCFLAGS.u575n += -DSTM32_U575N -DSTM32_UXXX
STACK_END.u575n = 0x200c0000
CPU.u575n = cortex-m33

XCFLAGS.wb55n += -DSTM32_WB55N -DSTM32_WBXX
STACK_END.wb55n = 0x20030000

XCFLAGS.wb55usb += -DSTM32_WB55USB -DSTM32_WBXX
STACK_END.wb55usb = 0x20030000

XCFLAGS += $(XCFLAGS.$(PROG))
STACK_END = $(STACK_END.$(PROG))

ifdef CPU.$(PROG)
CPU = $(CPU.$(PROG))
else
CPU = cortex-m4
endif

XCFLAGS += -DARCH_STM32
XCFLAGS += -mcpu=$(CPU)
XCFLAGS += -mthumb
XCFLAGS += -Wall -Werror
XCFLAGS += -Os
#XCFLAGS += -O2
XCFLAGS += -mno-unaligned-access
XCFLAGS += -ffunction-sections
# generate dependency information
XCFLAGS += -MD
XCFLAGS += -DBOOT

ifeq ($(CONFIG_NEWLIB), y)
XCFLAGS += -DCONFIG_NEWLIB
endif

ifeq ($(CONFIG_NEWLIB), y)
XLDFLAGS += -lc -lnosys
XLDFLAGS += -specs=nano.specs
else
XLDFLAGS += -nostdlib
endif
XLDFLAGS += -T boot.ld
XLDFLAGS += -Wl,--defsym,_stack_end=$(STACK_END)
XLDFLAGS += -Wl,--gc-sections
XLDFLAGS += $(XCFLAGS)

MODULES += appl/prod/proto-boot
MODULES += appl/shell
MODULES += appl/xmodem
MODULES += appl/xslog
MODULES += hal/board/stm32
MODULES += hal/core
MODULES += hal/cpu/arm
MODULES += hal/stm32/core
MODULES += std

ifneq ($(CONFIG_NEWLIB), y)
MODULES += lib/libc_min
endif

XCFLAGS += $(addsuffix /inc, $(addprefix -I$(ROOT)/modules/, $(MODULES)))

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
FILES += crc_ccitt16.o
FILES += xmodem.o
FILES += stm32_timer.o
FILES += xslog_simple.o

ifeq ($(findstring $(PROG), f100d f103bp h723n h743nucleo h745nucleo h735dk, h743wa),)
FILES += stm32_flash.o
endif

ifneq ($(CONFIG_NEWLIB), y)
FILES += libc.o
endif

FILES.h7xx += stm32_usart_b.o
FILES.h7xx += stm32_rcc_c.o
FILES.h7xx += stm32_hal_rtc.o
FILES.h7xx += stm32_flash_h7xx.o
FILES.h7xx += stm32_pwr_h7xx.o

FILES.h723n += stm32_board_h723n.o
FILES.h723n += $(FILES.h7xx)

FILES.h743wa += stm32_board_h743wa.o
FILES.h743wa += $(FILES.h7xx)

FILES.h743nucleo += stm32_board_h743nucleo.o
FILES.h743nucleo += $(FILES.h7xx)

FILES.h745nucleo += stm32_board_h745nucleo.o
FILES.h745nucleo += $(FILES.h7xx)

FILES.h735dk += stm32_board_h735dk.o
FILES.h735dk += $(FILES.h7xx)

FILES.f7xx += stm32_pwr_f7xx.o

FILES.f4xx += stm32_pwr_f4xx.o

FILES.fxxx += stm32_exti_fxxx.o

FILES.f100d += stm32_usart_a.o
FILES.f100d += stm32_rcc_f1.o
FILES.f100d += stm32_flash_f1.o
FILES.f100d += stm32_board_f100d.o

FILES.f103bp += stm32_usart_a.o
FILES.f103bp += stm32_rcc_f1.o
FILES.f103bp += stm32_flash_f1.o
FILES.f103bp += stm32_board_f103bp.o

FILES.f411bp += stm32_usart_a.o
FILES.f411bp += stm32_rcc_a.o
FILES.f411bp += stm32_board_f411bp.o
FILES.f411bp += $(FILES.f4xx)
FILES.f411bp += $(FILES.fxxx)

FILES.f401bp += stm32_usart_a.o
FILES.f401bp += stm32_rcc_a.o
FILES.f401bp += stm32_board_f401bp.o
FILES.f401bp += $(FILES.f4xx)
FILES.f401bp += $(FILES.fxxx)

FILES.f407deb += stm32_usart_a.o
FILES.f407deb += stm32_rcc_a.o
FILES.f407deb += stm32_board_f407deb.o
FILES.f407deb += $(FILES.f4xx)
FILES.f407deb += $(FILES.fxxx)

FILES.f407debm += stm32_usart_a.o
FILES.f407debm += stm32_rcc_a.o
FILES.f407debm += stm32_board_f407debm.o
FILES.f407debm += $(FILES.f4xx)
FILES.f407debm += $(FILES.fxxx)

FILES.f429 += stm32_usart_a.o
FILES.f429 += stm32_rcc_a.o
FILES.f429 += stm32_board_f429.o
FILES.f429 += stm32_hal_spi.o
FILES.f429 += $(FILES.f4xx)
FILES.f429 += $(FILES.fxxx)

FILES.f469d += stm32_usart_a.o
FILES.f469d += stm32_rcc_a.o
FILES.f469d += stm32_board_f469d.o
FILES.f469d += $(FILES.f4xx)
FILES.f469d += $(FILES.fxxx)

FILES.f746 += stm32_usart_b.o
FILES.f746 += stm32_rcc_a.o
FILES.f746 += stm32_hal_rtc.o
FILES.f746 += stm32_board_f746.o
FILES.f746 += $(FILES.f7xx)
FILES.f746 += $(FILES.fxxx)

FILES.f767 += stm32_usart_b.o
FILES.f767 += stm32_rcc_a.o
FILES.f767 += stm32_hal_rtc.o
FILES.f767 += stm32_board_f767.o
FILES.f767 += $(FILES.f7xx)
FILES.f767 += $(FILES.fxxx)

FILES.l4xx += stm32_usart_b.o
FILES.l4xx += stm32_rcc_b.o
FILES.l4xx += stm32_hal_rtc.o
FILES.l4xx += stm32_pwr_lxxx.o
FILES.l4xx += stm32_exti_lxxx.o

FILES.l432 += $(FILES.l4xx)
FILES.l432 += stm32_board_l432.o
FILES.l432 += stm32_hal_spi.o

FILES.l452 += $(FILES.l4xx)
FILES.l452 += stm32_board_l452.o

FILES.l496 += $(FILES.l4xx)
FILES.l496 += stm32_board_l496.o
FILES.l496 += stm32_hal_spi.o

FILES.l4r += stm32_usart_b.o
FILES.l4r += stm32_rcc_b.o
FILES.l4r += stm32_board_l4r.o
FILES.l4r += stm32_pwr_lxxx.o

FILES.uxxx += stm32_usart_b.o
FILES.uxxx += stm32_rcc_d.o
FILES.uxxx += stm32_exti_uxxx.o
FILES.uxxx += stm32_pwr_uxxx.o
FILES.uxxx += stm32_hal_rtc.o
#FILES.uxxx += stm32_pwr_lxxx.o

FILES.u575n += $(FILES.uxxx)
FILES.u575n += stm32_board_u575n.o

FILES.wbxx += stm32_usart_b.o
FILES.wbxx += stm32_rcc_b.o
FILES.wbxx += stm32_hal_rtc.o
FILES.wbxx += stm32_pwr_lxxx.o
FILES.wbxx += stm32_exti_lxxx.o

FILES.wb55n += $(FILES.wbxx)
FILES.wb55n += stm32_board_wb55n.o

FILES.wb55usb += $(FILES.wbxx)
FILES.wb55usb += stm32_board_wb55usb.o

FILES.g4xx += stm32_usart_b.o
FILES.g4xx += stm32_rcc_b.o
FILES.g4xx += stm32_hal_rtc.o
FILES.g4xx += stm32_pwr_lxxx.o
FILES.g4xx += stm32_exti_lxxx.o

FILES.g474n += $(FILES.g4xx)
FILES.g474n += stm32_board_g474n.o

FILES.g431kbn += $(FILES.g4xx)
FILES.g431kbn += stm32_board_g431kbn.o

FILES += $(FILES.$(PROG))

DEPFILES = $(addprefix $(OBJDIR)/,$(patsubst %.o,%.d,$(FILES)))

# include dependency information
-include $(DEPFILES)

$(BUILD_DIR) $(OBJDIR):
	mkdir -p $@

OFILES = $(addprefix $(OBJDIR)/,$(FILES))
$(OFILES): | $(OBJDIR)

$(BUILD_DIR)/$(PROG).elf: $(OFILES) |$(BUILD_DIR)
	 $(XCC) -o $@ $^ $(XLDFLAGS)

$(BUILD_DIR)/$(PROG).bin: $(BUILD_DIR)/$(PROG).elf |$(BUILD_DIR)
	 $(XOBJCOPY) -O binary $^ $@

$(OBJDIR)/%.o: %.S
	 $(XCC) -c $(XCFLAGS) -o $@ $<

$(OBJDIR)/%.o: %.c
	 $(XCC) -c $(XCFLAGS) -o $@ $<

endif # PROG
