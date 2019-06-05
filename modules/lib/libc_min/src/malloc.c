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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "io.h"
#include "shell.h"

typedef struct _malloc_hdr_t malloc_hdr_t;

struct _malloc_hdr_t {
  malloc_hdr_t *next;
  unsigned int size;
};

#define CHUNK_POW2 8
#define CHUNK_SIZE sizeof(malloc_hdr_t)

#define DEBUG 0

typedef struct {
  malloc_hdr_t *free_list;
#if DEBUG
  unsigned int sbrk_count;
  unsigned int count;
  unsigned int free_count;
#endif
} malloc_data_t;

static malloc_data_t malloc_data;

static void _add(malloc_hdr_t *r)
{
  malloc_hdr_t *p, *prev = NULL;

  for (p = malloc_data.free_list; p; prev = p, p = p->next)
    if ((prev < r) && (r < p))
      break;

  if (r + r->size == p) {
    r->size += p->size;
    p = p->next;
  }

  r->next = p;

  if (!prev)
    malloc_data.free_list = r;
  else if (prev + prev->size == r) {
    prev->next = r->next;
    prev->size += r->size;
  } else
    prev->next = r;
}

void *malloc(size_t nbytes)
{
  malloc_hdr_t *p, *next, *prev;
  unsigned int nunits, aunits;

  if (nbytes == 0)
    return NULL;

  nunits = (nbytes + CHUNK_SIZE - 1) / CHUNK_SIZE + 1;

restart:
  prev = NULL;
  for (p = malloc_data.free_list; p; prev = p, p = p->next)
    /* don't leave a useless one unit free */
    if (p->size > nunits + 1 || p->size == nunits)
      break;

  if (p == NULL) {
    aunits = ALIGN(nunits, CHUNK_POW2);
    p = sbrk(aunits * CHUNK_SIZE);
#if DEBUG
    malloc_data.sbrk_count += aunits;
#endif
    if (p == (void *)-1)
      return NULL;
    p->next = NULL;
    p->size = aunits;
    _add(p);
    goto restart;
  }

  if (p->size == nunits)
    next = p->next;
  else {
    next = p + nunits;
    next->size = p->size - nunits;
    next->next = p->next;
    p->size = nunits;
  }

  if (prev)
    prev->next = next;
  else
    malloc_data.free_list = next;

#if DEBUG
  malloc_data.count += nunits;
#endif

  p->next = 0;

  return (void *)(p + 1);
}

void free(void *ap)
{
  malloc_hdr_t *r;

  if (!ap)
    return;

  r = (malloc_hdr_t *)ap - 1;

#if DEBUG
  malloc_data.free_count += r->size;
#endif

  _add(r);
}

void *realloc(void *ptr, size_t size)
{
  malloc_hdr_t *bp;
  void *nptr;
  size_t sz;

  if (!ptr)
    return malloc(size);

  bp = (malloc_hdr_t*)ptr - 1;
  sz = CHUNK_SIZE * bp->size;

  if (sz > size)
    return ptr;

  nptr = malloc(size);
  if (!nptr)
    return NULL;

  memcpy(nptr, ptr, sz);
  free(ptr);

  return nptr;
}

#if DEBUG
static int cmd_malloc(int argc, char *argv[])
{
  malloc_hdr_t *p;
  unsigned int tot_free = 0;

  xprintf("free list:\n");

  for (p = malloc_data.free_list; p; p = p->next) {
    xprintf("s %p e %p next %p s %d\n", p, p + p->size, p->next, p->size);
    tot_free += p->size;
  }

  xprintf("\n");
  xprintf("count %d(%d)\n", malloc_data.count, CHUNK_SIZE * malloc_data.count);
  xprintf("sbrk  %d(%d)\n", malloc_data.sbrk_count,
          CHUNK_SIZE * malloc_data.sbrk_count);
  xprintf("freed %d(%d)\n", malloc_data.free_count,
          CHUNK_SIZE * malloc_data.free_count);
  xprintf("free  %d(%d)\n", tot_free, CHUNK_SIZE * tot_free);

  xprintf("\n");
  xprintf("chunk size %d\n", CHUNK_SIZE);

  return 0;
}

SHELL_CMD(malloc, cmd_malloc);
#endif
