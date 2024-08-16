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
PROG += f401bp64
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
PROG += h745nm4
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

H7_BOARDS += h723n
H7_BOARDS += h735dk
H7_BOARDS += h743wa
H7_BOARDS += h743n
H7_BOARDS += h745n

ifneq (,$(findstring $(PROG),$(H7_BOARDS)))
CONFIG_STM32_H7 = 1
else
CONFIG_STM32_H7 = 0
endif

XCFLAGS.h735dk += -DSTM32_H7XX
STACK_END.h735dk = 0x20020000
CPU.h735dk = cortex-m7
LWIP.h735dk = y

XCFLAGS.h743wa += -DSTM32_H7XX
XCFLAGS.h743wa += -DSTM32_VOS_HACK
STACK_END.h743wa = 0x20020000
CPU.h743wa = cortex-m7

XCFLAGS.h743n += -DSTM32_H7XX
XCFLAGS.h743n += -DSTM32_VOS_HACK
STACK_END.h743n = 0x20020000
CPU.h743n = cortex-m7
LWIP.h743n = y

XCFLAGS.h723n += -DSTM32_H7XX
STACK_END.h723n = 0x20020000
CPU.h723n = cortex-m7
LWIP.h723n = y

XCFLAGS.h745n += -DSTM32_H7XX
XCFLAGS.h745n += -DSTM32_VOS_HACK
STACK_END.h745n = 0x20020000
CPU.h745n = cortex-m7
LWIP.h745n = y

XCFLAGS.h745nm4 += -DSTM32_H7XX
XCFLAGS.h745nm4 += -DCONFIG_BUTTON_INT=0
APPL_RAM_BASE.h745nm4 = 0x30000000
STACK_END.h745nm4 = 0x30020000
APPL_FLASH_BASE.h745nm4 = 0x8100000
CPU.h745nm4 = cortex-m4

XCFLAGS.h563n += -DSTM32_H5XX
#XCFLAGS.h563n += -DCONFIG_BUTTON_INT=0
XCFLAGS.h563n += -DCONFIG_CAN_TEST
STACK_END.h563n = 0x200a0000
CPU.h563n = cortex-m33
LWIP.h563n = y
APPL_FLASH_BASE.h563n = 0x8020000

XCFLAGS.l073n += -DSTM32_L0XX
XCFLAGS.l073n += -DCONFIG_TIMER_16BIT=1
XCFLAGS.l073n += -DCONFIG_BUTTON_INT=0
XCFLAGS.l073n += -DCONFIG_N_INTS=32
CPU.l073n = cortex-m0plus
STACK_END.l073n = 0x20005000
APPL_FLASH_BASE.l073n = 0x8004000

XCFLAGS.l4rn += -DSTM32_L4R
STACK_END.l4rn = 0x200A0000

XCFLAGS.f030demo += -DSTM32_F030 -DSTM32_F0XX
XCFLAGS.f030demo += -DCONFIG_FAST_LOG_ENABLE=0
XCFLAGS.f030demo += -DCONFIG_N_INTS=32
XCFLAGS.f030demo += -DCONFIG_TIMER_16BIT=1
XCFLAGS.f030demo += -DCONFIG_BUTTON_INT=0
XCFLAGS.f030demo += -DCONFIG_SHELL_HIST=2
XCFLAGS.f030demo += -DCONFIG_SHELL_MAX_LINE_LEN=32
XCFLAGS.f030demo += -DCONFIG_SHELL_TAB_COMPLETE=1
XCFLAGS.f030demo += -DCONFIG_SHELL_HELP=0
XCFLAGS.f030demo += -DCONFIG_STM32_HAL_COMMANDS=0
XCFLAGS.f030demo += -DCONFIG_HAL_DMA_COMMANDS=0
XCFLAGS.f030demo += -DCONFIG_SIMPLE_EXCEPTION_HANDLER=1
XCFLAGS.f030demo += -DCONFIG_MALLOC_CHUNK_POW2=6
CPU.f030demo = cortex-m0
CONFIG_COPY_ISR.f030demo = 1
STACK_END.f030demo = 0x20001000
APPL_FLASH_BASE.f030demo = 0x8000400
OPTFLAGS.f030demo += -Os

