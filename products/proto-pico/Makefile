ROOT ?= ../..
SDK_PATH = $(ROOT)/external/pico-sdk

ifneq ($(TOOLCHAIN_ROOT),)
TOOLCHAIN_PFX = $(TOOLCHAIN_ROOT)/
endif

XCC = $(TOOLCHAIN_PFX)arm-none-eabi-gcc
XLD = $(TOOLCHAIN_PFX)arm-none-eabi-gcc
XOBJCOPY = $(TOOLCHAIN_PFX)arm-none-eabi-objcopy

BUILD_DIR = build

RP2_COMMON_MODS += hardware_adc
RP2_COMMON_MODS += hardware_flash
RP2_COMMON_MODS += hardware_pll
RP2_COMMON_MODS += hardware_timer
RP2_COMMON_MODS += hardware_base
RP2_COMMON_MODS += hardware_gpio
RP2_COMMON_MODS += hardware_pwm
RP2_COMMON_MODS += hardware_uart
RP2_COMMON_MODS += hardware_claim
RP2_COMMON_MODS += hardware_i2c
RP2_COMMON_MODS += hardware_resets
RP2_COMMON_MODS += hardware_vreg
RP2_COMMON_MODS += hardware_divider
RP2_COMMON_MODS += hardware_clocks
RP2_COMMON_MODS += hardware_interp
RP2_COMMON_MODS += hardware_rtc
RP2_COMMON_MODS += hardware_watchdog
RP2_COMMON_MODS += hardware_irq
RP2_COMMON_MODS += hardware_spi
RP2_COMMON_MODS += hardware_xosc
RP2_COMMON_MODS += hardware_dma
RP2_COMMON_MODS += hardware_pio
RP2_COMMON_MODS += hardware_sync
RP2_COMMON_MODS += pico_bootrom
RP2_COMMON_MODS += pico_double
RP2_COMMON_MODS += pico_float
RP2_COMMON_MODS += pico_int64_ops
RP2_COMMON_MODS += pico_malloc
RP2_COMMON_MODS += pico_mem_ops
RP2_COMMON_MODS += pico_multicore
RP2_COMMON_MODS += pico_platform
RP2_COMMON_MODS += pico_printf
RP2_COMMON_MODS += pico_runtime
RP2_COMMON_MODS += pico_stdio
RP2_COMMON_MODS += pico_stdio_usb
RP2_COMMON_MODS += pico_unique_id

MODS_FULL += $(addprefix $(SDK_PATH)/src/rp2_common/, $(RP2_COMMON_MODS))

COMMON_MODS += boot_picoboot
COMMON_MODS += boot_uf2
COMMON_MODS += pico_base
COMMON_MODS += pico_binary_info
COMMON_MODS += pico_bit_ops
COMMON_MODS += pico_divider
COMMON_MODS += pico_stdlib
COMMON_MODS += pico_sync
COMMON_MODS += pico_time
COMMON_MODS += pico_util

MODS_FULL += $(addprefix $(SDK_PATH)/src/common/, $(COMMON_MODS))

RP2040_MODS += hardware_regs
RP2040_MODS += hardware_structs

MODS_FULL += $(addprefix $(SDK_PATH)/src/rp2040/, $(RP2040_MODS))

XCINC += $(addprefix -I, $(addsuffix /include, $(MODS_FULL)))

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

XLDFLAGS += -T memmap_default.ld
XLDFLAGS += -Wl,--gc-sections
XLDFLAGS += --specs=nosys.specs

PICO_FILES += common/pico_sync/critical_section.c
PICO_FILES += common/pico_sync/sem.c
PICO_FILES += common/pico_sync/lock_core.c
PICO_FILES += common/pico_sync/mutex.c
PICO_FILES += common/pico_time/timeout_helper.c
PICO_FILES += common/pico_time/time.c
PICO_FILES += common/pico_util/queue.c
PICO_FILES += common/pico_util/pheap.c
PICO_FILES += common/pico_util/datetime.c
PICO_FILES += rp2_common/hardware_adc/adc.c
PICO_FILES += rp2_common/hardware_claim/claim.c
PICO_FILES += rp2_common/hardware_clocks/clocks.c
PICO_FILES += rp2_common/hardware_dma/dma.c
PICO_FILES += rp2_common/hardware_flash/flash.c
PICO_FILES += rp2_common/hardware_gpio/gpio.c
PICO_FILES += rp2_common/hardware_i2c/i2c.c
PICO_FILES += rp2_common/hardware_interp/interp.c
PICO_FILES += rp2_common/hardware_pio/pio.c
PICO_FILES += rp2_common/hardware_pll/pll.c
PICO_FILES += rp2_common/hardware_rtc/rtc.c
PICO_FILES += rp2_common/hardware_spi/spi.c
PICO_FILES += rp2_common/hardware_sync/sync.c
PICO_FILES += rp2_common/hardware_timer/timer.c
PICO_FILES += rp2_common/hardware_uart/uart.c
PICO_FILES += rp2_common/hardware_vreg/vreg.c
PICO_FILES += rp2_common/hardware_watchdog/watchdog.c
PICO_FILES += rp2_common/hardware_xosc/xosc.c
PICO_FILES += rp2_common/pico_bootrom/bootrom.c
PICO_FILES += rp2_common/pico_double/double_init_rom.c
PICO_FILES += rp2_common/pico_double/double_math.c
PICO_FILES += rp2_common/pico_float/float_init_rom.c
PICO_FILES += rp2_common/pico_float/float_math.c
PICO_FILES += rp2_common/pico_malloc/pico_malloc.c
PICO_FILES += rp2_common/pico_mem_ops/mem_ops.c
PICO_FILES += rp2_common/pico_multicore/multicore.c
PICO_FILES += rp2_common/pico_platform/platform.c
PICO_FILES += rp2_common/pico_printf/printf.c
PICO_FILES += rp2_common/pico_runtime/runtime.c
PICO_FILES += rp2_common/pico_standard_link/binary_info.c
PICO_FILES += rp2_common/pico_stdio_usb/stdio_usb_descriptors.c
PICO_FILES += rp2_common/pico_stdlib/stdlib.c
PICO_FILES += rp2_common/pico_unique_id/unique_id.c

