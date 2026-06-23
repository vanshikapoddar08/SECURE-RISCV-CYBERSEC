#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

// Define the size of the buffer. 256 is highly efficient for 8-bit microcontrollers.
#define BUFFER_SIZE 256

// The Blueprint for a Buffer Object
typedef struct {
    uint8_t data[BUFFER_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
} RingBuffer;

// Function Prototypes - Notice every function requires a pointer to a specific RingBuffer
void buffer_init(RingBuffer *buf);
uint8_t buffer_write(RingBuffer *buf, uint8_t c);
uint8_t buffer_read(RingBuffer *buf, uint8_t *c);
uint8_t buffer_available(RingBuffer *buf);
void buffer_flush(RingBuffer *buf);
char reciving_from_python(RingBuffer *buf,uint32_t uart_base);
#endif