XCFLAGS.f072n += -DSTM32_F072 -DSTM32_F0XX
XCFLAGS.f072n += -DCONFIG_FAST_LOG_SHIFT=4
XCFLAGS.f072n += -DCONFIG_N_INTS=32
XCFLAGS.f072n += -DCONFIG_BUTTON_INT=0
CPU.f072n = cortex-m0
CONFIG_COPY_ISR.f072n = 1
STACK_END.f072n = 0x20004000
APPL_FLASH_BASE.f072n = 0x8004000
OPTFLAGS.f072n += -Os

XCFLAGS.f100d += -DSTM32_F1XX
XCFLAGS.f100d += -DCONFIG_SYSLOG_NUM_LINES=4
XCFLAGS.f100d += -DCONFIG_FAST_LOG_SHIFT=4
XCFLAGS.f100d += -DCONFIG_MALLOC_CHUNK_POW2=6
XCFLAGS.f100d += -DCONFIG_N_INTS=61
XCFLAGS.f100d += -DCONFIG_TIMER_16BIT=1
CPU.f100d = cortex-m3
STACK_END.f100d = 0x20002000

XCFLAGS.f103deb += -DSTM32_F1XX
XCFLAGS.f103deb += -DSTM32_F103
XCFLAGS.f103deb += -DCONFIG_FAST_LOG_ENABLE=0
XCFLAGS.f103deb += -DCONFIG_MALLOC_CHUNK_POW2=6
XCFLAGS.f103deb += -DCONFIG_N_INTS=68
XCFLAGS.f103deb += -DCONFIG_TIMER_16BIT=1
CPU.f103deb = cortex-m3
STACK_END.f103deb = 0x20002800
APPL_FLASH_BASE.f103deb = 0x8002800

XCFLAGS.f103bp += -DSTM32_F1XX
XCFLAGS.f103bp += -DSTM32_F103
XCFLAGS.f103bp += -DCONFIG_SYSLOG_NUM_LINES=32
XCFLAGS.f103bp += -DCONFIG_FAST_LOG_SHIFT=4
XCFLAGS.f103bp += -DCONFIG_MALLOC_CHUNK_POW2=6
XCFLAGS.f103bp += -DCONFIG_N_INTS=68
XCFLAGS.f103bp += -DCONFIG_TIMER_16BIT=1
XCFLAGS.f103bp += -DCONFIG_ENABLE_ADC=1
CPU.f103bp = cortex-m3
STACK_END.f103bp = 0x20005000

XCFLAGS.f103n += -DSTM32_F1XX
XCFLAGS.f103n += -DSTM32_F103
XCFLAGS.f103n += -DCONFIG_SYSLOG_NUM_LINES=32
XCFLAGS.f103n += -DCONFIG_FAST_LOG_SHIFT=4
XCFLAGS.f103n += -DCONFIG_MALLOC_CHUNK_POW2=6
XCFLAGS.f103n += -DCONFIG_N_INTS=68
XCFLAGS.f103n += -DCONFIG_TIMER_16BIT=1
XCFLAGS.f103n += -DCONFIG_ENABLE_ADC=1
CPU.f103n = cortex-m3
STACK_END.f103n = 0x20005000

XCFLAGS.at32f403bp += -DAT32_F4XX
XCFLAGS.at32f403bp += -DAT32_F403BP -DAT32_F403
# no ethernet on at32f403
XCFLAGS.at32f403bp += -DCONFIG_N_INTS=79
XCFLAGS.at32f403bp += -DCONFIG_ENABLE_ADC=1
STACK_END.at32f403bp = 0x20018000

XCFLAGS.f303rt += -DSTM32_F3XX
XCFLAGS.f303rt += -DSTM32_F303
XCFLAGS.f303rt += -DCONFIG_SYSLOG_NUM_LINES=32
XCFLAGS.f303rt += -DCONFIG_FAST_LOG_SHIFT=4
XCFLAGS.f303rt += -DCONFIG_MALLOC_CHUNK_POW2=6
XCFLAGS.f303rt += -DCONFIG_N_INTS=60
XCFLAGS.f303rt += -DCONFIG_BUTTON_INT=0
XCFLAGS.f303rt += -DWDOG_KICK
APPL_FLASH_BASE.f303rt = 0x8004000
STACK_END.f303rt = 0x2000a000

