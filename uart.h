#ifndef UART_H
#define UART_H
#include <stdint.h>

/* Verified via: qemu-system-riscv32 -M sifive_u,dumpdtb=u.dtb -bios none
 *               dtc -I dtb -O dts u.dtb | grep -A8 uart
 * DO NOT change these without re-running the dumpdtb check on this machine. */
#define UART0_BASE 0x10010000   /* SiFive UART0 (confirmed) */
#define UART1_BASE 0x10011000   /* SiFive UART1 (confirmed) */

/* Verified via QEMU source: include/hw/char/sifive_uart.h
 * enum { SIFIVE_UART_TXFIFO=0, RXFIFO=4, TXCTRL=8, RXCTRL=12, IE=16, IP=20, DIV=24 } */
#define UART_REG_TXFIFO  0x00
#define UART_REG_RXFIFO  0x04
#define UART_REG_TXCTRL  0x08
#define UART_REG_RXCTRL  0x0C
#define UART_REG_IE      0x10
#define UART_REG_IP      0x14
#define UART_REG_DIV     0x18

#define UART_TXFIFO_FULL  0x80000000u  /* bit 31 of TXFIFO reg on write-side read */
#define UART_RXFIFO_EMPTY 0x80000000u  /* bit 31 of RXFIFO reg */

void uart_init(uint32_t uart_base);
void uart_putc(uint32_t uart_base, char c);
void uart_puts(uint32_t uart_base, const char *s);
char uart_getc(uint32_t uart_base);
int  uart_has_data(uint32_t uart_base);

#endif