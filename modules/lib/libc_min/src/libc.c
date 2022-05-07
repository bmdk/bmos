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

#include <stdarg.h>
#include <stdlib.h>
#include "io.h"

typedef unsigned int size_t;

int vsnprintf(char *str, size_t size, const char *format, va_list ap);
int snprintf(char *str, size_t size, const char *format, ...);
unsigned long int strtoul(const char *nptr, char **endptr, int base);
int sscanf(const char *str, const char *format, ...);
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strchr(const char *s, int c);
int memcmp(const void *_s1, const void *_s2, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *p, int v, size_t s);
int atoi(const char *nptr);
#if 0
void *malloc(size_t size);
void free(void *ptr);
#endif

#if 0
int __errno;
#endif

unsigned long int strtoul(const char *nptr, char **endptr, int base)
{
  unsigned long val = 0;
  char c;

  for (;;) {
    c = *nptr++;
    if (!(c == ' ' || c == '\t'))
      break;
  }

  if (base == 0) {
    if (c == '0') {
      c = *nptr++;
      if (c == 'x') {
        base = 16;
        c = *nptr++;
      } else
        base = 8;
    } else
      base = 10;
  } else if (base == 16 && c == '0') {
    c = *nptr++;
    c |= 0x20;
    if (c == 'x')
      c = *nptr++;
    else if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')))
      goto end;
  }
  for (;;) {
    switch (base) {
    case 16:
      c |= 0x20;
      if (c >= '0' && c <= '9')
        val = (val << 4) + c - '0';
      else if (c >= 'a' && c <= 'f')
        val = (val << 4) + c - 'a' + 10;
      else
        goto end;
      break;
    case 10:
      if (c >= '0' && c <= '9')
        val = val * 10 + c - '0';
      else
        goto end;
      break;
    case 8:
      if (c >= '0' && c <= '7')
        val = (val << 3) + c - '0';
      else
        goto end;
      break;
    }
    c = *nptr++;
  }

end:
  if (endptr)
    *endptr = (char *)nptr;
  return val;
}

unsigned int scani(const char *ptr, unsigned int max_len, int *val)
{
  unsigned int v = 0, i;
  char c;

  if (max_len < 0)
    max_len = 10;

  for (i = 0; i < max_len; i++) {
    c = *ptr++;
    if (!(c >= '0' && c <= '9'))
      break;
    v = v * 10 + c - '0';
  }

  *val = v;
  return i;
}

unsigned int scanx(const char *ptr, unsigned int max_len, unsigned int *val)
{
  unsigned int v = 0, i;
  char c;

  if (max_len < 0)
    max_len = 8;

  for (i = 0; i < max_len; i++) {
    c = 0x20 | *ptr++;
    if (c >= '0' && c <= '9')
      v = (v << 4) + c - '0';
    else if (c >= 'a' && c <= 'f')
      v = (v << 4) + c - 'a' + 10;
    else
      break;
  }

  *val = v;
  return i;
}

size_t strlen(const char *s)
{
  size_t count = 0;

  while (*s++)
    count++;

  return count;
}

int memcmp(const void *_s1, const void *_s2, size_t n)
{
  const unsigned char *s1, *s2;
  size_t i;

  s1 = _s1;
  s2 = _s2;

  for (i = 0; i < n; i++) {
    unsigned char c1 = *s1++;
    unsigned char c2 = *s2++;
    if (c1 < c2)
      return -1;
    else if (c1 > c2)
      return 1;
  }

  return 0;
}

void *memcpy(void *dest, const void *src, size_t n)
{
  unsigned int i;
  unsigned char *d = dest;
  const unsigned char *s = src;

  for (i = 0; i < n; i++)
    *d++ = *s++;

  return dest;
}

void *memset(void *p, int v, size_t s)
{
  unsigned char *c = p;
  size_t i;

  for (i = 0; i < s; i++)
    *c++ = (unsigned char)v;

  return p;
}

