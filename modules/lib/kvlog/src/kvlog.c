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

#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if TEST
#include <syslog.h>
#define xslog syslog
#define xprintf printf
#else
#include <xslog.h>

#include <shell.h>
#include <io.h>
#endif

#include <kvlog.h>

#if !TEST
#include <stm32_flash.h>
#endif

#define FLASH_SECT1 0x8008000
#define FLASH_SECT2 0x800c000

#define KV_ALIGN(_num_) ALIGN(_num_, 3)

#define KV_STORE_HDR_MAGIC 0x4B560001

#if STM32_L4XX
#define ONLY_REWRITE_ZERO 1
#endif

typedef struct {
  unsigned int magic;
  unsigned int seq;
} kv_store_hdr_t;

typedef struct {
  unsigned char start;
#if ONLY_REWRITE_ZERO
  unsigned char pad1[7];
  unsigned char pad2[5];
#endif
  unsigned char type;
  unsigned short rlen;
} kv_hdr_t;

#define KV_HDR_START 0x99
#define KV_HDR_DELETED 0x00
#define KV_HDR_ERASED 0xff

#define KV_TYPE_INVALID 0
#define KV_TYPE_STR 1
#define KV_TYPE_INT 2
#define KV_TYPE_UINT 3

#define FLASH_BUF_LEN 64

typedef struct {
  void *data;
  unsigned int pos;
} kv_data_store_t;

typedef struct {
  kv_data_store_t store[2];
  kv_data_store_t *current;
  kv_data_store_t *copy;
  unsigned int size;
} kv_data_t;

kv_data_t kv_data;

#define KV_ITER_STATUS_OK 0
#define KV_ITER_STATUS_INVALID -1
typedef struct {
  kv_hdr_t *hdr;
  unsigned int ofs;
  int status;
} kv_iter_t;

static int kv_iter_start(kv_data_store_t *store, kv_iter_t *it)
{
  kv_store_hdr_t *shdr = (kv_store_hdr_t *)store->data;

  if (shdr->magic != KV_STORE_HDR_MAGIC)
    return -1;

  it->hdr = (kv_hdr_t *)(shdr + 1);
  it->ofs = sizeof(kv_store_hdr_t);
  it->status = KV_ITER_STATUS_OK;

  return shdr->seq;
}

static kv_hdr_t *kv_iter_next(kv_iter_t *it)
{
  unsigned int rlen;
  kv_hdr_t *hdr = it->hdr;

  if (hdr->start != KV_HDR_START && hdr->start != KV_HDR_DELETED)
    return NULL;

  rlen = hdr->rlen;

  /* if the current record size brings us past the end of the flash
     block then something is wrong */
  if (it->ofs + rlen > kv_data.size) {
    it->status = KV_ITER_STATUS_OK;
    return NULL;
  }

  it->hdr = (kv_hdr_t *)((unsigned char *)(hdr) + rlen);
  it->ofs += rlen;

  return hdr;
}

static unsigned int kv_iter_get_ofs(kv_iter_t *it)
{
  return it->ofs;
}

static int kv_iter_status(kv_iter_t *it)
{
  return it->status;
}

#if TEST
static int _kv_write_header(kv_data_store_t *sp, unsigned int seq, int init_pos)
{
  kv_store_hdr_t hdr;

  hdr.magic = KV_STORE_HDR_MAGIC;
  hdr.seq = seq;

  memcpy(sp->data, &hdr, sizeof(kv_store_hdr_t));
  if (init_pos)
    sp->pos = sizeof(kv_store_hdr_t);

  return 0;
}

static int _kv_write(kv_data_store_t *sp, kv_hdr_t *rec)
{
  unsigned int rlen = rec->rlen;

  if (kv_data.size < sp->pos + rlen)
    return -1;

  memcpy(sp->data + sp->pos, rec, rlen);
  sp->pos += rlen;

  return 0;
}

static void kv_clear(void *data, unsigned int size)
{
  memset(data, 0xff, size);
}