XCFLAGS.f411bp += -DSTM32_F411 -DSTM32_F4XX
STACK_END.f411bp = 0x20020000
APPL_FLASH_BASE.f411bp = 0x8010000
#XCFLAGS.f411bp += -DWS2811
XCFLAGS.f411bp += -DCONFIG_ENABLE_ADC_DMA
XCFLAGS.f411bp += -DDISP
XCFLAGS.f411bp += -DCFG_TUSB_MCU=OPT_MCU_STM32F4
XCFLAGS.f411bp += -DSTM32F411xE
XCFLAGS.f411bp += -DCONFIG_KVLOG_ENABLE=1

STACK_END.f401bp = 0x20018000
APPL_FLASH_BASE.f401bp = 0x8010000
XCFLAGS.f401bp += -DSTM32_F401 -DSTM32_F4XX
XCFLAGS.f401bp += -DCONFIG_ENABLE_ADC
XCFLAGS.f401bp += -DWS2811
XCFLAGS.f401bp += -DWS2811_DISPLAY_32X8_CLOCK
XCFLAGS.f401bp += -DWS2811_STRING_GRB
XCFLAGS.f401bp += -DCFG_TUSB_MCU=OPT_MCU_STM32F4
XCFLAGS.f401bp += -DSTM32F401xC
XCFLAGS.f401bp += -DCONFIG_KVLOG_ENABLE=1


STACK_END.f401bp64 = 0x20010000
APPL_FLASH_BASE.f401bp64 = 0x8010000
XCFLAGS.f401bp64 = $(XCFLAGS.f401bp)

XCFLAGS.f4d += -DSTM32_F407
XCFLAGS.f4d += -DSTM32_F4XX
XCFLAGS.f4d += -DCONFIG_ETH_PHY_ADDR=1
STACK_END.f4d = 0x20020000
LWIP.f4d = y

XCFLAGS.f407deb += -DSTM32_F407
XCFLAGS.f407deb += -DSTM32_F4XX
STACK_END.f407deb = 0x20020000
LWIP.f407deb = y

XCFLAGS.f407debm += -DSTM32_F407
XCFLAGS.f407debm += -DSTM32_F4XX
STACK_END.f407debm = 0x20020000

XCFLAGS.f429d += -DSTM32_F429
XCFLAGS.f429d += -DSTM32_F4XX
XCFLAGS.f429d += -DSTM32_LCD
STACK_END.f429d = 0x20030000

XCFLAGS.f469d += -DSTM32_F469 -DSTM32_F4XX
STACK_END.f469d = 0x20030000

XCFLAGS.f746d += -DSTM32_F746 -DSTM32_F7XX -DSTM32_LCD
CPU.f746d = cortex-m7
STACK_END.f746d = 0x20050000
LWIP.f746d = y

XCFLAGS.f767n += -DSTM32_F767 -DSTM32_F7XX
CPU.f767n = cortex-m7
STACK_END.f767n = 0x20080000
LWIP.f767n = y

XCFLAGS.l432n += -DSTM32_L432 -DSTM32_L4XX
STACK_END.l432n = 0x20010000
XCFLAGS.l432n += -DWS2811
XCFLAGS.l432n += -DCONFIG_ENABLE_ADC

XCFLAGS.l452n += -DSTM32_L452 -DSTM32_L4XX
STACK_END.l452n = 0x20028000

XCFLAGS.l452np += -DSTM32_L452 -DSTM32_L4XX
STACK_END.l452np = 0x20028000

XCFLAGS.l496n += -DSTM32_L496 -DSTM32_L4XX
XCFLAGS.l496n += -DCONFIG_CAN_TEST
STACK_END.l496n = 0x20040000

XCFLAGS.g0b1n += -DSTM32_G0B1 -DSTM32_G0XX
XCFLAGS.g0b1n += -DCONFIG_N_INTS=32
XCFLAGS.g0b1n += -DCONFIG_BUTTON_INT=0
XCFLAGS.g0b1n += -DCONFIG_CAN_TEST
CPU.g0b1n = cortex-m0plus
APPL_FLASH_BASE.g0b1n = 0x8000800
STACK_END.g0b1n = 0x20024000

XCFLAGS.g030deb += -DSTM32_G030 -DSTM32_G0XX
XCFLAGS.g030deb += -DCONFIG_N_INTS=32
XCFLAGS.g030deb += -DCONFIG_FAST_LOG_ENABLE=0
XCFLAGS.g030deb += -DCONFIG_TIMER_16BIT=1
XCFLAGS.g030deb += -DCONFIG_BUTTON_INT=1
XCFLAGS.g030deb += -DCONFIG_SHELL_HIST=2
XCFLAGS.g030deb += -DCONFIG_SHELL_MAX_LINE_LEN=32
XCFLAGS.g030deb += -DCONFIG_MALLOC_CHUNK_POW2=6
CPU.g030deb = cortex-m0plus
APPL_FLASH_BASE.g030deb = 0x8002800
STACK_END.g030deb = 0x20002000
OPTFLAGS.g030deb = -Os

