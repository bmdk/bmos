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

#include <avr/io.h>

#include "common.h"
#include "hal_avr_common.h"
#include "hal_avr_uart.h"
#include "io.h"

#define BAUD 57600

#define AVR_USART_DIV(_clk_, _baud_) ((_clk_ + (16 * (_baud_) / 2) - 1) / \
                                      16 / (_baud_) - 1)

#define USART_DIV AVR_USART_DIV(CLOCK, BAUD)

#if __AVR_AT90CAN128__
void uart_init(void)
{
  UBRR0H = (USART_DIV >> 8) & 0xff;
  UBRR0L = USART_DIV & 0xff;

  UCSR0A &= ~BIT(1);

  UCSR0C = BIT(1) | BIT(2); /* 8-bit data */
  UCSR0B = BIT(3) | BIT(4); /* Enable RX and TX */
}

void debug_putc(int ch)
{
  while ( !( UCSR0A & BIT(UDRE)) )
    ;
  UDR0 = ch & 0xff;
}

int debug_getc(void)
{
  if ( !(UCSR0A & BIT(RXC)) )
    return -1;
  return UDR0;
}
#elif __AVR_ATmega128__
void uart_init(void)
{
  UBRR1H = (USART_DIV >> 8) & 0xff;
  UBRR1L = USART_DIV & 0xff;

  UCSR1A &= ~BIT(1);

  UCSR1C = BIT(1) | BIT(2); /* 8-bit data */
  UCSR1B = BIT(3) | BIT(4); /* Enable RX and TX */
}

void debug_putc(int ch)
{
  while ( !( UCSR1A & BIT(UDRE)) )
    ;
  UDR1 = ch & 0xff;
}

int debug_getc(void)
{
  if ( !(UCSR1A & BIT(RXC)) )
    return -1;
  return UDR1;
}
#endif

int debug_ser_tx_done(void)
{
  return 1;
}
