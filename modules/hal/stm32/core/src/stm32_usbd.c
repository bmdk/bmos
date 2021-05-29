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

#include <string.h>

#include "bmos_task.h"
#include "common.h"
#include "hal_int.h"
#include "io.h"
#include "shell.h"

#include "xslog.h"

#define USB_FS_BASE 0x50000000

struct usb_otg_fs_t {
  unsigned int gotgctl;
  unsigned int gotgint;
  unsigned int gahbcfg;
  unsigned int gusbcfg;
  unsigned int grstctl;
  unsigned int gintsts;
  unsigned int gintmsk;
  unsigned int grxstsr;
  unsigned int grxstsp;
  unsigned int grxfsiz;
  unsigned int dieptxf0;
  unsigned int hnptxsts;
  unsigned int pad1[2];
  unsigned int gccfg;
  unsigned int cid;
  unsigned int pad2[48];
  unsigned int hptxfsiz;
  unsigned int dieptxf1;
  unsigned int dieptxf2;
  unsigned int dieptxf3;
  unsigned int pad3[188];
  unsigned int hcfg;
  unsigned int hfir;
  unsigned int hfnum;
  unsigned int pad4[1];
  unsigned int hptxsts;
  unsigned int haint;
  unsigned int haintmsk;
  unsigned int pad5[9];
  unsigned int hprt;
  unsigned int pad6[47];
  struct hc_t {
    unsigned int hcchar;
    unsigned int pad1;
    unsigned int hcint;
    unsigned int hcintmsk;
    unsigned int hctsiz;
    unsigned int pad2[3];
  } hc[8];
  unsigned int pad7[128];
  unsigned int dcfg;
  unsigned int dctl;
  unsigned int dsts;
  unsigned int pad8;
  unsigned int diepmsk;
  unsigned int doepmsk;
  unsigned int daint;
  unsigned int daintmsk;
  unsigned int pad9[3];
  unsigned int dvbusdis;
  unsigned int dvbuspulse;
  unsigned int diepempmsk;
  unsigned int pad10[50];
  struct din_t {
    unsigned int diepctl;
    unsigned int pad0;
    unsigned int diepint;
    unsigned int pad1;
    unsigned int dieptsiz;
    unsigned int pad2;
    unsigned int dtxfsts;
    unsigned int pad3;
  } din[8];
  unsigned int pad11[64];
  struct doout_t {
    unsigned int doepctl;
    unsigned int pad0;
    unsigned int doepint;
    unsigned int pad1;
    unsigned int doeptsiz;
    unsigned int pad3[3];
  } dout[8];
};

#define USB_GUSBCFG_PHYSEL BIT(6)
#define USB_GUSBCFG_TRDT(v) (((v) & 0x7) << 10)
#define USB_GUSBCFG_FDMOD BIT(30)

#define USB_GCCFG_NOVBUSSENS BIT(21)
#define USB_GCCFG_SFOUTEN BIT(20)
#define USB_GCCFG_VBUSBSEN BIT(19)
#define USB_GCCFG_VBUSASEN BIT(18)
#define USB_GCCFG_PWRDWN BIT(16)

#define DIEPCTL_EPENA BIT(31)
#define DIEPCTL_EPDIS BIT(30)
#define DIEPCTL_SNAK BIT(27)
#define DIEPCTL_CNAK BIT(26)

static volatile struct usb_otg_fs_t *usb = (void *)USB_FS_BASE;

#define USB_GINTSTS_IEPINT BIT(18)
#define USB_GINTSTS_OEPINT BIT(19)
#define USB_GINTSTS_RXFLVL BIT(4)

void usb_init()
{
  usb->gusbcfg = USB_GUSBCFG_FDMOD;
  usb->gccfg = USB_GCCFG_NOVBUSSENS;
}