static void kv_store_reinit(kv_data_store_t *store)
{
  kv_clear(store->data, kv_data.size);
  store->pos = 0;
}

static int kv_store_init(kv_data_store_t *store, unsigned int size)
{
  unsigned char *d = malloc(size);

  if (!d)
    return -1;
  store->data = d;

  kv_store_reinit(store);

  return 0;
}

static void kv_store_fini(kv_data_store_t *store)
{
  free(store->data);
}

#define TEST_SIZE 512

int kv_init()
{
  if (kv_store_init(&kv_data.store[0], TEST_SIZE) < 0)
    return -1;

  if (kv_store_init(&kv_data.store[1], TEST_SIZE) < 0) {
    kv_store_fini(&kv_data.store[0]);
    return -1;
  }

  kv_data.current = &kv_data.store[0];
  kv_data.copy = &kv_data.store[1];

  kv_data.size = TEST_SIZE;

  return 0;
}
#else
#if 0
static void kv_clear(void *data, unsigned int size)
{
  unsigned int flash_block = 3;

  if (data == (void *)FLASH_SECT1)
    flash_block = 2;

  flash_erase(flash_block, 1);
}
#endif

static int _kv_write_header(kv_data_store_t *sp, unsigned int seq, int init_pos)
{
  kv_store_hdr_t hdr;

  hdr.magic = KV_STORE_HDR_MAGIC;
  hdr.seq = seq;

  flash_program((unsigned int)sp->data, &hdr, sizeof(kv_store_hdr_t));
  if (init_pos)
    sp->pos = sizeof(kv_store_hdr_t);

  return 0;
}

static void _kv_write_ofs(kv_data_store_t *sp, void *data,
                          unsigned int len, unsigned int ofs)
{
  flash_program((unsigned int)sp->data + ofs, data, len);
}

static int _kv_write(kv_data_store_t *sp, kv_hdr_t *rec)
{
  unsigned int rlen = rec->rlen;

  if (kv_data.size < sp->pos + rlen)
    return -1;

  flash_program((unsigned int)sp->data + sp->pos, rec, rlen);
  sp->pos += rlen;

  return 0;
}

#define KV_SEQ_INVALID 0

static unsigned int kv_store_get_seq(kv_data_store_t *store)
{
  kv_store_hdr_t *shdr = (kv_store_hdr_t *)store->data;

  if (shdr->magic != KV_STORE_HDR_MAGIC)
    return KV_SEQ_INVALID;

  return shdr->seq;
}

/* TODO:
   this does not handle partial writing of a record -
   fixed by copying up to the invalid record and erasing
   or
   interruption of record invalidation after a valid write -
   fixed by invalidating the last key at startup */
static int kv_scan(kv_data_store_t *store)
{
  kv_iter_t iter;

  if (kv_iter_start(store, &iter) < 0)
    return -1;

  kv_hdr_t *hdr = kv_iter_next(&iter);

  while (hdr)
    hdr = kv_iter_next(&iter);

  if (kv_iter_status(&iter) == KV_ITER_STATUS_INVALID)
    return -1;

  return kv_iter_get_ofs(&iter);
}

void kv_store_reinit(kv_data_store_t *store)
{
  unsigned int sect;

  switch ((unsigned long)store->data) {
  case FLASH_SECT1:
    sect = 2;
    break;
  case FLASH_SECT2:
    sect = 3;
    break;
  default:
    return;
  }

  flash_erase(sect, 1);
}

#define FLASH_BLOCK_SIZE 0x4000
int kv_store_init(kv_data_store_t *store, void *base)
{
  int ofs;

  store->data = base;
  ofs = kv_scan(store);
  if (ofs < 0)
    return -1;

  store->pos = ofs;

  return 0;
}

void *flash[] = { (void *)FLASH_SECT1, (void *)FLASH_SECT2 };