XCFLAGS.c031n += -DSTM32_C031 -DSTM32_C0XX
XCFLAGS.c031n += -DCONFIG_N_INTS=32
#XCFLAGS.c031n += -DCONFIG_FAST_LOG_ENABLE=0
XCFLAGS.c031n += -DCONFIG_FAST_LOG_SHIFT=4
XCFLAGS.c031n += -DCONFIG_TIMER_16BIT=1
#XCFLAGS.c031n += -DCONFIG_BUTTON_INT=0
XCFLAGS.c031n += -DCONFIG_SHELL_HIST=2
XCFLAGS.c031n += -DCONFIG_SHELL_MAX_LINE_LEN=32
#XCFLAGS.c031n += -DCONFIG_SHELL_HELP=0
#XCFLAGS.c031n += -DCONFIG_STM32_HAL_COMMANDS=0
#XCFLAGS.c031n += -DCONFIG_HAL_DMA_COMMANDS=0
#XCFLAGS.c031n += -DCONFIG_SIMPLE_EXCEPTION_HANDLER=1
XCFLAGS.c031n += -DCONFIG_MALLOC_CHUNK_POW2=6
XCFLAGS.c031n += -DCONFIG_SYSLOG_NUM_LINES=32
CPU.c031n = cortex-m0plus
XCFLAGS.c031n += -DCONFIG_ENABLE_ADC
XCFLAGS.c031n += -DDISP
XCFLAGS.c031n += -DI2C_DEMO
APPL_FLASH_BASE.c031n = 0x8000800
STACK_END.c031n = 0x20003000
OPTFLAGS.c031n = -Os

XCFLAGS.g474n += -DSTM32_G4XX -DSTM32_G474
XCFLAGS.g474n += -DCONFIG_ENABLE_ADC
STACK_END.g474n = 0x20020000

XCFLAGS.g431kbn += -DSTM32_G4XX -DSTM32_G432
XCFLAGS.g431kbn += -DCONFIG_ENABLE_ADC
STACK_END.g431kbn = 0x20008000

XCFLAGS.u083n += -DSTM32_U083N -DSTM32_U0XX
CPU.u083n = cortex-m0plus
STACK_END.u083n = 0x2000a000

XCFLAGS.u575n += -DSTM32_U575 -DSTM32_U5XX
CPU.u575n = cortex-m33
STACK_END.u575n = 0x200c0000

XCFLAGS.wb55n += -DSTM32_WBXX -DSTM32_WB55
STACK_END.wb55n = 0x20030000

XCFLAGS.wb55usb += -DSTM32_WBXX -DSTM32_WB55
STACK_END.wb55usb = 0x20030000

XCFLAGS += -DARCH_STM32
XCFLAGS += -DAPPL
XCFLAGS += -DBMOS
XCFLAGS += -DCONFIG_POISON_STACK=1

LD_SCRIPT = $(OBJDIR)/appl.ld

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

# cmsis
XCFLAGS.cmsis += -I$(BMOS_ROOT)/external/cmsis_5/CMSIS/Core/Include
XCFLAGS.cmsisf4 += -I$(BMOS_ROOT)/external/st/cmsis_device_f4/Include
XCFLAGS.cmsish7 += -I$(BMOS_ROOT)/external/st/cmsis_device_h7/Include

XCFLAGS.f401bp += $(XCFLAGS.cmsis) $(XCFLAGS.cmsisf4)
XCFLAGS.f401bp64 += $(XCFLAGS.cmsis) $(XCFLAGS.cmsisf4)
XCFLAGS.f411bp += $(XCFLAGS.cmsis) $(XCFLAGS.cmsisf4)

XCFLAGS.h743wa += $(XCFLAGS.cmsis) $(XCFLAGS.cmsish7)

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
FILES += stm32_timer.o
FILES += circ_buf.o
FILES += hal_dma.o
FILES += stm32_dma_if.o
FILES += stm32_rcc_ls.o
FILES += xlib.o
FILES += mshell.o

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

