ROOT ?= /home/brian/src/bmos
TOOLCHAIN_ROOT = /usr/local/compilers/arm/gcc-arm-none-eabi-8-2018-q4-major/bin
SDK_PATH = $(ROOT)/external/pico-sdk

XCC = $(TOOLCHAIN_ROOT)/arm-none-eabi-gcc
XLD = $(TOOLCHAIN_ROOT)/arm-none-eabi-gcc
XOBJCOPY = $(TOOLCHAIN_ROOT)/arm-none-eabi-objcopy

BUILD_DIR = build

HW_MODS += adc flash pll timer base gpio pwm uart claim i2c resets vreg
#HW_MODS += divider
HW_MODS += clocks interp rtc watchdog irq spi xosc dma pio sync

PICO_MODS += bit_ops int64_ops standard_link bootrom malloc stdio
PICO_MODS += cxx_options mem_ops stdio_semihosting
PICO_MODS += divider multicore stdio_uart double platform stdio_usb
PICO_MODS += fix printf stdlib float runtime unique_id

COMMON_MODS += boot_picoboot pico_base pico_divider pico_time
COMMON_MODS += boot_uf2 pico_binary_info pico_stdlib pico_util
COMMON_MODS += pico_bit_ops pico_sync

RP2040_MODS += hardware_regs hardware_structs

XCFLAGS += -O3

XCFLAGS += -march=armv6-m -mcpu=cortex-m0plus -mthumb
XCFLAGS += -Wall -Werror -Wno-unused-function
XCFLAGS += -ffunction-sections
XCFLAGS += -fdata-sections
XCFLAGS += -DNDEBUG
XCFLAGS += -std=gnu11

XCFLAGS += -DPICO_BIT_OPS_PICO=1
XCFLAGS += -DPICO_COPY_TO_RAM=0
XCFLAGS += -DPICO_CXX_ENABLE_EXCEPTIONS=0
XCFLAGS += -DPICO_DIVIDER_HARDWARE=1
XCFLAGS += -DPICO_DOUBLE_PICO=1
XCFLAGS += -DPICO_FLOAT_PICO=1
XCFLAGS += -DPICO_INT64_OPS_PICO=1
XCFLAGS += -DPICO_MEM_OPS_PICO=1
XCFLAGS += -DPICO_NO_FLASH=0
XCFLAGS += -DPICO_NO_HARDWARE=0
XCFLAGS += -DPICO_ON_DEVICE=1
XCFLAGS += -DPICO_PRINTF_PICO=1
XCFLAGS += -DPICO_USE_BLOCKED_RAM=0
#XCFLAGS += -DPICO_STDIO_UART=1
#XCFLAGS += -DPICO_STDIO_USB=1

XLDFLAGS += -T memmap_default.ld
XLDFLAGS += -Wl,--gc-sections
XLDFLAGS += --specs=nosys.specs

BOOT_MODS_FULL += $(SDK_PATH)/src/rp2_common/boot_stage2
BOOT_MODS_INC += $(addprefix -I, $(addsuffix /asminclude, $(BOOT_MODS_FULL)))
#BOOT_MODS_INC += $(addprefix -I, $(addsuffix /include, $(BOOT_MODS_FULL)))
XCINC += $(BOOT_MODS_INC)
VPATH += $(BOOT_MODS_FULL)

HW_MODS_FULL += $(addprefix $(SDK_PATH)/src/rp2_common/hardware_, $(HW_MODS))
HW_MODS_INC += $(addprefix -I, $(addsuffix /include, $(HW_MODS_FULL)))
XCINC += $(HW_MODS_INC)
VPATH += $(HW_MODS_FULL)

PICO_MODS_FULL += $(addprefix $(SDK_PATH)/src/rp2_common/pico_, $(PICO_MODS))
PICO_MODS_INC += $(addprefix -I, $(addsuffix /include, $(PICO_MODS_FULL)))
XCINC += $(PICO_MODS_INC)
VPATH += $(PICO_MODS_FULL)

