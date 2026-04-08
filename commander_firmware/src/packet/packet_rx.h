#ifndef PICO_RP2040_PACKET_RX_H
#define PICO_RP2040_PACKET_RX_H

#include "packet/packet.h"

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

static inline void packet_rx_log(const packet_rx_t* rx) {
    _Static_assert(sizeof(packet_rx_t) == 8, "packet_rx_t size changed, update packet_rx_log!");

    printf(
        "│ id: %03u │ cmd: %03u │ args: [%03u, %03u, %03u, %03u, %03u, %03u] │\n",
        rx->id, rx->cmd, rx->args[0], rx->args[1], rx->args[2], rx->args[3], rx->args[4], rx->args[5]);
}

#define PACKET_RX_EP_CAPACITY (64 / sizeof(packet_rx_t))

#define PACKET_RX_RB_Q_SIZE 16  // buffer size (must be power of two), effective capacity = SIZE - 1
_Static_assert((PACKET_RX_RB_Q_SIZE & (PACKET_RX_RB_Q_SIZE - 1)) == 0, "PACKET_RX_RB_Q_SIZE must be power of two");
#define PACKET_RX_RB_Q_MASK (PACKET_RX_RB_Q_SIZE - 1)

static packet_rx_t packet_rx_rb_buf[PACKET_RX_RB_Q_SIZE];
static volatile uint8_t packet_rx_rb_head = 0;  // read
static volatile uint8_t packet_rx_rb_tail = 0;  // write

static inline bool packet_rx_rb_push(const packet_rx_t* packet_rx) {
    uint32_t next = (packet_rx_rb_tail + 1) & PACKET_RX_RB_Q_MASK;
    if (next == packet_rx_rb_head) return false;
    packet_rx_rb_buf[packet_rx_rb_tail] = *packet_rx;
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

#include "handlers/boot.h"

#define RX_PROCESS_FIFO(INTERFACE, AVAILABLE, READ)                              \
    do {                                                                         \
        uint32_t count = TU_DIV_CEIL(AVAILABLE(INTERFACE), 8);                   \
        if (!count) return;                                                      \
        if (count > PACKET_RX_EP_CAPACITY) count = PACKET_RX_EP_CAPACITY;        \
        packet_rx_t packet_rx;                                                   \
        do {                                                                     \
            uint32_t read = READ(INTERFACE, &packet_rx, sizeof(packet_rx));      \
            if (read < (sizeof(packet_rx.id) + sizeof(packet_rx.cmd))) continue; \
            if (packet_rx.cmd == CMD_SYSTEM_REBOOT)                              \
                handle_boot_rom(packet_rx);                                      \
            packet_rx_rb_push(&packet_rx);                                       \
        } while (--count);                                                       \
    } while (0)

#define RX_PROCESS_BUF(INTERFACE, BUFFER, BUFSIZE)                              \
    do {                                                                        \
        (void)INTERFACE;                                                        \
                                                                                \
        const packet_rx_t* end = (const packet_rx_t*)(BUFFER + BUFSIZE);        \
        for (const packet_rx_t* p = (const packet_rx_t*)BUFFER; p < end; p++) { \
            if (p->cmd == CMD_SYSTEM_REBOOT)                                    \
                handle_boot_rom(*p);                                            \
            packet_rx_rb_push(p);                                               \
        }                                                                       \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif