#ifndef MSHELL_H
#define MSHELL_H

void polled_shell(void);
void mshell_add_uart(uart_t *u, unsigned int baud,
                     unsigned int num, unsigned int txop);
void mshell_init(const char *name, unsigned int dest);
int _xgetc(int timeout);

#endif
