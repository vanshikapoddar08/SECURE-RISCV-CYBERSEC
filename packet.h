#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include "buffer.h"

#define MAGIC_BYTE 0xAB

#define PKT_SYN        0x01
#define PKT_SYN_ACK    0x02
#define PKT_ACK        0x03
#define PKT_DATA       0x04
#define PKT_DATA_ACK   0x05
#define PKT_DONE       0xd0

#define PKT_RECV_OK        0
#define PKT_RECV_TIMEOUT  -1
#define PKT_RECV_BAD_FRAME -2
#define PKT_RECV_WRONG_ID  -3

typedef struct
{
    uint8_t magic;
    uint8_t sender_id;
    uint8_t type;
    uint8_t receiver_id;
    uint8_t length;
    uint8_t payload[16];
    uint8_t checksum;
    uint8_t stop_byte;

} Packet;

void packet_send(Packet *p, uint32_t uart_base);
void packet_copy(Packet *dst, const Packet *src);
int packet_receive(Packet *p, uint32_t uart_base, uint8_t myid);
void payload_print(Packet *p, RingBuffer *buf);
int packet_receive_with_timeout(Packet *p, uint32_t uart_base,
                                uint32_t timeout_ms, uint8_t my_id);
int packet_resync(uint32_t uart_base, uint32_t timeout_ms);

typedef enum {
    STATE_DISCONNECTED,
    STATE_WAITING_SYN_ACK,
    STATE_CONNECTED,
    STATE_WAITING_DATA_ACK
} ConnectionState;

#endif
