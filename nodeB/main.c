#include "../uart.h"
#include "../packet.h"
#include "../buffer.h"
#include "../time.h"
#include "../integrity.h"

#define HANDSHAKE_TIMEOUT_MS 1000  // defining constanst for better readablilty of code
#define LISTEN_TIMEOUT_MS    2000
#define CONNECTED_POLL_MS    50
#define ACK_TIMEOUT_MS       1000
#define MAX_ACK_RETRIES      3

static void send_ctrl(Packet *tx, uint32_t uart, uint8_t type)
{
    tx->type      = type;
    tx->length    = 0;
    tx->stop_byte = PKT_DONE;
    tx->checksum  = calculate_integrity(tx);
    packet_send(tx, uart);
}

static int handle_unexpected_syn(Packet *rx, Packet *tx, uint32_t uart)
{
    if (rx->type != PKT_SYN)
        return 0;
    //uart_puts(UART0_BASE, "\n[RESYNC] SYN received, sending SYN-ACK\n");
    send_ctrl(tx, uart, PKT_SYN_ACK);
    return 1;
}

static void handle_incoming_data(Packet *rx, Packet *tx, uint32_t peer_uart,uint8_t my_id, uint32_t python_uart)
{
    if (rx->type != PKT_DATA || rx->receiver_id != my_id)
        return;

    for (int i = 0; i < rx->length; i++)
        uart_putc(python_uart, rx->payload[i]);
    //uart_putc(python_uart, '\n');
    send_ctrl(tx, peer_uart, PKT_DATA_ACK);
}

static int try_send_from_python(Packet *tx, Packet *pending_tx, RingBuffer *buf,
                                uint32_t peer_uart)
{
    if (buffer_available(buf) == 0 && !uart_has_data(UART0_BASE))
        return 0;

    if (reciving_from_python(buf, UART0_BASE) == (char)-1) {
        uart_puts(UART0_BASE, "[B] buffer overflow, input dropped\n");
        return 0;
    }

    tx->type = PKT_DATA;
    payload_print(tx, buf);

    //if (tx->length > 0 && tx->payload[tx->length - 1] == '\n')
    //    tx->length--;

    if (tx->length == 0)
        return 0;

    tx->stop_byte = PKT_DONE;
    tx->checksum  = calculate_integrity(tx);
    packet_copy(pending_tx, tx);
    packet_send(tx, peer_uart);
    return 1;
}

int main(void)
{
    uart_init(UART0_BASE);
    uart_init(UART1_BASE);

    uart_puts(UART0_BASE, "\n[Node B] boot ok\n");

    ConnectionState state = STATE_DISCONNECTED;
    uint8_t my_id   = 0xa2;
    uint8_t peer_id = 0xa1;
    uint8_t cheker  = 0;

    Packet tx_packet  = {0};
    Packet rx_packet  = {0};
    Packet pending_tx = {0};  // A cache packetfor resending in case of "STATE_WAITING_DATA_ACK" timeout problem

    tx_packet.magic       = MAGIC_BYTE;
    tx_packet.sender_id   = my_id;
    tx_packet.receiver_id = peer_id;

    RingBuffer uart0_buf;
    buffer_init(&uart0_buf);

    while (1)
    {
        switch (state)
        {
        case STATE_DISCONNECTED:
            //uart_puts(UART0_BASE, "\n[B] waiting for SYN\n");

            if (packet_receive_with_timeout(&rx_packet, UART1_BASE,LISTEN_TIMEOUT_MS, my_id) == PKT_RECV_OK)
            {
                if (rx_packet.type == PKT_SYN)
                {
                    send_ctrl(&tx_packet, UART1_BASE, PKT_SYN_ACK);
                    //uart_puts(UART0_BASE, "\n[B] SYN received, SYN-ACK sent\n");
                    state = STATE_WAITING_SYN_ACK;
                }
            }
            break;

        case STATE_WAITING_SYN_ACK:
            if (packet_receive_with_timeout(&rx_packet, UART1_BASE,HANDSHAKE_TIMEOUT_MS, my_id) == PKT_RECV_OK)
            {
                if (handle_unexpected_syn(&rx_packet, &tx_packet, UART1_BASE))
                    break; 

                if (rx_packet.type == PKT_ACK)
                {
                    uart_puts(UART0_BASE, "\n[B] connected to Node A\n");
                    state  = STATE_CONNECTED;
                    cheker = 0;
                }
            }
            else
            {
                uart_puts(UART0_BASE, "\n[B] ACK timeout, back to listening\n");
                state = STATE_DISCONNECTED;
            }
            break;

        case STATE_CONNECTED:
            if (try_send_from_python(&tx_packet, &pending_tx, &uart0_buf, UART1_BASE))
            {
                state = STATE_WAITING_DATA_ACK;
                cheker = 0;
                break;
            }

            if (packet_receive_with_timeout(&rx_packet, UART1_BASE,CONNECTED_POLL_MS, my_id) == PKT_RECV_OK)
            {
                if (handle_unexpected_syn(&rx_packet, &tx_packet, UART1_BASE))
                {
                    state = STATE_WAITING_SYN_ACK;
                    break;
                }

                handle_incoming_data(&rx_packet, &tx_packet, UART1_BASE,
                                     my_id, UART0_BASE);
            }
            break;

        case STATE_WAITING_DATA_ACK:
            if (packet_receive_with_timeout(&rx_packet, UART1_BASE,
                                            ACK_TIMEOUT_MS, my_id) == PKT_RECV_OK)
            {
                if (handle_unexpected_syn(&rx_packet, &tx_packet, UART1_BASE))
                {
                    state = STATE_WAITING_SYN_ACK;
                    break;
                }

                if (rx_packet.type == PKT_DATA)
                {
                    handle_incoming_data(&rx_packet, &tx_packet, UART1_BASE,
                                         my_id, UART0_BASE);
                }
                else if (rx_packet.type == PKT_DATA_ACK)
                {
                    state  = STATE_CONNECTED;
                    cheker = 0;
                }
            }
            else
            {
                cheker++;
                //uart_puts(UART0_BASE, "[B] ACK timeout, retransmitting\n");
                if (cheker >= MAX_ACK_RETRIES)
                {
                    //uart_puts(UART0_BASE, "\n[B] giving up, back to listening\n");
                    state  = STATE_DISCONNECTED;
                    cheker = 0;
                }
                else
                {
                    packet_send(&pending_tx, UART1_BASE);
                }
            }
            break;
        }
    }
}
