/* Copyright (c) 2019-2021 Brian Thomas Murphy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef ONEWIRE_H
#define ONEWIRE_H

#include "hal_gpio.h"
#include "hal_uart.h"

#define OW_DEVID_LEN 8

typedef void ow_found_cb_t(unsigned int count,
                           unsigned char devid[][OW_DEVID_LEN]);
typedef void ow_temp_cb_t(unsigned int idx, unsigned int temp);

void one_wire_init_uart(ow_found_cb_t *found, ow_temp_cb_t *temp, uart_t *uart,
                        unsigned int poll_interval);
void one_wire_init_gpio(ow_found_cb_t *found, ow_temp_cb_t *temp,
                        gpio_handle_t gpio, unsigned int poll_interval);


#endif
