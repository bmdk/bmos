#ifndef MSHELL_H
#define MSHELL_H

#include "bmos_msg_queue.h"
#include "hal_uart.h"

void polled_shell(void);
void mshell_add_uart(uart_t *u, unsigned int baud,
                     unsigned int num, unsigned int txop);
void mshell_add_queue(bmos_queue_t *q, unsigned int num, unsigned int txop);
void mshell_init(const char *name, unsigned int dest);
int xgetc(void);
int _xgetc(int timeout);
bmos_queue_t *mshell_queue();

#endif