int kv_init()
{
  kv_data_store_t *store[2];
  int valid[2];
  unsigned int seq[2];
  unsigned int i;

  kv_data.size = FLASH_BLOCK_SIZE;

  for (i = 0; i < 2; i++) {
    store[i] = &kv_data.store[i];
    valid[i] = kv_store_init(store[i], flash[i]);
    seq[i] = kv_store_get_seq(store[i]);
  }

  if (valid[1] < 0 || (seq[1] < seq[0])) {
    kv_data.current = store[0];
    kv_data.copy = store[1];
  } else {
    kv_data.current = store[1];
    kv_data.copy = store[0];
  }

  return 0;
}
#endif

void kv_list()
{
  kv_iter_t iter;

  if (kv_iter_start(kv_data.current, &iter) < 0)
    return;

  for (;;) {
    kv_hdr_t *hdr;
    char *key, *val;
    unsigned int uval;
    int ival;

    hdr = kv_iter_next(&iter);

    if (!hdr)
      break;

    key = (char *)(hdr + 1);

    if (hdr->start == KV_HDR_START) {
      switch (hdr->type) {
      case KV_TYPE_STR:
        val = key + strlen(key) + 1;
        xprintf("%12s = %s\n", key, val);
        break;
      case KV_TYPE_INT:
        memcpy(&ival, key + strlen(key) + 1, sizeof(int));
        xprintf("%12s = %d\n", key, ival);
        break;
      case KV_TYPE_UINT:
        memcpy(&uval, key + strlen(key) + 1, sizeof(unsigned int));
        xprintf("%12s = 0x%08x(%u)\n", key, uval, uval);
        break;
      default:
        break;
      }
    }
  }
}

void kv_dump(int full)
{
  kv_iter_t iter;
  kv_hdr_t *hdr;

  int seq = kv_iter_start(kv_data.current, &iter);

  if (seq < 0)
    return;

  xprintf("DUMP: seq=%d\n", seq);

  for (;;) {
    char *key, *val;
    int ival;
    unsigned int uval;

    hdr = kv_iter_next(&iter);
    if (!hdr)
      break;

    key = (char *)(hdr + 1);

    if (full || hdr->start == KV_HDR_START) {
      switch (hdr->type) {
      case KV_TYPE_STR:
        val = key + strlen(key) + 1;
        xprintf("%02x T:%d L:%d %s=%s\n",
                hdr->start, hdr->type, hdr->rlen, key, val);
        break;
      case KV_TYPE_INT:
        memcpy(&ival, key + strlen(key) + 1, sizeof(int));
        xprintf("%02x T:%d L:%d %s=%d\n",
                hdr->start, hdr->type, hdr->rlen, key, ival);
        break;
      case KV_TYPE_UINT:
        memcpy(&uval, key + strlen(key) + 1, sizeof(unsigned int));
        xprintf("%02x T:%d L:%d %s=0x%08x\n",
                hdr->start, hdr->type, hdr->rlen, key, uval);
        break;
      default:
        break;
      }
    }
  }

  if (kv_iter_status(&iter) == KV_ITER_STATUS_INVALID)
    xprintf("invalid header\n");
}

void kv_copy_valid()
{
  kv_data_store_t *src = kv_data.current;
  kv_data_store_t *dst = kv_data.copy;
  kv_hdr_t *hdr;
  unsigned int lofs;
  unsigned int ofs = sizeof(kv_store_hdr_t);
  kv_store_hdr_t *shdr = (kv_store_hdr_t *)src->data;

  if (shdr->magic != KV_STORE_HDR_MAGIC)
    return;

  hdr = (kv_hdr_t *)(shdr + 1);

  kv_store_reinit(dst);

  /* make space to write header */
  dst->pos = sizeof(kv_store_hdr_t);

  for (;;) {
    if (hdr->start != KV_HDR_START && hdr->start != KV_HDR_DELETED)
      break;

    /* sanity checks here */

    if (hdr->start == KV_HDR_START)
      _kv_write(dst, hdr);

    lofs = hdr->rlen;
    ofs += lofs;
    hdr = (kv_hdr_t *)((unsigned char *)(hdr) + lofs);

    if (ofs >= kv_data.size)
      break;
  }

  /* write header after copy */
  _kv_write_header(dst, shdr->seq + 1, 0);

  kv_data.current = dst;
  kv_data.copy = src;
}

