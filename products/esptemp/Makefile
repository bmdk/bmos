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
ROOT ?= $(shell pwd)/../..

ifeq ($(PROG),)

PROG += f401bp
PROG += f401bp64
PROG += f411bp

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

XCFLAGS.f411bp += -DSTM32_F411 -DSTM32_F4XX
XCFLAGS.f411bp += -DSTM32_F411BP
XCFLAGS.f411bp += -DONE_WIRE=1
XCFLAGS.f411bp += -DCONFIG_ESP01_UART=1
XCFLAGS.f411bp += -mcpu=cortex-m4
STACK_END.f411bp = 0x20020000
APPL_FLASH_BASE.f411bp = 0x8010000
#XCFLAGS.f411bp += -DWS2811
XCFLAGS.f411bp += -DCONFIG_ENABLE_ADC
XCFLAGS.f411bp += -DDISP
XCFLAGS.f411bp += -DCFG_TUSB_MCU=OPT_MCU_STM32F4
XCFLAGS.f411bp += -DSTM32F411xE
XCFLAGS.f411bp += -DCONFIG_KVLOG_ENABLE=1

STACK_END.f401bp = 0x20018000
APPL_FLASH_BASE.f401bp = 0x8010000
XCFLAGS.f401bp += -DSTM32_F401 -DSTM32_F4XX
XCFLAGS.f401bp += -DSTM32_F401BP
XCFLAGS.f401bp += -mcpu=cortex-m4
XCFLAGS.f401bp += -DCONFIG_ENABLE_ADC
XCFLAGS.f401bp += -DONE_WIRE=1
XCFLAGS.f401bp += -DCONFIG_ESP01_UART=1
#XCFLAGS.f401bp += -DWS2811
#XCFLAGS.f401bp += -DWS2811_DISPLAY_32X8_CLOCK
#XCFLAGS.f401bp += -DWS2811_STRING_GRB
XCFLAGS.f401bp += -DCFG_TUSB_MCU=OPT_MCU_STM32F4
XCFLAGS.f401bp += -DSTM32F401xC
XCFLAGS.f401bp += -DCONFIG_KVLOG_ENABLE=1


STACK_END.f401bp64 = 0x20010000
APPL_FLASH_BASE.f401bp64 = 0x8010000
XCFLAGS.f401bp64 = $(XCFLAGS.f401bp)

XCFLAGS += $(XCFLAGS.$(PROG))
STACK_END = $(STACK_END.$(PROG))

XCFLAGS += -DARCH_STM32
XCFLAGS += -mthumb
XCFLAGS += -Wall -Werror
XCFLAGS += -Wno-error=strict-overflow
XCFLAGS += -mno-unaligned-access
XCFLAGS += -ffreestanding
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
MODULES += lib/kvlog
MODULES += os/bmos
MODULES += std

MODULES += lib/graph/fb

MODULES += prot/usb/tusb/if
MODULES += prot/usb/tusb/tusb
MODULES += lib/libc_min

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
FILES += stm32_rcc_ls.o
FILES += xlib.o

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

FILES.f4xx += stm32_pwr_f4xx.o
FILES.f4xx += stm32_flash.o
FILES.fxxx += stm32_exti_fxxx.o

FILES.f411bp += stm32_hal_rtc.o
FILES.f411bp += stm32_usart_a.o
FILES.f411bp += stm32_rcc_a.o
FILES.f411bp += onewire.o
FILES.f411bp += esp_at.o
FILES.f411bp += stm32_board_f411bp.o
FILES.f411bp += stm32_hal_dma.o
FILES.f411bp += stm32_hal_i2c_b.o
FILES.f411bp += font1.o
FILES.f411bp += fb.o
FILES.f411bp += ssd1306_fonts.o
FILES.f411bp += i2c_test.o
FILES.f411bp += ws2811.o
FILES.f411bp += stm32_hal_adc_fxxx.o
FILES.f411bp += adc.o
FILES.f411bp += kvlog.o

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
#FILES.f401bp += ws2811.o
FILES.f401bp += font1.o
FILES.f401bp += fb.o
FILES.f401bp += stm32_hal_adc_fxxx.o
FILES.f401bp += adc.o
FILES.f401bp += kvlog.o
FILES.f401bp += onewire.o
FILES.f401bp += esp_at.o
FILES.f401bp += $(FILES.tusb)
FILES.f401bp += $(FILES.f4xx)
FILES.f401bp += $(FILES.fxxx)

FILES.f401bp64 += $(FILES.f401bp)

ifdef APPL_FLASH_BASE.$(PROG)
APPL_FLASH_BASE = $(APPL_FLASH_BASE.$(PROG))
else
APPL_FLASH_BASE = 0x8008000
endif

ifdef APPL_RAM_BASE.$(PROG)
APPL_RAM_BASE = $(APPL_RAM_BASE.$(PROG))
else
APPL_RAM_BASE = 0x20000000
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
	        -D __APPL_RAM_BASE=$(APPL_RAM_BASE) \
	        -D __APPL_RAM_SIZE=$(APPL_RAM_SIZE) \
	        -D CONFIG_COPY_ISR=$(CONFIG_COPY_ISR) \
	        -D __APPL_FLASH_BASE=$(APPL_FLASH_BASE) \
	        $< -o $@

$(BUILD_DIR)/$(PROG).elf: $(OFILES) $(LD_SCRIPT) |$(BUILD_DIR)
	 $(XCC) -o $@ $(OFILES) $(XLDFLAGS) -T $(LD_SCRIPT)

$(BUILD_DIR)/$(PROG).bin: $(BUILD_DIR)/$(PROG).elf |$(BUILD_DIR)
	 $(XOBJCOPY) -O binary -R .eth -R .framebuf $^ $@

$(OBJDIR)/%.o: %.S
	 $(XCC) -c $(XCFLAGS) -o $@ $<

$(OBJDIR)/%.o: %.c
	 $(XCC) -c $(XCFLAGS) -D__S_FILE__=\"$(notdir $<)\" -o $@ $<

endif # PROG