FILES.h5xx += stm32_usart_b.o
FILES.h5xx += stm32_rcc_h5.o
FILES.h5xx += stm32_pwr_h5xx.o
FILES.h5xx += stm32_flash.o
FILES.h5xx += stm32_hal_gpdma.o
FILES.h5xx += stm32_exti_uxxx.o
FILES.h5xx += stm32_hal_rtc.o
FILES.h5xx += stm32_wdog.o
FILES.h5xx += stm32_eth_h7xx.o
FILES.h5xx += stm32_hal_sleep.o

FILES.h563n += $(FILES.h5xx)

FILES.h7xx += stm32_usart_b.o
FILES.h7xx += stm32_rcc_h7.o
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

FILES.h723n += stm32_fdcan.o
FILES.h723n += can_test.o

FILES.h735dk += stm32_lcd.o
FILES.h735dk += stm32_fdcan.o
FILES.h735dk += can_test.o

FILES.h743wa += stm32_hal_spi_b.o
FILES.h743wa += spi_test.o
FILES.h743wa += st7735.o


FILES.h745n += stm32_wwdg.o
FILES.h745n += stm32_fdcan.o
FILES.h745n += can_test.o


FILES.h563n += stm32_fdcan.o
FILES.h563n += can_test.o

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
#FILES.f0xx += stm32_hal_rtc.o

XSLOG.f030demo += xslog_simple.o
FILES.f030demo += $(FILES.f0xx)

XSLOG.f072n += xslog_simple.o
FILES.f072n += $(FILES.f0xx)

FILES.f1xx += stm32_exti_fxxx.o
FILES.f1xx += stm32_flash_f1.o
FILES.f1xx += stm32_hal_bdma.o
FILES.f1xx += stm32_pwr_f1xx.o
FILES.f1xx += stm32_rcc_f1.o
FILES.f1xx += stm32_usart_a.o
FILES.f1xx += stm32_hal_adc_fxxx.o

FILES.f3xx += stm32_exti_fxxx.o
FILES.f3xx += stm32_flash_f1.o
FILES.f3xx += stm32_hal_bdma.o
FILES.f3xx += stm32_hal_rtc.o
FILES.f3xx += stm32_pwr_f1xx.o
FILES.f3xx += stm32_rcc_f1.o
FILES.f3xx += stm32_usart_b.o

#FILES.f100d += stm32_hal_rtc.o # Needs new driver
XSLOG.f100d += xslog_simple.o
FILES.f100d += $(FILES.f1xx)

FILES.f103deb += $(FILES.f1xx)
XSLOG.f103deb = xslog_simple.o

FILES.f103bp += $(FILES.f1xx)
FILES.f103bp += adc.o
FILES.f103bp += stm32_can.o
FILES.f103bp += can_test.o

BOARD.at32f403bp += at32_board_f403bp.o
FILES.at32f403bp += $(FILES.f1xx)
FILES.at32f403bp += adc.o
FILES.at32f403bp += stm32_can.o
FILES.at32f403bp += can_test.o

FILES.f103n += $(FILES.f1xx)
FILES.f103n += adc.o

FILES.f303rt += stm32_wdog.o
FILES.f303rt += $(FILES.f3xx)

FILES.f411bp += stm32_hal_rtc.o
FILES.f411bp += stm32_usart_a.o
FILES.f411bp += stm32_rcc_a.o
FILES.f411bp += stm32_hal_dma.o
FILES.f411bp += stm32_hal_i2c_b.o
FILES.f411bp += font1.o
FILES.f411bp += fb.o
FILES.f411bp += ssd1306_fonts.o
FILES.f411bp += i2c_test.o
FILES.f411bp += ws2811.o
FILES.f411bp += ws2811_task.o
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
FILES.f401bp += stm32_hal_dma.o
FILES.f401bp += ws2811.o
FILES.f401bp += ws2811_task.o
FILES.f401bp += font1.o
FILES.f401bp += fb.o
FILES.f401bp += stm32_hal_adc_fxxx.o
FILES.f401bp += adc.o
FILES.f401bp += kvlog.o
FILES.f401bp += $(FILES.tusb)
FILES.f401bp += $(FILES.f4xx)
FILES.f401bp += $(FILES.fxxx)

FILES.f401bp64 += $(FILES.f401bp)
BOARD.f401bp64 += stm32_board_f401bp.o