static void kv_invalidate(kv_data_store_t *store, const char *dkey,
                          unsigned int max_ofs)
{
  kv_hdr_t *hdr;
  unsigned int ofs = sizeof(kv_store_hdr_t);
  kv_store_hdr_t *shdr = (kv_store_hdr_t *)store->data;

  if (shdr->magic != KV_STORE_HDR_MAGIC)
    return;

  hdr = (kv_hdr_t *)(shdr + 1);

  for (;;) {
    char *key;
    unsigned int lofs;

    if (ofs >= max_ofs)
      break;

    if (hdr->start != KV_HDR_START && hdr->start != KV_HDR_DELETED)
      break;

    if (hdr->start == KV_HDR_START) {
      key = (char *)(hdr + 1);
      if (!strcmp(key, dkey)) {
#if TEST
        hdr->start = KV_HDR_DELETED;
#else
#if ONLY_REWRITE_ZERO
        char buf[8];
        memset(buf, 0, sizeof(buf));
        _kv_write_ofs(store, buf, sizeof(buf), ofs);
#else
        kv_hdr_t chdr;

        memcpy(&chdr, hdr, sizeof(kv_hdr_t));
        chdr.start = KV_HDR_DELETED;
        /* write to flash */
        _kv_write_ofs(store, &chdr, sizeof(kv_hdr_t), ofs);
#endif
#endif
      }
    }

    lofs = hdr->rlen;
    hdr = (kv_hdr_t *)((unsigned char *)(hdr) + lofs);
    ofs += lofs;
  }
}

static int kv_write(kv_hdr_t *rec)
{
  kv_data_store_t *store = kv_data.current;
  unsigned int ppos = store->pos;

  if (store->pos == 0)
    _kv_write_header(store, 1, 1);

  if (!(_kv_write(store, rec) == 0)) {
    kv_copy_valid();
    /* current has changed - reread */
    store = kv_data.current;
    ppos = store->pos;

    if (_kv_write(store, rec) < 0) {
      xprintf("REALLY NO SPACE");
      return -1;
    }
  }

  kv_invalidate(store, (char *)(rec + 1), ppos);

  return 0;
}

void kv_delete(const char *key)
{
  kv_data_store_t *store = kv_data.current;

  kv_invalidate(store, key, store->pos);
}

static int kv_set_data(const char *key, unsigned int type,
                       const void *vdata, unsigned int vlen)
{
  unsigned int rlen, klen, alen, pad_len;
  kv_hdr_t *hdr;
  unsigned char rec[FLASH_BUF_LEN];
  char *prec;

  klen = strlen(key);

  rlen = sizeof(kv_hdr_t) + klen + vlen + 1;
  alen = KV_ALIGN(rlen);

  if (alen > FLASH_BUF_LEN)
    return -1;

  hdr = (kv_hdr_t *)rec;

  hdr->start = KV_HDR_START;
  hdr->type = type;

  hdr->rlen = alen;

  prec = (char *)(hdr + 1);

  strcpy(prec, key);
  prec += klen + 1;

  memcpy(prec, vdata, vlen);
  prec += vlen;

  pad_len = alen - rlen;

#if 0
  printf("klen %d vlen %d tot %d(%d) pad %d\n",
         klen, vlen, klen + vlen + 2, alen, pad_len);
#endif

  memset(prec, 0, pad_len);

  kv_write(hdr);

  return 0;
}

int kv_set_str(const char *key, const char *val)
{
  unsigned int vlen;

  vlen = strlen(val) + 1;

  return kv_set_data(key, KV_TYPE_STR, val, vlen);
}

int kv_set_uint(const char *key, unsigned int val)
{
  return kv_set_data(key, KV_TYPE_UINT, &val, sizeof(unsigned int));
}

int kv_set_int(const char *key, int val)
{
  return kv_set_data(key, KV_TYPE_INT, &val, sizeof(int));
}

