#include "buffer.h"
#include "uart.h"
// setting up the buffer
void buffer_init(RingBuffer *buf) {
    buf->head = 0;
    buf->tail = 0;
}

// writing to the buffer
uint8_t buffer_write(RingBuffer *buf, uint8_t c) {
    uint8_t next_head = (buf->head + 1);
    if (next_head == buf->tail) {
        return -1; // Buffer is full
    }
    buf->data[buf->head] = c;
    buf->head = next_head;
    return 0; // Success
}

// reading from the buffer
uint8_t buffer_read(RingBuffer *buf, uint8_t *c) {
    if (buf->head == buf->tail) {
        return -1; // Buffer is empty
    }
    *c = buf->data[buf->tail];
    buf->tail = (buf->tail + 1);
    return 0; // Success
}

void buffer_flush(RingBuffer *buf) {
    buf->head = 0;
    buf->tail = 0;
}

uint8_t buffer_available(RingBuffer *buf) {
    if (buf->head >= buf->tail) {
        return buf->head - buf->tail;
    } 
    else {
        return (BUFFER_SIZE - buf->tail) + buf->head;
    }
}



char reciving_from_python(RingBuffer *buf,uint32_t uart_base) {
    while (uart_has_data(uart_base)) {
        uint8_t c = uart_getc(uart_base);
        if (buffer_write(buf, c) == -1) {
            // Buffer is full, you might want to handle this case
            return -1; // Indicate buffer overflow
        }
    }
    return 0; //done


}
