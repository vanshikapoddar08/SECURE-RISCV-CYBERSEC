#ifndef TIME_H
#define TIME_H

#include <stdint.h>

uint64_t get_mtime(void);
int uart_getc_timeout(uint32_t uart_base, uint32_t timeout_ms);
#endif // TIME_H