FILES.f4d += stm32_hal_rtc.o
FILES.f4d += stm32_usart_a.o
FILES.f4d += stm32_rcc_a.o
FILES.f4d += stm32_hal_dma.o
FILES.f4d += stm32_eth.o
FILES.f4d += $(FILES.f4xx)
FILES.f4d += $(FILES.fxxx)

FILES.f407deb += stm32_hal_rtc.o
FILES.f407deb += stm32_usart_a.o
FILES.f407deb += stm32_rcc_a.o
FILES.f407deb += stm32_hal_dma.o
FILES.f407deb += stm32_eth.o
FILES.f407deb += $(FILES.f4xx)
FILES.f407deb += $(FILES.fxxx)

FILES.f407debm += stm32_hal_rtc.o
FILES.f407debm += stm32_usart_a.o
FILES.f407debm += stm32_rcc_a.o
FILES.f407debm += stm32_hal_dma.o
FILES.f407debm += $(FILES.f4xx)
FILES.f407debm += $(FILES.fxxx)

FILES.f429d += stm32_usart_a.o
FILES.f429d += stm32_rcc_a.o
FILES.f429d += stm32_hal_spi.o
FILES.f429d += stm32_hal_dma.o
FILES.f429d += stm32_lcd.o
FILES.f429d += $(FILES.f4xx)
FILES.f429d += $(FILES.fxxx)

FILES.f469d += stm32_usart_a.o
FILES.f469d += stm32_rcc_a.o
FILES.f469d += stm32_hal_dma.o
FILES.f469d += $(FILES.f4xx)
FILES.f469d += $(FILES.fxxx)

FILES.f746d += stm32_hal_dma.o
FILES.f746d += stm32_usart_b.o
FILES.f746d += stm32_rcc_a.o
FILES.f746d += stm32_hal_rtc.o
FILES.f746d += stm32_eth.o
FILES.f746d += stm32_lcd.o
FILES.f746d += $(FILES.f7xx)
FILES.f746d += $(FILES.fxxx)

FILES.f767n += stm32_hal_dma.o
FILES.f767n += stm32_usart_b.o
FILES.f767n += stm32_rcc_a.o
FILES.f767n += stm32_hal_rtc.o
FILES.f767n += stm32_eth.o
FILES.f767n += $(FILES.f7xx)
FILES.f767n += $(FILES.fxxx)

FILES.h723n += $(FILES.h7xx)
FILES.h735dk += $(FILES.h7xx)
FILES.h743wa += $(FILES.h7xx)
FILES.h743n += $(FILES.h7xx)
FILES.h745n += $(FILES.h7xx)
FILES.h745nm4 += $(FILES.h7xx)

FILES.g0xx += stm32_flash.o
FILES.g0xx += stm32_hal_bdma.o
FILES.g0xx += stm32_hal_dmamux.o
FILES.g0xx += stm32_hal_rtc.o
FILES.g0xx += stm32_pwr_g0xx.o
FILES.g0xx += stm32_rcc_g0.o
FILES.g0xx += stm32_usart_b.o

FILES.g0b1n += $(FILES.g0xx)
FILES.g0b1n += stm32_fdcan.o
FILES.g0b1n += can_test.o

FILES.c0xx += stm32_flash.o
FILES.c0xx += stm32_hal_bdma.o
FILES.c0xx += stm32_hal_dmamux.o
FILES.c0xx += stm32_hal_rtc.o
FILES.c0xx += stm32_pwr_g0xx.o
FILES.c0xx += stm32_rcc_g0.o
FILES.c0xx += stm32_usart_b.o
FILES.c0xx += stm32_hal_adc_g0.o
FILES.c0xx += stm32_hal_i2c.o
FILES.c0xx += font1.o
FILES.c0xx += ssd1306_fonts.o
FILES.c0xx += fb.o
FILES.c0xx += i2c_test.o

FILES.c031n += $(FILES.c0xx)
FILES.c031n += stm32_exti_uxxx.o
FILES.c031n += adc.o

FILES.g030deb += $(FILES.g0xx)
FILES.g030deb += stm32_exti_uxxx.o
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
FILES.g4xx += stm32_fdcan.o

FILES.wbxx += crc_ccitt16.o
FILES.wbxx += stm32_exti_lxxx.o
FILES.wbxx += stm32_flash.o
FILES.wbxx += stm32_hal_bdma.o
FILES.wbxx += stm32_pwr_lxxx.o
FILES.wbxx += stm32_rcc_b.o
FILES.wbxx += stm32_usart_b.o
FILES.wbxx += stm32_wdog.o

