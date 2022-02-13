#ifndef ONEWIRE_H
#define ONEWIRE_H

#include "hal_gpio.h"
#include "hal_uart.h"

#define OW_DEVID_LEN 8

typedef void ow_found_cb_t(unsigned int count,
                           unsigned char devid[][OW_DEVID_LEN]);
typedef void ow_temp_cb_t(unsigned int idx, unsigned int temp);

void one_wire_init_uart(ow_found_cb_t *found, ow_temp_cb_t *temp, uart_t *uart);
void one_wire_init_gpio(ow_found_cb_t *found, ow_temp_cb_t *temp,
                        gpio_handle_t gpio);


#endif