COMMON_MODS_FULL += $(addprefix $(SDK_PATH)/src/common/, $(COMMON_MODS))
COMMON_MODS_INC += $(addprefix -I, $(addsuffix /include, $(COMMON_MODS_FULL)))
XCINC += $(COMMON_MODS_INC)
#VPATH += $(COMMON_MODS_FULL)

RP2040_MODS_FULL += $(addprefix $(SDK_PATH)/src/rp2040/, $(RP2040_MODS))
RP2040_MODS_INC += $(addprefix -I, $(addsuffix /include, $(RP2040_MODS_FULL)))
XCINC += $(RP2040_MODS_INC)
VPATH += $(RP2040_MODS_FULL)

COMMON_FILES += common/pico_sync/critical_section.c
COMMON_FILES += common/pico_sync/sem.c
COMMON_FILES += common/pico_sync/lock_core.c
COMMON_FILES += common/pico_sync/mutex.c
COMMON_FILES += common/pico_time/timeout_helper.c
COMMON_FILES += common/pico_time/time.c
COMMON_FILES += common/pico_util/queue.c
COMMON_FILES += common/pico_util/pheap.c
COMMON_FILES += common/pico_util/datetime.c

define PICO_RULE
$(BUILD_DIR)/$$(patsubst %.c,%.o,$$(subst /,~,$(1))): $(SDK_PATH)/src/$(1)
	$$(XCC) -c $$(XCINC) $$(XCFLAGS) -o $$@ $$^
endef

COMMON_OBJS = $(patsubst %.c,%.o,$(subst /,~,$(COMMON_FILES)))
LOBJS += $(COMMON_OBJS)

TINYUSB_MODS_FULL += $(addprefix $(SDK_PATH)/src/rp2040/, $(RP2040_MODS))

TUSB_ROOT = $(ROOT)/external/tinyusb

XCINC += -I$(TUSB_ROOT)/src
XCINC += -I$(TUSB_ROOT)/src/common
XCINC += -I$(TUSB_ROOT)/hw

XCFLAGS += -DCFG_TUSB_MCU=OPT_MCU_RP2040
XCFLAGS += -DCFG_TUSB_DEBUG=0
XCFLAGS += -DCFG_TUSB_OS=OPT_OS_PICO

XCFLAGS += -DARCH_PICO=1
XCFLAGS += -DBMOS=1

XCINC += -I .

LOBJS += adc.o
LOBJS += binary_info.o
LOBJS += bootrom.o
LOBJS += claim.o
LOBJS += clocks.o
LOBJS += dma.o
LOBJS += double_init_rom.o
LOBJS += double_math.o
LOBJS += flash.o
LOBJS += float_init_rom.o
LOBJS += float_math.o
LOBJS += gpio.o
LOBJS += i2c.o
LOBJS += interp.o
#LOBJS += irq.o
LOBJS += mem_ops.o
LOBJS += multicore.o
LOBJS += pico_malloc.o
LOBJS += platform.o
LOBJS += pio.o
LOBJS += pll.o
LOBJS += printf.o
#LOBJS += rp2040_usb_device_enumeration.o
LOBJS += rtc.o
LOBJS += runtime.o
LOBJS += spi.o
#LOBJS += stdio.o
#LOBJS += stdio_semihosting.o
#LOBJS += stdio_uart.o
#LOBJS += stdio_usb.o
LOBJS += stdio_usb_descriptors.o
LOBJS += stdlib.o
LOBJS += sync.o
LOBJS += timer.o
LOBJS += uart.o
LOBJS += unique_id.o
LOBJS += vreg.o
LOBJS += watchdog.o
LOBJS += xosc.o

TUSB_BASE = $(TUSB_ROOT)/src

TUSB_MODS += $(TUSB_BASE)/portable/raspberrypi/rp2040

TUSB_OBJS += dcd_rp2040.o
TUSB_OBJS += rp2040_usb.o

TUSB_MODS += $(TUSB_BASE)/device

TUSB_OBJS += usbd.o
TUSB_OBJS += usbd_control.o

TUSB_CLASSES = audio cdc dfu hid midi msc net usbtmc vendor

TUSB_MODS += $(addprefix $(TUSB_BASE)/class/, $(TUSB_CLASSES))

