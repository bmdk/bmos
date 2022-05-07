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

#define XMODEM_INIT_TIMEOUT 2
#define XMODEM_TIMEOUT 1
#define XMODEM_RETRIES 10

void xmodem_start(xmodem_data_t *d)
{
  d->state = XMODEM_STATE_INIT;
  d->data_len = 0;
  d->timeout = XMODEM_INIT_TIMEOUT;
  d->retries = 10;
  d->putc('C');
  d->pktnum = 1;
}

int xmodem_data(xmodem_data_t *d, void *data, unsigned int len)
{
  const char *c = (const char *)data;
  int i, err;

  if (len == 0) {
    if (d->timeout > 0) {
      d->timeout--;
      if (d->timeout == 0) {
        if (d->state != XMODEM_STATE_INIT) {
          d->retries--;
          if (d->retries == 0) {
            err = -6;
            goto abort;
          }
          d->data_len = 0;
          d->putc(NAK);
          d->state = XMODEM_STATE_START;
          return 1;
        }
        d->timeout = XMODEM_INIT_TIMEOUT;
        d->putc('C');
        d->retries--;
        if (d->retries == 0) {
          err = -2;
          goto abort;
        }
        return 1;
      }
      return 1;
    }
  } else {
    d->timeout = XMODEM_TIMEOUT;
    d->retries = XMODEM_RETRIES;
  }

  while (len > 0) {
    if (d->state == XMODEM_STATE_INIT)
      d->state = XMODEM_STATE_START;

    if (d->state == XMODEM_STATE_START) {
      switch (*c++) {
      case EOT:
        d->putc(ACK);
        return 0;
      case SOH:
        d->pktlen = 128;
        break;
      case STX:
        d->pktlen = 1024;
        break;
      default:
        err = -3;
        goto abort;
      }
      len--;
      d->state = XMODEM_STATE_PKTNUM_1;
      if (len == 0)
        return 1;
    }

    if (d->state == XMODEM_STATE_PKTNUM_1) {
      unsigned char pktnum = *c++;
      len--;
      if (pktnum != d->pktnum) {
        err = -4;
        goto abort;
      }
      d->state = XMODEM_STATE_PKTNUM_2;
      d->timeout = XMODEM_TIMEOUT;
      if (len == 0)
        return 1;
    }

    if (d->state == XMODEM_STATE_PKTNUM_2) {
      unsigned char pktnum = *c++;
      len--;
      if (pktnum + d->pktnum != 255) {
        err = -4;
        goto abort;
      }
      d->state = XMODEM_STATE_DATA;
      d->timeout = XMODEM_TIMEOUT;
      d->data_len = 0;
      if (len == 0)
        return 1;
    }

    if (d->state == XMODEM_STATE_DATA) {
      unsigned int rem, crc, pkt_crc;

      rem = d->pktlen + 2 - d->data_len;
      if (rem > len)
        rem = len;
      memcpy(&d->data[d->data_len], c, rem);
      c += rem;
      d->data_len += rem;
      if (d->data_len == d->pktlen + 2) {
        crc = crc_ccitt16(0, d->data, d->pktlen);
        pkt_crc = (((unsigned int)d->data[d->pktlen]) << 8) + \
                  d->data[d->pktlen + 1];
        if (crc != pkt_crc) {
          err = -5;
          goto abort;
        }

        err = d->block(d->block_ctx, d->data, d->pktlen);
        if (err != 0)
          goto abort;

        d->data_len = 0;
        d->putc(ACK);
        d->state = XMODEM_STATE_START;
        d->pktnum++;
      }
      len -= rem;
      if (len == 0)
        return 1;
    }
  }

  return 1;

abort:
  for (i = 0; i < 8; i++)
    d->putc(CAN);
  for (i = 0; i < 8; i++)
    d->putc('\b');

  return err;
}
