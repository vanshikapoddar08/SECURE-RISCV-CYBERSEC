#include "uart.h"


void uart_init(uint32_t uart_base) {
    volatile uint32_t *txctrl = (volatile uint32_t *)(uart_base + UART_REG_TXCTRL);
    volatile uint32_t *rxctrl = (volatile uint32_t *)(uart_base + UART_REG_RXCTRL);
    volatile uint32_t *ie     = (volatile uint32_t *)(uart_base + UART_REG_IE);
    volatile uint32_t *div    = (volatile uint32_t *)(uart_base + UART_REG_DIV);

  
    *div    = 0;
    *ie     = 0;     /* disable interrupts -- we're polling, not using PLIC */
    *txctrl = 0x01;   /* bit0 = txen */
    *rxctrl = 0x01;   /* bit0 = rxen */
}

void uart_putc(uint32_t uart_base, char c) {
    volatile uint32_t *txfifo = (volatile uint32_t *)(uart_base + UART_REG_TXFIFO);
    while (*txfifo & UART_TXFIFO_FULL) { }
    *txfifo = (uint32_t)(unsigned char)c;
}

void uart_puts(uint32_t uart_base, const char *s) {
    while (*s) {
        uart_putc(uart_base, *s++);
    }
}


static int has_cached_byte[2] = {0, 0};
static uint8_t cached_byte[2] = {0, 0};

static int uart_index(uint32_t uart_base) {
    return (uart_base == UART1_BASE) ? 1 : 0;
}

int uart_has_data(uint32_t uart_base) {
    int idx = uart_index(uart_base);
    if (has_cached_byte[idx]) {
        return 1;
    }
    volatile uint32_t *rxfifo = (volatile uint32_t *)(uart_base + UART_REG_RXFIFO);
    uint32_t val = *rxfifo;          /* this DOES pop a byte if one is present */
    if (val & UART_RXFIFO_EMPTY) {
        return 0;                     
    }
   
    cached_byte[idx] = (uint8_t)(val & 0xFF);
    has_cached_byte[idx] = 1;
    return 1;
}

char uart_getc(uint32_t uart_base) {
    int idx = uart_index(uart_base);
    if (has_cached_byte[idx]) {
        has_cached_byte[idx] = 0;
        return (char)cached_byte[idx];
    }
    volatile uint32_t *rxfifo = (volatile uint32_t *)(uart_base + UART_REG_RXFIFO);
    uint32_t val;
    do {
        val = *rxfifo;
    } while (val & UART_RXFIFO_EMPTY);
    return (char)(val & 0xFF);
}