char *strcpy(char *dest, const char *src)
{
  char *d = dest;
  const char *s = src;

  for (;;) {
    char v = *s++;
    *d++ = v;
    if (!v)
      break;
  }

  return dest;
}

int atoi(const char *nptr)
{
  int val = 0, neg = 0;
  char c;

  for (;;) {
    c = *nptr++;
    if (c == ' ' || c == '\t')
      continue;
    break;
  }

  if (c == '-') {
    neg = 1;
    c = *nptr++;
  }

  for (;;) {
    if (c >= '0' && c <= '9')
      val = val * 10 + (c - '0');
    else
      break;
    c = *nptr++;
  }

  if (neg)
    return -val;

  return val;
}

int strcmp(const char *s1, const char *s2)
{
  for (;;) {
    unsigned char c1 = *(unsigned char *)s1++;
    unsigned char c2 = *(unsigned char *)s2++;
    int diff = c1 - c2;
    if (diff != 0)
      return diff;
    else if (c1 == 0)
      return 0;
  }
}

int strncmp(const char *s1, const char *s2, size_t n)
{
  while (n--) {
    unsigned char c1 = *(unsigned char *)s1++;
    unsigned char c2 = *(unsigned char *)s2++;
    int diff = c1 - c2;
    if (diff != 0)
      return diff;
    else if (c1 == 0)
      return 0;
  }

  return 0;
}

char *strchr(const char *s, int c)
{
  for (;;) {
    char cu = *s;
    if (!cu)
      return 0;
    if (cu == c)
      return (char *)s;
    s++;
  }
}

#if 0
void *malloc(size_t size)
{
  return _sbrk(size);
}

void free(void *ptr)
{
  xprintf("leak %p from %p\n", ptr, __builtin_return_address(0));
  /* leak */
}
#endif

#define OUT(c) \
  do { \
    if (count >= size - 1) \
    goto end; \
    *str++ = (c); \
    count++; \
  } while (0)

#define INS(c) \
  do { \
    int _i; \
    if (count >= size - 1) \
    goto end; \
    for (_i = count + 1; _i >= 1; _i--) \
    str[_i] = str[_i - 1]; \
    *str = (c); \
    count++; \
  } while (0)

static int printx(char *str, size_t size, int width,
                  int zero, unsigned int val)
{
  unsigned int count = 0;

  for (;;) {
    char c = val & 0xf;
    if (c >= 0 && c <= 9)
      INS(c + '0');
    else
      INS(c + 'a' - 10);
    val >>= 4;
    if (val == 0)
      break;
  }

  if (width > 0) {
    int i;
    char padchr = ' ';
    if (zero)
      padchr = '0';
    width -= count;
    for (i = 0; i < width; i++)
      INS(padchr);
  }

end:
  str[count] = '\0';
  return count;
}

static int printu(char *str, size_t size, int width,
                  int zero, unsigned int val)
{
  unsigned int count = 0;

  for (;;) {
    char c = (val % 10) + '0';
    INS(c);
    val = val / 10;
    if (val == 0)
      break;
  }

  if (width > 0) {
    int i;
    char padchr = ' ';
    if (zero)
      padchr = '0';
    width -= count;
    for (i = 0; i < width; i++)
      INS(padchr);
  }

end:
  str[count] = '\0';
  return count;
}

static int printi(char *str, size_t size, int width, int zero, int val)
{
  unsigned int count = 0;

  if (val < 0) {
    OUT('-');
    val = -val;
  }

  for (;;) {
    char c = (val % 10) + '0';
    INS(c);
    val = val / 10;
    if (val == 0)
      break;
  }

  if (width > 0) {
    int i;
    char padchr = ' ';
    if (zero)
      padchr = '0';
    width -= count;
    for (i = 0; i < width; i++)
      INS(padchr);
  }

end:
  str[count] = '\0';
  return count;
}

#undef RESET
#define RESET() do { fmt = 0; zero = 0; width = -1; lalign = 0; } while (0)

