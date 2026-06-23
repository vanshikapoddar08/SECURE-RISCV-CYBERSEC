#include "packet.h"
#include "uart.h"
#include "time.h"
#include "integrity.h"

static int read_byte_timeout(uint32_t uart_base, uint32_t timeout_ms, uint8_t *out)
{
    int c = uart_getc_timeout(uart_base, timeout_ms);
    if (c == -1)
        return PKT_RECV_TIMEOUT;
    *out = (uint8_t)c;
    return PKT_RECV_OK;
}

int packet_resync(uint32_t uart_base, uint32_t timeout_ms)
{
    uint64_t start = get_mtime();
    uint64_t limit = (uint64_t)timeout_ms * 10000ULL;

    while ((get_mtime() - start) < limit) {
        if (!uart_has_data(uart_base))
            continue;
        if ((uint8_t)uart_getc(uart_base) == MAGIC_BYTE)
            return PKT_RECV_OK;
    }

    return PKT_RECV_TIMEOUT;
}

static int read_packet_body(Packet *p, uint32_t uart_base,
                            uint32_t timeout_ms, uint8_t my_id)
{
    if (read_byte_timeout(uart_base, timeout_ms, &p->sender_id) != PKT_RECV_OK)
        return PKT_RECV_TIMEOUT;

    if (read_byte_timeout(uart_base, timeout_ms, &p->type) != PKT_RECV_OK)
        return PKT_RECV_TIMEOUT;

    if (read_byte_timeout(uart_base, timeout_ms, &p->receiver_id) != PKT_RECV_OK)
        return PKT_RECV_TIMEOUT;

    if (read_byte_timeout(uart_base, timeout_ms, &p->length) != PKT_RECV_OK)
        return PKT_RECV_TIMEOUT;

    for (int i = 0; i < p->length; i++) {
        if (read_byte_timeout(uart_base, timeout_ms, &p->payload[i]) != PKT_RECV_OK)
            return PKT_RECV_TIMEOUT;
    }

    if (read_byte_timeout(uart_base, timeout_ms, &p->checksum) != PKT_RECV_OK)
        return PKT_RECV_TIMEOUT;

    if (read_byte_timeout(uart_base, timeout_ms, &p->stop_byte) != PKT_RECV_OK)
        return PKT_RECV_TIMEOUT;

    if (p->stop_byte != PKT_DONE)
        return PKT_RECV_BAD_FRAME;

    if (p->receiver_id != my_id)
        return PKT_RECV_WRONG_ID;

    if (!verify_integrity(p))
        return PKT_RECV_BAD_FRAME;

    return PKT_RECV_OK;
}

void packet_send(Packet *p, uint32_t uart_base)
{
    uart_putc(uart_base, p->magic);
    uart_putc(uart_base, p->sender_id);
    uart_putc(uart_base, p->type);
    uart_putc(uart_base, p->receiver_id);
    uart_putc(uart_base, p->length);

    for (int i = 0; i < p->length; i++)
        uart_putc(uart_base, p->payload[i]);

    uart_putc(uart_base, p->checksum);
    uart_putc(uart_base, p->stop_byte);
}

void packet_copy(Packet *dst, const Packet *src)
{
    int i;

    dst->magic       = src->magic;
    dst->sender_id   = src->sender_id;
    dst->type        = src->type;
    dst->receiver_id = src->receiver_id;
    dst->length      = src->length;
    for (i = 0; i < src->length; i++)
        dst->payload[i] = src->payload[i];
    dst->checksum  = src->checksum;
    dst->stop_byte = src->stop_byte;
}

int packet_receive(Packet *p, uint32_t uart_base, uint8_t myid)
{
    p->magic = uart_getc(uart_base);
    if (p->magic != MAGIC_BYTE)
        return PKT_RECV_BAD_FRAME;

    p->sender_id = uart_getc(uart_base);
    p->type = uart_getc(uart_base);
    p->receiver_id = uart_getc(uart_base);
    if (p->receiver_id != myid)
        return PKT_RECV_WRONG_ID;

    p->length = uart_getc(uart_base);
    for (int i = 0; i < p->length; i++)
        p->payload[i] = uart_getc(uart_base);

    p->checksum = uart_getc(uart_base);
    p->stop_byte = uart_getc(uart_base);

    if (!verify_integrity(p))
        return PKT_RECV_BAD_FRAME;

    return PKT_RECV_OK;
}

void payload_print(Packet *p, RingBuffer *buf)
{
    uint8_t B_in_buf = buffer_available(buf);
    if (B_in_buf == 0) {
        p->length = 0;
        p->payload[0] = '\0';
        return;
    }

    if (B_in_buf > 16)
        p->length = 16;
    else
        p->length = B_in_buf;

    for (uint8_t i = 0; i < p->length; i++) {
        uint8_t transfer_char;
        buffer_read(buf, &transfer_char);
        p->payload[i] = transfer_char;
    }
}

int packet_receive_with_timeout(Packet *p, uint32_t uart_base,
                                uint32_t timeout_ms, uint8_t my_id)
{
    int temp_char;

    temp_char = uart_getc_timeout(uart_base, timeout_ms);
    if (temp_char == -1)
        return PKT_RECV_TIMEOUT;

    p->magic = (uint8_t)temp_char;
    if (p->magic != MAGIC_BYTE) {
        packet_resync(uart_base, timeout_ms);
        return PKT_RECV_BAD_FRAME;
    }

    return read_packet_body(p, uart_base, timeout_ms, my_id);
}