define PICO_RULE
$(BUILD_DIR)/$$(patsubst %.c,%.o,$$(subst /,~,$(1))): $(SDK_PATH)/src/$(1)
	$$(XCC) -c $$(XCINC) $$(XCFLAGS) -o $$@ $$^
endef

LOBJS += $(patsubst %.c,%.o,$(subst /,~,$(PICO_FILES)))

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

PICO_ASM += rp2_common/hardware_divider/divider.S
PICO_ASM += rp2_common/hardware_irq/irq_handler_chain.S
PICO_ASM += rp2_common/pico_bit_ops/bit_ops_aeabi.S
PICO_ASM += rp2_common/pico_divider/divider.S
PICO_ASM += rp2_common/pico_double/double_aeabi.S
PICO_ASM += rp2_common/pico_double/double_v1_rom_shim.S
PICO_ASM += rp2_common/pico_float/float_aeabi.S
PICO_ASM += rp2_common/pico_float/float_v1_rom_shim.S
PICO_ASM += rp2_common/pico_int64_ops/pico_int64_ops_aeabi.S
PICO_ASM += rp2_common/pico_mem_ops/mem_ops_aeabi.S

define PICO_ASM_RULE
$(BUILD_DIR)/$$(patsubst %.S,%.o,$$(subst /,~,$(1))): $(SDK_PATH)/src/$(1)
	$$(XCC) -c $$(XCINC) $$(XCFLAGS) -o $$@ $$^
endef

LOBJS += $(patsubst %.S,%.o,$(subst /,~,$(PICO_ASM)))

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
LOBJS += xtime.o

BUILD_LOBJS = $(addprefix $(BUILD_DIR)/,$(LOBJS))

PICO_UF2 ?= y
ifeq ($(PICO_UF2),y)
TARGET_UF2 = $(BUILD_DIR)/app.uf2
endif

all: $(BUILD_DIR)/app.elf $(TARGET_UF2)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(foreach file,$(PICO_FILES),$(eval $(call PICO_RULE,$(file))))
$(foreach file,$(PICO_ASM),$(eval $(call PICO_ASM_RULE,$(file))))

$(BUILD_DIR)/%.o: %.c
	$(XCC) -c $(XCINC) $(XCFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.S
	$(XCC) -c $(XCINC) $(XCFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.S
	$(XCC) -c $(XCINC) $(XCFLAGS) -o $@ $^

BOOT_STAGE2_PATH = $(SDK_PATH)/src/rp2_common/boot_stage2
BOOT_MODS_INC += -I$(BOOT_STAGE2_PATH)/asminclude

$(BUILD_DIR)/boot_stage2.elf: $(BOOT_STAGE2_PATH)/boot2_w25q080.S | $(BUILD_DIR)
	$(XCC) $(XCINC) $(BOOT_XCFLAGS) $(BOOT_MODS_INC) -O3 -T boot_stage2.ld --specs=nosys.specs -nostartfiles -o $@ $^

$(BUILD_DIR)/boot_stage2.raw: $(BUILD_DIR)/boot_stage2.elf
	$(XOBJCOPY) -O binary $< $@

$(BUILD_DIR)/boot_stage2.raw.S: $(BUILD_DIR)/boot_stage2.raw
	$(SDK_PATH)/src/rp2_common/boot_stage2/pad_checksum -s 0xffffffff $< $@

$(BUILD_LOBJS): | $(BUILD_DIR)
$(BUILD_APP_OBJS): | $(BUILD_DIR)

$(BUILD_DIR)/app.elf: $(BUILD_APP_OBJS) $(BUILD_LOBJS) $(BUILD_DIR)/boot_stage2.raw.o
	$(XLD) $(XCFLAGS) $(XLDFLAGS) -o $@ $^ -lm

ELF2UF2_PATH = $(SDK_PATH)/tools/elf2uf2
$(BUILD_DIR)/elf2uf2: $(ELF2UF2_PATH)/main.cpp
	g++ -o $@ -O2 -I $(ELF2UF2_PATH) -I $(SDK_PATH)/src/common/boot_uf2/include $(ELF2UF2_PATH)/main.cpp

$(BUILD_DIR)/%.uf2: $(BUILD_DIR)/%.elf $(BUILD_DIR)/elf2uf2
	$(BUILD_DIR)/elf2uf2 $< $@

clean:
	rm -fR $(BUILD_DIR)