#define SPACES(cnt) do { int i; for (i = 0; i < (cnt); i++) OUT(' '); \
} while (0)

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
  int fmt, zero, width, len, int_arg, lalign;
  unsigned int count = 0, uint_arg;
  unsigned char ch_arg;
  char *s_arg;
  char c;

  if (size <= 0)
    return 0;

  RESET();

  for (;;) {
    c = *format++;

    if (fmt) {
      switch (c) {
      case '%':
        OUT('%');
        RESET();
        break;
      case 'p':
        OUT('0');
        OUT('x');
        if (width < 0)
          width = 8;
        zero = 1;
      case 'X':
      case 'x':
        uint_arg = va_arg(ap, unsigned int);
        len = printx(str, size - count, width, zero, uint_arg);
        str += len;
        count += len;
        RESET();
        break;
      case 'd':
      case 'i':
        int_arg = va_arg(ap, int);
        len = printi(str, size - count, width, zero, int_arg);
        str += len;
        count += len;
        RESET();
        break;
      case 'u':
        int_arg = va_arg(ap, unsigned int);
        len = printu(str, size - count, width, zero, int_arg);
        str += len;
        count += len;
        RESET();
        break;
      case 'c':
        ch_arg = (unsigned char)va_arg(ap, int);
        OUT(ch_arg);
        RESET();
        break;
      case 's':
        s_arg = va_arg(ap, char *);
        len = strlen(s_arg);
        if (!lalign)
          SPACES(width - len);
        while (*s_arg)
          OUT(*s_arg++);
        if (lalign)
          SPACES(width - len);
        RESET();
        break;
      case '-':
        lalign = 1;
        break;
      case '0':
        if (width < 0) {
          zero = 1;
          width = 0;
          break;
        }
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        if (width < 0)
          width = 0;
        width = width * 10 + c - '0';
        break;
      default:
        RESET();
      }
    } else {
      switch (c) {
      case '\0':
        goto end;
      case '%':
        fmt = 1;
        break;
      default:
        OUT(c);
      }
    }
  }

end:
  *str = '\0';
  return count;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
  va_list ap;
  int ret;

  va_start(ap, format);
  ret = vsnprintf(str, size, format, ap);
  va_end(ap);

  return ret;
}

#undef RESET
#define RESET() do { fmt = 0; width = -1; } while (0)

int vsscanf(const char *str, const char *format, va_list ap)
{
  int f, c, fmt, width, count = 0, l;
  int *ival;
  unsigned int *uival;

  RESET();

  for (;;) {
    f = *format++;
    if (!f)
      break;
    if (fmt) {
      switch (f) {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        if (width < 0)
          width = 0;
        width = width * 10 + f - '0';
        break;
      case 'd':
        ival =  va_arg(ap, int *);
        l = scani(str, width, ival);
        if (l == 0)
          goto end;
        str += l;
        count++;
        RESET();
        break;
      case 'x':
        uival =  va_arg(ap, unsigned int *);
        l = scanx(str, width, uival);
        if (l == 0)
          goto end;
        str += l;
        count++;
        RESET();
        break;
      default:
        RESET();
      }
    } else {
      switch (f) {
      case '%':
        fmt = 1;
        break;
      default:
        c = *str++;
        if (f != c)
          goto end;
      }
    }
  }

end:
  return count;
}

int sscanf(const char *str, const char *format, ...)
{
  va_list ap;
  int count;

  va_start(ap, format);
  count = vsscanf(str, format, ap);
  va_end(ap);

  return count;
}

void *calloc(size_t nmemb, size_t size)
{
  size_t s;
  void *p;

  s = nmemb * size;

  p = malloc(s);

  if (p)
    memset(p, 0, s);

  return p;
}

void *memmove(void *dest, const void *src, size_t n)
{
  unsigned int i;
  unsigned char *d = dest;
  const unsigned char *s = src;

  for (i = 0; i < n; i++)
    *d++ = *s++;

  return dest;
}
