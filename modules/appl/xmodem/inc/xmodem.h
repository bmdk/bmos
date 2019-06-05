/* Copyright (c) 2019 Brian Thomas Murphy
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

#ifndef XMODEM_H
#define XMODEM_H

typedef void xmodem_putc_t (int c);
typedef int xmodem_block_t (void *block_ctx, void *data, unsigned int len);

#define XMODEM_PKT_SIZ (1024 + 2)

typedef struct {
  xmodem_putc_t *putc;
  xmodem_block_t *block;
  void *block_ctx;
  /* DATA */
  char data[XMODEM_PKT_SIZ];
  unsigned short timeout;
  unsigned short data_len;
  unsigned short pktlen;
  unsigned char state;
  unsigned char retries;
  unsigned char pktnum;
} xmodem_data_t;

void xmodem_start(xmodem_data_t *d);
int xmodem_data(xmodem_data_t *d, void *data, unsigned int len);

#endif
