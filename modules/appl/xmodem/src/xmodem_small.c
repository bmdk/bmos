/* Copyright (c) 2019-2022 Brian Thomas Murphy
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

#include <string.h>

#include "debug_ser.h"
#include "crc_ccitt16.h"
#include "io.h"
#include "xmodem.h"

#define XMODEM_STATE_INIT 1
#define XMODEM_STATE_START 2
#define XMODEM_STATE_PKTNUM_1 3
#define XMODEM_STATE_PKTNUM_2 4
#define XMODEM_STATE_DATA 5

#define SOH 0x01
#define STX 0x02
#define EOT 0x04
#define ACK 0x06
#define NAK 0x15
#define CAN 0x18

#define XMODEM_RETRIES 0

void xmodem_start(xmodem_data_t *d)
{
  d->state = XMODEM_STATE_INIT;
  d->data_len = 0;
#if XMODEM_RETRIES > 0
  d->retries = XMODEM_RETRIES;
#endif
  d->pktnum = 1;
  debug_putc('C');
}

int xmodem_data(xmodem_data_t *d, int ch)
{
  if (ch < 0) {
    if (d->state != XMODEM_STATE_INIT) {
      d->data_len = 0;
      debug_putc(NAK);
      d->state = XMODEM_STATE_START;
    } else {
#if XMODEM_RETRIES > 0
      debug_putc('C');
      d->retries--;
      if (d->retries == 0)
        goto abort;
#else
      goto abort;
#endif
    }
    return 1;
  }

  if (d->state == XMODEM_STATE_INIT)
    d->state = XMODEM_STATE_START;

  if (d->state == XMODEM_STATE_START) {
    switch (ch) {
    case EOT:
      debug_putc(ACK);
      return 0;
    case SOH:
      d->pktlen = 128;
      break;
    case STX:
      d->pktlen = 1024;
      break;
    default:
      goto abort;
    }
    d->state = XMODEM_STATE_PKTNUM_1;
  } else if (d->state == XMODEM_STATE_PKTNUM_1) {
    if ((unsigned char)ch != d->pktnum)
      goto abort;
    d->state = XMODEM_STATE_PKTNUM_2;
  } else if (d->state == XMODEM_STATE_PKTNUM_2) {
    if ((unsigned char)ch + d->pktnum != 255)
      goto abort;
    d->state = XMODEM_STATE_DATA;
    d->data_len = 0;
  } else if (d->state == XMODEM_STATE_DATA) {
    unsigned int crc, pkt_crc;

    d->data[d->data_len] = (unsigned char)ch;
    d->data_len++;
    if (d->data_len == d->pktlen + 2) {
      crc = crc_ccitt16(0, d->data, d->pktlen);
      pkt_crc = (((unsigned int)d->data[d->pktlen]) << 8) + \
                d->data[d->pktlen + 1];
      if (crc != pkt_crc)
        goto abort;

      if (xmodem_block(d->block_ctx, d->data, d->pktlen))
        goto abort;

      d->data_len = 0;
      debug_putc(ACK);
      d->state = XMODEM_STATE_START;
      d->pktnum++;
    }
  }

  return 1;
abort:
  return -1;
}