FILES.l0xx += stm32_usart_b.o
FILES.l0xx += stm32_rcc_l0.o
FILES.l0xx += stm32_flash_l0.o
FILES.l0xx += stm32_pwr_l0xx.o
FILES.l0xx += stm32_hal_bdma.o

FILES.l073n += $(FILES.l0xx)

FILES.l4xx += stm32_usart_b.o
FILES.l4xx += stm32_rcc_b.o
FILES.l4xx += stm32_flash.o
FILES.l4xx += crc_ccitt16.o
FILES.l4xx += stm32_wdog.o
FILES.l4xx += stm32_pwr_lxxx.o
FILES.l4xx += stm32_exti_lxxx.o

FILES.l432n += stm32_hal_bdma.o
FILES.l432n += stm32_hal_rtc.o
FILES.l432n += stm32_hal_spi.o
FILES.l432n += stm32_hal_adc.o
FILES.l432n += adc.o
FILES.l432n += ws2811.o
FILES.l432n += ws2811_task.o

FILES.l452n += stm32_hal_bdma.o
FILES.l452n += stm32_hal_rtc.o
FILES.l452n += stm32_hal_sleep.o

FILES.l452np += stm32_hal_bdma.o
FILES.l452np += stm32_hal_rtc.o
FILES.l452np += stm32_hal_sleep.o

FILES.l496n += stm32_hal_bdma.o
FILES.l496n += stm32_hal_spi.o
FILES.l496n += stm32_hal_adc.o
FILES.l496n += stm32_can.o
FILES.l496n += can_test.o

FILES.l432n += $(FILES.l4xx)
FILES.l452n += $(FILES.l4xx)
FILES.l452np += $(FILES.l4xx)
FILES.l496n += $(FILES.l4xx)

#FILES.l496n += ds1302.o
#FILES.l496n += ds1302_test.o
FILES.l496n += stm32_hal_rtc.o

FILES.l4rn += stm32_hal_bdma.o
FILES.l4rn += stm32_usart_b.o
FILES.l4rn += stm32_rcc_b.o
FILES.l4rn += stm32_wdog.o
FILES.l4rn += stm32_pwr_lxxx.o
FILES.l4rn += stm32_exti_lxxx.o
FILES.l4rn += stm32_flash.o

FILES.g474n += $(FILES.g4xx)
FILES.g474n += stm32_hal_rtc.o
FILES.g474n += stm32_hal_i2c.o
FILES.g474n += font1.o
FILES.g474n += fb.o
FILES.g474n += ssd1306_fonts.o
FILES.g474n += i2c_test.o
FILES.g474n += can_test.o
FILES.g474n += stm32_hal_adc.o
FILES.g474n += adc.o

FILES.g431kbn += $(FILES.g4xx)
FILES.g431kbn += stm32_hal_rtc.o
FILES.g431kbn += can_test.o
FILES.g431kbn += stm32_hal_adc.o
FILES.g431kbn += adc.o

FILES.u083n += stm32_hal_rtc.o
FILES.u083n += stm32_exti_uxxx.o
FILES.u083n += stm32_pwr_g0xx.o
FILES.u083n += stm32_rcc_g0.o
FILES.u083n += stm32_usart_b.o
FILES.u083n += stm32_hal_bdma.o
FILES.u083n += stm32_hal_dmamux.o
FILES.u083n += stm32_flash.o

FILES.u575n += stm32_hal_rtc.o
FILES.u575n += stm32_exti_uxxx.o
FILES.u575n += stm32_pwr_uxxx.o
FILES.u575n += stm32_rcc_u5.o
FILES.u575n += stm32_usart_b.o
FILES.u575n += stm32_hal_gpdma.o
FILES.u575n += stm32_fdcan.o
FILES.u575n += can_test.o
FILES.u575n += stm32_flash.o

FILES.wb55n += $(FILES.wbxx)
FILES.wb55n += stm32_hal_rtc.o

FILES.wb55usb += $(FILES.wbxx)
FILES.wb55usb += stm32_hal_rtc.o
CPU.f100d = cortex-m3

ifdef XSLOG.$(PROG)
FILES += $(XSLOG.$(PROG))
else
FILES += xslog.o
endif

include ../Makefile.rules

endif # PROG
