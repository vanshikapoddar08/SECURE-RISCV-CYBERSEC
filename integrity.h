#ifndef INTEGRITY_H
#define INTEGRITY_H

#include <stdint.h>
#include "packet.h" // Needed so we know what a Packet looks like

// Generates the integrity value for a packet
uint8_t calculate_integrity(Packet *p);

// Verifies if the packet's integrity value matches its contents
// Returns 1 if valid, 0 if corrupted
int verify_integrity(Packet *p);

#endif