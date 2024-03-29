#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#define BOARD_DEVICE_RHPORT_NUM     0

#define BOARD_DEVICE_RHPORT_SPEED   OPT_MODE_FULL_SPEED

#define CFG_TUSB_RHPORT0_MODE     (OPT_MODE_DEVICE | BOARD_DEVICE_RHPORT_SPEED)

#define CFG_TUSB_OS               OPT_OS_CUSTOM

#define CFG_TUSB_MEM_SECTION

#define CFG_TUSB_MEM_ALIGN          __attribute__ ((aligned(4)))

#define CFG_TUD_ENDPOINT0_SIZE    64

#define CFG_TUD_CDC              1
#define CFG_TUD_MSC              0
#define CFG_TUD_HID              0
#define CFG_TUD_MIDI             0
#define CFG_TUD_VENDOR           0

#define CFG_TUD_CDC_RX_BUFSIZE   64
#define CFG_TUD_CDC_TX_BUFSIZE   64

#endif