int kv_get_val(const char *skey, int *type, void **val)
{
  kv_iter_t iter;
  const char *key, *cval;
  int len = -1;

  *type = KV_TYPE_INVALID;

  if (kv_iter_start(kv_data.current, &iter) < 0)
    return -1;

  for (;;) {
    kv_hdr_t *hdr = kv_iter_next(&iter);
    if (!hdr)
      break;

    key = (char *)(hdr + 1);
    if (hdr->start == KV_HDR_START && !strcmp(skey, key)) {
      cval = key + strlen(key) + 1;
      switch (hdr->type) {
      case KV_TYPE_STR:
        len = strlen(cval) + 1;
        break;
      case KV_TYPE_UINT:
        len = sizeof(unsigned int);
        break;
      case KV_TYPE_INT:
        len = sizeof(int);
        break;
      default:
        len = -1;
        break;
      }
      *type = hdr->type;
      *val = (void *)(cval);
      break;
    }
  }

  return len;
}

const char *kv_get_str(const char *skey)
{
  const char *val;
  int len, type;

  len = kv_get_val(skey, &type, (void **)&val);
  if (len < 0)
    return NULL;

  if (type != KV_TYPE_STR)
    return NULL;

  return val;
}

unsigned int kv_get_uint(const char *skey)
{
  void *val;
  int len, type;
  unsigned int uval;

  len = kv_get_val(skey, &type, &val);
  if (len != sizeof(unsigned int) || type != KV_TYPE_UINT)
    return 0;

  memcpy(&uval, val, sizeof(unsigned int));

  return uval;
}

unsigned int kv_get_int(const char *skey)
{
  void *val;
  int len, type;
  int ival;

  len = kv_get_val(skey, &type, &val);
  if (len != sizeof(unsigned int) || type != KV_TYPE_INT)
    return 0;

  memcpy(&ival, val, sizeof(int));

  return ival;
}

#if !TEST
int cmd_kv(int argc, char *argv[])
{
  char *key, *value;
  unsigned int uval;
  int ival;
  const char *v;

  if (argc < 2)
    return -1;

  switch (argv[1][0]) {
  case 's':
    if (argc < 4)
      return -1;
    key = argv[2];
    value = argv[3];
    kv_set_str(key, value);
    break;
  case 'u':
    if (argc < 4)
      return -1;
    key = argv[2];
    uval = strtoul(argv[3], NULL, 0);
    kv_set_uint(key, uval);
    break;
  case 'i':
    if (argc < 4)
      return -1;
    key = argv[2];
    ival = strtoul(argv[3], NULL, 0);
    kv_set_int(key, ival);
    break;
  case 'd':
    if (argc < 3)
      return -1;
    key = argv[2];
    kv_delete(key);
    break;
  case 'y':
    kv_dump(0);
    break;
  case 'z':
    kv_dump(1);
    break;
  case 'l':
    kv_list();
    break;
  case 'c':
    kv_copy_valid();
    break;
  case 'r':
    if (argc < 3)
      return -1;
    key = argv[2];

    switch (argv[1][1]) {
    case 's':
    case '\0':
      v = kv_get_str(key);
      if (v)
        xprintf("%s = %s\n", key, v);
      break;
    case 'u':
      uval = kv_get_uint(key);
      xprintf("%s = %08x(%u)\n", key, uval, uval);
      break;
    case 'i':
      ival = kv_get_int(key);
      xprintf("%s = %d\n", key, ival);
      break;
    }
    break;
  case 'f':
    flash_erase(2, 2);
    break;
  }

  return 0;
}

SHELL_CMD(kv, cmd_kv);
#endif

#if TEST
int main()
{
  unsigned int i, j;

  kv_init();

  for (j = 0; j < 72; j++) {
    kv_set_str("secretkey", "ssshh!");

    for (i = 0; i < 22; i++)
      kv_set_str("brian", "murphy");

    kv_set_str("yyy", "hello");
  }

  printf("END\n");
  kv_dump(0);
}
#endif
