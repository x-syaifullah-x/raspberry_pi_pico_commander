#ifndef PICO_RP2040_EXEC_H
#define PICO_RP2040_EXEC_H

#include "handlers/adc.h"
#include "handlers/led_default.h"
#include "handlers/system.h"
#include "handlers/tb_6612_fng.h"
#include "include/packet.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline packet_tx_t dispatch(packet_rx_t packet_rx) {
    switch (packet_rx.cmd) {
        case CMD_SYSTEM:
            return handle_system(packet_rx);
        case CMD_LED_DEFAULT:
            return handle_led_default(packet_rx);
        case CMD_ADC_READ:
            return handle_adc(packet_rx);
        case CMD_TB6612FNG:
            return handle_tb6612fng(packet_rx);
        default:
            return (packet_tx_t){.type = TYPE_ERR, .payload = {ERR_CMD_UNKNOWN}};
    }
}

#ifdef __cplusplus
}
#endif

#endif