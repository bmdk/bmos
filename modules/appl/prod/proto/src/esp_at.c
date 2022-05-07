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

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "bmos_queue.h"
#include "bmos_syspool.h"
#include "bmos_task.h"
#include "fast_log.h"
#include "kvlog.h"
#include "onewire.h"
#include "xassert.h"
#include "xslog.h"
#include "xtime.h"

//#define DEBUG 1

extern uart_t esp_uart;
static bmos_queue_t *esp_rxq;
static bmos_queue_t *esp_txq;

static char temp_host_ip[20] = "";
static unsigned int temp_host_port;
static char wifi_ssid[65];
static char wifi_psk[65];

static char at_cmd[160];

#if ONE_WIRE
static void ow_temp(unsigned int idx, unsigned int temp)
{
  unsigned char *p;
  static bmos_op_msg_t *m;

  xslog(LOG_INFO, "OW: idx: %d temp %d.%03d", idx, temp / 1000, temp % 1000);

  if (esp_rxq) {
    m = op_msg_wait(syspool);

    p = BMOS_OP_MSG_GET_DATA(m);

    FAST_LOG('T', "%p %p\n", m, p);

    *(unsigned int *)p = temp;

    op_msg_put(esp_rxq, m, 1, sizeof(unsigned int));
  }
}

static void ow_found(unsigned int count, unsigned char devid[][OW_DEVID_LEN])
{
  unsigned int i;

  for (i = 0; i < count; i++) {
    unsigned char *d = devid[i];

    xslog(LOG_INFO, "OW: idx %d owid: %02x%02x%02x%02x%02x%02x%02x%02x\n",
          i, d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]);
  }

}
#endif

static void _esp_uart_write(void *data, unsigned int len)
{
  unsigned char *p;
  static bmos_op_msg_t *m;

  if (len > SYSPOOL_SIZE)
    return;

  m = op_msg_wait(syspool);
  XASSERT(m);

  p = BMOS_OP_MSG_GET_DATA(m);

  memcpy(p, data, len);

  op_msg_put(esp_txq, m, 0, len);
}

static void _esp_uart_write_str(char *str)
{
  _esp_uart_write(str, strlen(str));
}

static void _esp_uart_write_packet(void *data, unsigned int len)
{
  snprintf(at_cmd, sizeof(at_cmd), "AT+CIPSEND=%d\r\n", len);
  _esp_uart_write(at_cmd, strlen(at_cmd));

  task_delay(20);

  _esp_uart_write(data, len);
}

#define ESP_DATA_LEN 256

typedef struct {
  char data[ESP_DATA_LEN];
  unsigned int idx;
} esp_rx_data_t;

esp_rx_data_t esp_rx_data;

static void esp_rx_flush(esp_rx_data_t *rxd)
{
  rxd->idx = 0;
}

#if 1
void dump(const char *name, void *data, unsigned int len)
{
  unsigned int i;
  char *d = (char *)data;
  char lbuf[64];
  char *plbuf = lbuf;
  unsigned int rlen = sizeof(lbuf);

  for (i = 0; i < len; i++) {
    int l;

    l = snprintf(plbuf, rlen, "%02x ", d[i]);
    rlen -= l;
    plbuf += l;
  }

  xslog(LOG_INFO, "%s: %s", name, lbuf);
}
#endif


static int esp_rx(esp_rx_data_t *rxd, void *data, unsigned int len)
{
  int rem = ESP_DATA_LEN - rxd->idx;

  if (len + 1 > rem) {
#if 0
    xslog(LOG_INFO, "esp data overflow");
#endif
    return -1;
  }

  memcpy(&rxd->data[rxd->idx], data, len);
  rxd->idx += len;
  rxd->data[rxd->idx] = '\0';

  return 0;
}

static int esp_rx_readline(esp_rx_data_t *rxd,
                           char *line, unsigned int len)
{
  char *eol, *d = (char *)rxd->data;
  unsigned int line_len, idx = rxd->idx;

  if (idx == 0)
    return 0;

  /* discard an initial line feed */
  if (*d == '\n') {
    d++;
    idx--;
  }

  if (idx == 0)
    return 0;

  eol = strchr(d, '\r');
  if (!eol)
    return 0;
  *eol = '\0';

  line_len = eol - d + 1;
  if (line_len < len)
    memcpy(line, d, line_len);

  d += line_len;
  if (line_len < idx) {
    idx -= line_len;
    memmove(rxd->data, d, idx + 1);
    rxd->idx = idx;
  } else
    rxd->idx = 0;

  return line_len;
}

#define ESP_NOP 0
#define ESP_CHECK_CONN 1
#define ESP_RESET 2
#define ESP_SET_MODE_STATION 3
#define ESP_CONNECT_WIFI 4
#define ESP_CONNECT_UDP 5
#define ESP_CONNECT_DONE 6

