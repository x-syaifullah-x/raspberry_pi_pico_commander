#ifndef PICO_RP2040_PACKET_RX_H
#define PICO_RP2040_PACKET_RX_H

#include "packet.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
    uint32_t raw[2];
    struct {
        id_t id;
        cmd_t cmd;
        uint8_t args[6];
    };
} packet_rx_t;

_Static_assert((sizeof(packet_rx_t) & (sizeof(packet_rx_t) - 1)) == 0, "packet_rx_t size must be power of two");

#define PACKET_RX_EP_CAPACITY (64 / sizeof(packet_rx_t))

#define PACKET_RX_RB_Q_SIZE 16  // buffer size (must be power of two), effective capacity = SIZE - 1
_Static_assert((PACKET_RX_RB_Q_SIZE & (PACKET_RX_RB_Q_SIZE - 1)) == 0, "PACKET_RX_RB_Q_SIZE must be power of two");
#define PACKET_RX_RB_Q_MASK (PACKET_RX_RB_Q_SIZE - 1)

static packet_rx_t packet_rx_rb_buf[PACKET_RX_RB_Q_SIZE];

static volatile uint8_t packet_rx_rb_head = 0;  // read
static volatile uint8_t packet_rx_rb_tail = 0;  // write

static inline bool packet_rx_rb_push(packet_rx_t packet_rx) {
    uint32_t next = (packet_rx_rb_tail + 1) & PACKET_RX_RB_Q_MASK;
    if (next == packet_rx_rb_head) return false;  // full
    packet_rx_rb_buf[packet_rx_rb_tail] = packet_rx;
    __asm__ volatile("" ::: "memory");  // compiler barrier
    packet_rx_rb_tail = next;
    return true;
}

static inline bool packet_rx_rb_pop(packet_rx_t* packet_rx) {
    if (packet_rx_rb_head == packet_rx_rb_tail) return false;  // empty
    *packet_rx = packet_rx_rb_buf[packet_rx_rb_head];
    __asm__ volatile("" ::: "memory");  // compiler barrier
    packet_rx_rb_head = (packet_rx_rb_head + 1) & PACKET_RX_RB_Q_MASK;
    return true;
}

static inline uint32_t packet_rx_rb_count(void) {
    return (packet_rx_rb_tail - packet_rx_rb_head) & PACKET_RX_RB_Q_MASK;
}

#ifdef __cplusplus
}
#endif

#endif