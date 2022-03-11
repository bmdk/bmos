#ifndef RP2040_RESETS_H
#define RP2040_RESETS_H

#define RESETS_RESET_UART1 23
#define RESETS_RESET_UART0 22
#define RESETS_RESET_PADS_BANK0 8
#define RESETS_RESET_IO_BANK0 5
#define RESETS_RESET_DMA 2
#define RESETS_RESET_ADC 0

void rp2040_reset_clr(unsigned int n);

#endif