#if DEBUG
/* *INDENT-OFF* */
static const char *connect_state[] = {
  "ESP_NOP", "ESP_CHECK_CONN", "ESP_RESET", "ESP_SET_MODE_STATION",
  "ESP_CONNECT_WIFI", "ESP_CONNECT_UDP", "ESP_CONNECT_DONE"
};
/* *INDENT-ON* */

static const char *decode_connect_state(unsigned int state)
{
  if (state > ESP_CONNECT_DONE)
    state = 0;

  return connect_state[state];
}
#endif

int connect(int *wait_ok)
{
  static int cur = ESP_CHECK_CONN;

  *wait_ok = 1;

#if DEBUG
  xslog(LOG_INFO, "connect state %s\n", decode_connect_state(cur));
#endif

  switch (cur) {
  case ESP_CHECK_CONN:
    _esp_uart_write_str("AT\r\n");
    cur = ESP_RESET;
    return 50;
    break;

  case ESP_RESET:
    _esp_uart_write_str("AT+RST\r\n");
    cur = ESP_SET_MODE_STATION;
    *wait_ok = 0;
    return 1000;
    break;

  case ESP_SET_MODE_STATION:
    esp_rx_flush(&esp_rx_data);
    _esp_uart_write_str("AT+CWMODE_CUR=1\r\n");
    cur = ESP_CONNECT_WIFI;
    return 50;
    break;

  case ESP_CONNECT_WIFI:
    snprintf(at_cmd, sizeof(at_cmd),
             "AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", wifi_ssid, wifi_psk);
    _esp_uart_write_str(at_cmd);
    cur = ESP_CONNECT_UDP;
    return 30000;
    break;

  case ESP_CONNECT_UDP:
    snprintf(at_cmd, sizeof(at_cmd),
             "AT+CIPSTART=\"UDP\",\"%s\",%d\r\n",
             temp_host_ip, temp_host_port);
    _esp_uart_write_str(at_cmd);
    cur = ESP_CONNECT_DONE;
    return 50;
    break;

  case ESP_CONNECT_DONE:
    cur = ESP_NOP;
    return -1;
    break;

  case ESP_NOP:
    return -1;
    break;

  default:
    cur = ESP_CHECK_CONN;
    return 50;
    break;
  }
}

void esp_task(void *data)
{
  unsigned char *p;
  bmos_op_msg_t *m;
  char buf[64];
  xtime_ms_t start = xtime_ms();
  xtime_ms_t now;
  int connected = 0;
  int delay, wait_ok;

  esp_rx_flush(&esp_rx_data);

  delay = connect(&wait_ok);

  for (;;) {
    m = op_msg_wait_ms(esp_rxq, delay);

    now = xtime_ms();

    if (delay >= 0 && xtime_diff_ms(now, start) > delay) {
      xslog(LOG_INFO, "TIMEOUT");
      delay = connect(&wait_ok);
      start = now;
      if (delay < 0)
        connected = 1;
    }

    if (!m)
      continue;

    switch (m->op) {
    case 0:
      esp_rx(&esp_rx_data, BMOS_OP_MSG_GET_DATA(m), m->len);
      while (esp_rx_readline(&esp_rx_data, buf, sizeof(buf)) > 0) {
#if DEBUG
        unsigned int slen = strlen(buf);
        if (slen > 0)
          xslog(LOG_INFO, "E: %s", buf);
#endif

        if (wait_ok && !strcmp(buf, "OK")) {
          delay = connect(&wait_ok);
          start = now;
          if (delay < 0)
            connected = 1;
        }
      }
      break;
    case 1:
      if (connected) {
        if (m->len == sizeof(unsigned int)) {
          p = BMOS_OP_MSG_GET_DATA(m);
          snprintf(buf, sizeof(buf), "{\"temp\":%d}", *(unsigned int *)p);
          _esp_uart_write_packet(buf, strlen(buf));
        }
      }
      break;
    }

    op_msg_return(m);
  }
}

void esp_init()
{
  const char *s;

#if ONE_WIRE
  unsigned int ow_gpio_bank = kv_get_uint("ow_gpio_bank");
  unsigned int ow_gpio_pin = kv_get_uint("ow_gpio_pin");

  one_wire_init_gpio(ow_found, ow_temp, GPIO(ow_gpio_bank, ow_gpio_pin), 2000);
#endif

  s = kv_get_str("temp_host_ip");
  if (!s || strlen(s) >= sizeof(temp_host_ip))
    return;

  strcpy(temp_host_ip, s);

  temp_host_port = kv_get_uint("temp_host_port");

  s = kv_get_str("wifi_ssid");
  if (!s || strlen(s) >= sizeof(wifi_ssid))
    return;

  strcpy(wifi_ssid, s);

  s = kv_get_str("wifi_psk");
  if (!s || strlen(s) >= sizeof(wifi_psk))
    return;

  strcpy(wifi_psk, s);

  esp_rxq = queue_create("espin", QUEUE_TYPE_TASK);
  esp_txq = uart_open(&esp_uart, 115200, esp_rxq, 0);

  task_init(esp_task, NULL, "esp", 2, 0, 512);
}
