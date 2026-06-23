#include "time.h"
#include "uart.h"

#define CLINT_BASE 0x02000000L
#define MTIME_ADDR (CLINT_BASE + 0xBFF8)

// Safely read the 64-bit mtime register on a 32-bit CPU
uint64_t get_mtime(void) 
{
    volatile uint32_t *mtime_lo = (volatile uint32_t *)(MTIME_ADDR);
    volatile uint32_t *mtime_hi = (volatile uint32_t *)(MTIME_ADDR + 4);
    uint32_t lo, hi;

    // Loop until the high bits don't change during the read
    do {
        hi = *mtime_hi;
        lo = *mtime_lo;
    } while (hi != *mtime_hi);

    // Shift the high 32 bits over and combine with the low 32 bits
    return (((uint64_t)hi) << 32) | lo;
}

int uart_getc_timeout(uint32_t uart_base, uint32_t timeout_ms) {
    // 10 MHz clock = 10,000 ticks per millisecond
    uint64_t timeout_ticks = (uint64_t)timeout_ms * 10000;
    uint64_t start_time = get_mtime();
    
    while (!uart_has_data(uart_base)) {
        // Check if current time minus start time exceeds the threshold
        if ((get_mtime() - start_time) >= timeout_ticks) {
            return -1; // Indicate timeout occurred
        }
    }
    
    // Data is ready
    return uart_getc(uart_base);
}