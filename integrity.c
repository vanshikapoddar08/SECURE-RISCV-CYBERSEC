#include "integrity.h"

uint8_t calculate_integrity(Packet *p) {
    // Current method: Simple Checksum
    uint8_t sum = 0;
    sum += p->magic;
    sum += p->sender_id;     // ADD — was missing
    sum += p->type;
    sum += p->receiver_id;   // ADD — was missing
    sum += p->length;

    
    // Only calculate based on the actual payload length to avoid garbage memory!
    for (int i = 0; i < p->length; i++) {
        sum += p->payload[i];
    }
    
    return sum;
}

int verify_integrity(Packet *p) {
    // Generate what the checksum SHOULD be
    uint8_t expected_hash = calculate_integrity(p);
    
    // Compare it to what actually arrived over the wire
    if (p->checksum == expected_hash) {
        return 1; // Valid!
    } else {
        return 0; // Corrupted!
    }
}


