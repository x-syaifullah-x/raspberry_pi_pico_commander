#ifndef PICO_RP2040_DISPATCHER_H
#define PICO_RP2040_DISPATCHER_H

#include "handlers/adc.h"
#include "handlers/led_default.h"
#include "handlers/tb_6612_fng.h"
#include "packet/packet_rx.h"
#include "packet/packet_tx.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline packet_tx_t dispatch(packet_rx_t packet_rx) {
    switch (packet_rx.cmd) {
        case CMD_LED_DEFAULT_BEGIN ... CMD_LED_DEFAULT_END:
            return handle_led_default(packet_rx);
        case CMD_ADC_BEGIN ... CMD_ADC_END:
            return handle_adc(packet_rx);
        case CMD_TB6612FNG_BEGIN ... CMD_TB6612FNG_END:
            return handle_tb6612fng(packet_rx);
        default:
            return (packet_tx_t){.id = packet_rx.id, .status = STATUS_ERR, .cmd = packet_rx.cmd, .data = {STATUS_ERR_CMD_UNKNOWN}};
    }
}

#ifdef __cplusplus
}
#endif

#endif