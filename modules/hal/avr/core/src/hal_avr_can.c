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
#include <avr/interrupt.h>

#include "common.h"
#include "io.h"

#define TX_MOB_START 8
#define N_MOBS 15

static inline void set_can_page(unsigned int n)
{
  CANPAGE = ((n & 0xf) << 4);
}

void can_init(unsigned int canid)
{
  int i;

  CANGCON = BIT(SWRES);

  /* 250 kbits/s */
  CANBT1 = 0x06;
  CANBT2 = 0x0C;
  CANBT3 = 0x37;

  CANGIT = 0;
  CANGIE = 0;

  CANTCON = 199;

  CANIE1 = 0;
  CANIE2 = 0;

  for (i = 0; i < N_MOBS; i++) {
    set_can_page(i);

    CANCDMOB &= 0;
    CANSTMOB &= 0;
  }

  /* set up RX mobs */
  for (i = 0; i < TX_MOB_START; i++) {
    set_can_page(i);

    CANSTMOB = 0;
    CANCDMOB = BIT(CONMOB1);

    CANIDT4 = 0;
    CANIDT3 = 0;
    CANIDT2 = (uint8_t)(canid << 5);
    CANIDT1 = (uint8_t)(canid >> 3);

    CANIDM4 = BIT(IDEMSK) | BIT(RTRMSK);
    CANIDM3 = 0;
    CANIDM2 = (uint8_t)(canid << 5);
    CANIDM1 = (uint8_t)(canid >> 3);
  }

  CANGCON = BIT(ENASTB);
}

int can_recv(void *buf)
{
  uint8_t mob;

  for (mob = 0; mob < TX_MOB_START; mob++) {
    set_can_page(mob);

    if (CANSTMOB & BIT(RXOK)) {
      int len, i;
      unsigned char *cbuf = (unsigned char *)buf;

      CANSTMOB &= 0;

      len = CANCDMOB & 0x0f;

      for (i = 0; i < len; i++)
        *cbuf++ = CANMSG;

      CANIE2 |= BIT(mob);
      CANCDMOB = BIT(CONMOB1);

      return len;
    }
  }

  return -1;
}

static void can_write(unsigned int id, const void *data, unsigned int len)
{
  const uint8_t *cdata = (const uint8_t *)data;
  uint8_t i;

  if (len > 8)
    len = 8;

  CANSTMOB &= 0;

  CANIDT4 = 0;
  CANIDT3 = 0;
  CANIDT2 = (uint8_t)(id << 5);
  CANIDT1 = (uint8_t)(id >> 3);

  for (i = 0; i < len; i++)
    CANMSG = *cdata++;

  CANCDMOB = BIT(CONMOB0) | len;
}

int can_send(unsigned int id, void *buf, unsigned int len)
{
  for (uint8_t mob = TX_MOB_START; mob < N_MOBS; mob++) {
    set_can_page(mob);

    /* TX done */
    if (CANSTMOB & BIT(TXOK)) {
      CANSTMOB &= 0;
      CANCDMOB &= 0;
    }

    if ((CANCDMOB & (BIT(CONMOB1) | BIT(CONMOB0))) == 0) {
      can_write(id, buf, len);
      return 0;
    }
  }

  return -1;
}