TUSB_OBJS += audio_device.o
TUSB_OBJS += cdc_device.o
TUSB_OBJS += dfu_rt_device.o
TUSB_OBJS += hid_device.o
TUSB_OBJS += midi_device.o
TUSB_OBJS += msc_device.o
TUSB_OBJS += net_device.o
TUSB_OBJS += usbtmc_device.o
TUSB_OBJS += vendor_device.o

TUSB_MODS += $(TUSB_BASE)

TUSB_OBJS += tusb.o

TUSB_MODS += $(TUSB_BASE)/common

TUSB_OBJS += tusb_fifo.o

LOBJS += $(TUSB_OBJS)
VPATH += $(TUSB_MODS)

LOBJS += bmos_crt0.o
LOBJS += bit_ops_aeabi.o
LOBJS += divider.o
LOBJS += double_aeabi.o
LOBJS += double_v1_rom_shim.o
LOBJS += float_aeabi.o
LOBJS += float_v1_rom_shim.o
LOBJS += irq_handler_chain.o
LOBJS += mem_ops_aeabi.o
LOBJS += pico_int64_ops_aeabi.o

APP_OBJS += app.o
BUILD_APP_OBJS = $(addprefix $(BUILD_DIR)/,$(APP_OBJS))

MODULES += appl/prod/proto-pico
MODULES += appl/shell
MODULES += hal/core
MODULES += hal/cpu/arm
MODULES += std
MODULES += os/bmos

VPATH += $(addsuffix /src, $(addprefix $(ROOT)/modules/, $(MODULES)))
XCINC += $(addsuffix /inc, $(addprefix -I$(ROOT)/modules/, $(MODULES)))

LOBJS += cortexm.o
LOBJS += fast_log.o
LOBJS += hal_int.o
LOBJS += io.o
LOBJS += mem.o
LOBJS += reg.o
LOBJS += shell.o
LOBJS += task.o
LOBJS += queue.o
LOBJS += op_msg.o
LOBJS += misc.o
LOBJS += pico_irq.o
LOBJS += pico_uart.o

BUILD_LOBJS = $(addprefix $(BUILD_DIR)/,$(LOBJS))

all: app.elf app.uf2

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(foreach file,$(COMMON_FILES),$(eval $(call PICO_RULE,$(file))))

$(BUILD_DIR)/%.o: %.c
	$(XCC) -c $(XCINC) $(XCFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.S
	$(XCC) -c $(XCINC) $(XCFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.S
	$(XCC) -c $(XCINC) $(XCFLAGS) -o $@ $^

$(BUILD_DIR)/boot_stage2.elf: boot2_w25q080.S | $(BUILD_DIR)
	$(XCC) $(XCINC) $(BOOT_XCFLAGS) -O3 -T boot_stage2.ld --specs=nosys.specs -nostartfiles -o $@ $^

$(BUILD_DIR)/boot_stage2.raw: $(BUILD_DIR)/boot_stage2.elf
	$(XOBJCOPY) -O binary $< $@

$(BUILD_DIR)/boot_stage2.raw.S: $(BUILD_DIR)/boot_stage2.raw
	$(SDK_PATH)/src/rp2_common/boot_stage2/pad_checksum -s 0xffffffff $< $@

$(BUILD_LOBJS): | $(BUILD_DIR)
$(BUILD_APP_OBJS): | $(BUILD_DIR)

app.elf: $(BUILD_APP_OBJS) $(BUILD_LOBJS) $(BUILD_DIR)/boot_stage2.raw.o
	$(XLD) $(XCFLAGS) $(XLDFLAGS) -o $@ $^ -lm

ELF2UF2_PATH = $(SDK_PATH)/tools/elf2uf2
$(BUILD_DIR)/elf2uf2: $(ELF2UF2_PATH)/main.cpp
	g++ -o $@ -O2 -I $(ELF2UF2_PATH) -I $(SDK_PATH)/src/common/boot_uf2/include $(ELF2UF2_PATH)/main.cpp

%.uf2: %.elf $(BUILD_DIR)/elf2uf2
	$(BUILD_DIR)/elf2uf2 $< $@

clean:
	rm -f *.elf *.uf2
	rm -fR $(BUILD_DIR)
