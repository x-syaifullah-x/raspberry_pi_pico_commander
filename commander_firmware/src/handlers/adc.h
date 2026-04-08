#ifndef PICO_RP2040_ADC_H
#define PICO_RP2040_ADC_H

#include <math.h>

#include "../include/packet.h"
#include "hardware/adc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INPUT_ADC_ONBOARD 0x04

static inline packet_tx_t handle_adc(packet_rx_t packet_rx) {
    packet_tx_t packet_tx = {.type = {packet_rx.cmd, 0x00}, .id = packet_rx.id, .payload = {0x00}};
    if (packet_rx.args_size == 0) {
        packet_tx.type[0] = TYPE_ERR;
        packet_tx.payload[0] = ERR_CMD_ARG_INVALID;
        return packet_tx;
    }
    uint8_t input = packet_rx.args[0];
    switch (input) {
        // printf '\x[ID]\x03\x04]' > /dev/hidrawN
        case INPUT_ADC_ONBOARD:
            adc_select_input(input);
            uint16_t adc = adc_read() /* 0 – 4095 */;
            float voltage = roundf((adc * 3.3f) / 4095.0f * 100) / 100;
            float temp = roundf((27.0F - (voltage - 0.706F) / 0.001721F) * 100) / 100;
            uint16_t voltage_fixed = voltage * 100;
            packet_tx.payload[0] = input;
            packet_tx.payload[1] = (voltage_fixed >> 0) & 0xFF;
            packet_tx.payload[2] = (voltage_fixed >> 8) & 0xFF;
            uint16_t temp_fixed = temp * 100;
            packet_tx.payload[3] = (temp_fixed >> 0) & 0xFF;
            packet_tx.payload[4] = (temp_fixed >> 8) & 0xFF;
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