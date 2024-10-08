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

#ifndef FAST_LOG_H_
#define FAST_LOG_H_

#ifndef CONFIG_FAST_LOG_ENABLE
#define CONFIG_FAST_LOG_ENABLE 1
#endif

void fast_log(const char *fmt, unsigned long v1, unsigned long v2);

#if CONFIG_FAST_LOG_ENABLE
extern unsigned char fast_log_mask[];
extern char fast_log_enabled;
extern int fast_log_stop_count;

#define FAST_LOG(_m_, fmt, v1, v2)                             \
  do {                                                         \
    if (fast_log_enabled && fast_log_mask[_m_]) {              \
      fast_log(fmt, (unsigned long)(v1), (unsigned long)(v2)); \
    }                                                          \
  } while (0)

static inline void fast_log_enable(int en)
{
  fast_log_enabled = en;
  fast_log_stop_count = -1;
}

void fast_log_dump(unsigned int ent, int debug);

void fast_log_init(const char *enable);

static inline void fast_log_stop(int count)
{
  if (fast_log_stop_count < 0 && count >= 0)
    fast_log_stop_count = count;
}
#else
#define FAST_LOG(_m_, fmt, v1, v2)                             \
  do {                                                         \
    if (0) {                                                   \
      fast_log(fmt, (unsigned long)(v1), (unsigned long)(v2)); \
    }                                                          \
  } while (0)

static inline void fast_log_init(const char *enable)
{
}

static inline void fast_log_dump(unsigned int ent, int debug)
{
}

static inline void fast_log_stop(int count)
{
}
#endif

#endif
