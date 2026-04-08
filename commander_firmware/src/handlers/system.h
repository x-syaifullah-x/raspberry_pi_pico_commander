#ifndef PICO_RP2040_SYSTEM_H
#define PICO_RP2040_SYSTEM_H

#include "../include/packet.h"
#include "hardware/watchdog.h"
#include "pico/bootrom.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum __attribute__((packed)) {
    SYSTEM_REBOOT = 0x01,
    SYSTEM_REBOOT_BOOTSEL = 0x02,
} system_reboot_t;

static inline packet_tx_t handle_system(packet_rx_t packet_rx) {
    packet_tx_t packet_tx = {.type = {packet_rx.cmd, 0x00}, .id = packet_rx.id, .payload = {0x00}};
    if (packet_rx.args_size == 0) {
        packet_tx.type[0] = TYPE_ERR;
        packet_tx.payload[0] = ERR_CMD_ARG_INVALID;
        return packet_tx;
    }
    switch (packet_rx.args[0]) {
        case SYSTEM_REBOOT:
            watchdog_reboot(0, 0, 0);
            return packet_tx;
        case SYSTEM_REBOOT_BOOTSEL:
            reset_usb_boot(0, 0);
            return packet_tx;
        default:
            packet_tx.type[0] = TYPE_ERR;
            packet_tx.payload[0] = ERR_CMD_ARG_INVALID;
            return packet_tx;
    }
}

#ifdef __cplusplus
}
#endif

#endif