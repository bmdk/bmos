#ifndef KVLOG_H
#define KVLOG_H

int kv_init(void);
int kv_set_str(const char *key, const char *val);

unsigned int kv_get_int(const char *skey);
unsigned int kv_get_uint(const char *skey);
const char *kv_get_str(const char *skey);

#endif
