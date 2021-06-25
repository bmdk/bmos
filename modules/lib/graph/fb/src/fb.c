#include <common.h>
#include <stdlib.h>
#include <string.h>
#include <xslog.h>
#include <io.h>

#include "fb.h"

struct _fb_t {
  unsigned char stride;
  unsigned char depth;
  unsigned short width;
  unsigned short height;
  unsigned int size;
  unsigned char *fb;
};

fb_t *fb_init(unsigned int width, unsigned int height, unsigned int depth)
{
  unsigned int size, stride;
  fb_t *fb;

  if (depth == 1) {
    stride = 0;
    if ((width & 7) != 0) {
      xslog(LOG_ERR, "invalid width for depth 1\n");
      return 0;
    }
  } else if (depth < 8)
    stride = 1;
  else if (depth < 16)
    stride = 2;
  else
    stride = 4;

  if (stride == 0)
      size = width * height / 8;
  else
      size = width * height * stride;

  fb = malloc(sizeof(fb_t));
  if (fb == 0)
    return 0;

  fb->fb = calloc(1, size);
  if (fb == 0) {
    free(fb);
    return 0;
  }

  fb->size = size;
  fb->width = width;
  fb->height = height;
  fb->stride = stride;
  fb->depth = depth;

  return fb;
}

void *fb_get(fb_t *fb)
{
  return (void *)fb->fb;
}

void fb_clear(fb_t *fb)
{
  memset(fb->fb, 0, fb->size);
}

unsigned int fb_width(fb_t *fb)
{
  return (unsigned int)fb->width;
}

unsigned int fb_height(fb_t *fb)
{
  return (unsigned int)fb->height;
}

void fb_draw(fb_t *fb, int x, int y, unsigned int col)
{
  void *addr;
  unsigned int base, stride, shift = 0;

  if (x < 0 || x >= fb->width)
    return;

  if (y < 0 || y >= fb->height)
    return;

  stride = fb->stride;

  if (stride == 0) {
    base = (y * fb->width + x) / 8;
    shift = x & 7;
  } else {
    base = (y * fb->width + x) * stride;
  }

  addr = &fb->fb[base];

  switch (stride) {
  case 0:
    if (col)
      *(unsigned char *)addr |= BIT(shift);
    else
      *(unsigned char *)addr &= ~BIT(shift);
    break;
  case 1:
    *(unsigned char *)addr = (unsigned char)col;
    break;
  case 2:
    *(unsigned short *)addr = (unsigned short)col;
    break;
  case 4:
  default:
    *(unsigned int *)addr = (unsigned int)col;
    break;
  }
}
