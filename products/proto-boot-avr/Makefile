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

PROG += a128st
PROG += at90can128ol

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

XCC = $(TOOLCHAIN_PFX)avr-gcc
XOBJCOPY = $(TOOLCHAIN_PFX)avr-objcopy

BUILD_DIR ?= build

OBJDIR = $(BUILD_DIR)/obj-$(PROG)

all: $(BUILD_DIR)/$(PROG).hex

clean:
	rm -fr $(BUILD_DIR)

XCFLAGS.a128st += -DA128ST -DATMEGA128
#STACK_END.a128st = 0x20020000
XCFLAGS.a128st += -DCONFIG_FAST_LOG_SHIFT=4
XCFLAGS.a128st += -DCONFIG_SHELL_HELP=0
#XLDFLAGS += -nostdlib
#XLDFLAGS += -lgcc
CPU.a128st = -mmcu=atmega128

XCFLAGS.at90can128ol += -DAT90CAN128OL -DAT90CAN128
XCFLAGS.at90can128ol += -DCONFIG_FAST_LOG_SHIFT=4
XCFLAGS.at90can128ol += -DCONFIG_SHELL_HELP=0
CPU.at90can128ol = -mmcu=at90can128

#XLDFLAGS += -nostartfiles

XCFLAGS += $(XCFLAGS.$(PROG))
XCFLAGS += -DARCH_AVR
XCFLAGS += $(CPU.$(PROG))
XCFLAGS += -Wall -Werror
XCFLAGS += -Wno-misspelled-isr
XCFLAGS += -Os
#XCFLAGS += -O2
XCFLAGS += -ffunction-sections
# generate dependency information
XCFLAGS += -MD
XCFLAGS += -DBOOT

XLDFLAGS += -Tboot.ld
XLDFLAGS += -Wl,--gc-sections
XLDFLAGS += $(XCFLAGS)

MODULES += appl/prod/proto-boot-avr
MODULES += appl/shell
MODULES += appl/xmodem
MODULES += appl/xslog
MODULES += hal/board/avr
MODULES += hal/avr/core
MODULES += hal/core
MODULES += hal/misc
MODULES += std
MODULES += lib/libc_min

XCFLAGS += $(addsuffix /inc, $(addprefix -I$(ROOT)/modules/, $(MODULES)))

VPATH += $(addsuffix /src, $(addprefix $(ROOT)/modules/, $(MODULES)))

FILES += main.o
FILES += shell.o
FILES += mem.o
FILES += xtime.o
FILES += io.o
FILES += crc_ccitt16.o
FILES += xmodem.o
FILES += hd44780.o
FILES += xslog_simple.o
#FILES += libc.o
#FILES += start.o
FILES += hal_gpio.o
FILES += hal_avr_uart.o
FILES += hal_avr_i2c.o
FILES += hal_avr_gpio.o
FILES += util.o
#FILES += malloc.o

FILES.at90can128ol += hal_avr_can.o

FILES += $(FILES.$(PROG))

DEPFILES = $(addprefix $(OBJDIR)/,$(patsubst %.o,%.d,$(FILES)))

# include dependency information
-include $(DEPFILES)

$(BUILD_DIR) $(OBJDIR):
	mkdir -p $@

OFILES = $(addprefix $(OBJDIR)/,$(FILES))
$(OFILES): | $(OBJDIR)

$(BUILD_DIR)/$(PROG).elf: $(OFILES) |$(BUILD_DIR)
	 $(XCC) -Wl,-Map=$(BUILD_DIR)/$(PROG).map -o $@ $^ $(XLDFLAGS)

$(BUILD_DIR)/$(PROG).bin: $(BUILD_DIR)/$(PROG).elf |$(BUILD_DIR)
	 $(XOBJCOPY) -O binary $^ $@

$(BUILD_DIR)/$(PROG).hex: $(BUILD_DIR)/$(PROG).elf |$(BUILD_DIR)
	 $(XOBJCOPY) -O ihex $^ $@

$(OBJDIR)/%.o: %.S
	 $(XCC) -c $(XCFLAGS) -o $@ $<

$(OBJDIR)/%.o: %.c
	 $(XCC) -c $(XCFLAGS) -o $@ $<

endif # PROG
