#ifndef PICO_RP2040_PACKET_TX_H
#define PICO_RP2040_PACKET_TX_H

#include "packet/packet.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
    uint32_t raw[2];
    struct {
        id_t id;
        status_t status;
        cmd_t cmd;
        uint8_t data[5];
    };
} packet_tx_t;

_Static_assert((sizeof(packet_tx_t) & (sizeof(packet_tx_t) - 1)) == 0, "packet_tx_t size must be power of two");

static inline void packet_tx_log(const packet_tx_t* tx) {
    _Static_assert(sizeof(packet_tx_t) == 8, "packet_tx_t size changed, update packet_tx_log!");

    printf(
        "│ id: %03u │ status: %03u │ cmd: %03u │ data: [%03u, %03u, %03u, %03u, %03u] │\n",
        tx->id, tx->status, tx->cmd, tx->data[0], tx->data[1], tx->data[2], tx->data[3], tx->data[4]);
}

#ifdef __cplusplus
}
#endif

#endif