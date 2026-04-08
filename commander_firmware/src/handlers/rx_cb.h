#ifndef PICO_RP2040_HANDLERS_RX_CB_H
#define PICO_RP2040_HANDLERS_RX_CB_H

#include "../handlers/boot.h"
#include "../packet/packet_rx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PACKET_RX 8

#define RX_PROCESS_FIFO(INTERFACE, AVAILABLE, READ)                              \
    do {                                                                         \
        uint32_t count = TU_DIV_CEIL(AVAILABLE(INTERFACE), 8);                   \
        if (!count) return;                                                      \
        if (count > MAX_PACKET_RX) count = MAX_PACKET_RX;                        \
        packet_rx_t packet_rx;                                                   \
        do {                                                                     \
            uint32_t read = READ(INTERFACE, &packet_rx, sizeof(packet_rx));      \
            if (read < (sizeof(packet_rx.id) + sizeof(packet_rx.cmd))) continue; \
            if (packet_rx.cmd == CMD_SYSTEM_REBOOT)                              \
                handle_boot_rom(packet_rx);                                      \
            packet_rx_rb_push(packet_rx);                                        \
        } while (--count);                                                       \
    } while (0)

#define RX_PROCESS_BUF(INTERFACE, BUFFER, BUFSIZE)                                   \
    do {                                                                             \
        (void)INTERFACE;                                                             \
                                                                                     \
        packet_rx_t packet_rx;                                                       \
        const packet_rx_t* end = (const packet_rx_t*)(BUFFER + BUFSIZE);             \
        for (const packet_rx_t* p = (const packet_rx_t*)BUFFER; p + 1 <= end; p++) { \
            memcpy(&packet_rx, p, sizeof(packet_rx_t));                              \
            if (packet_rx.cmd == CMD_SYSTEM_REBOOT)                                  \
                handle_boot_rom(packet_rx);                                          \
            packet_rx_rb_push(packet_rx);                                            \
        }                                                                            \
    } while (0)

#define RX_PROCESS_BUF_OLD(INTERFACE, BUFFER, BUFSIZE, EPSIZE)        \
    do {                                                              \
        (void)INTERFACE;                                              \
                                                                      \
        uint32_t count = TU_DIV_CEIL(BUFSIZE, sizeof(packet_tx_t));   \
        if (!count) return;                                           \
        if (count > EPSIZE / sizeof(packet_rx_t))                     \
            count = EPSIZE / sizeof(packet_rx_t);                     \
                                                                      \
        packet_rx_t packet_rx;                                        \
                                                                      \
        for (uint32_t i = 0; i < count; i++) {                        \
            uint32_t offset = i * sizeof(packet_rx_t);                \
            if (offset + sizeof(packet_rx_t) > BUFSIZE)               \
                break;                                                \
            memcpy(&packet_rx, BUFFER + offset, sizeof(packet_rx_t)); \
            if (packet_rx.cmd == CMD_SYSTEM_REBOOT)                   \
                handle_boot_rom(packet_rx);                           \
            packet_rx_rb_push(packet_rx);                             \
        }                                                             \
    } while (0)
#ifdef __cplusplus
}
#endif

#endif