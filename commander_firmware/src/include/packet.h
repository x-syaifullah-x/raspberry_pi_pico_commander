#ifndef PICO_RP2040_PACKET_H
#define PICO_RP2040_PACKET_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum __attribute__((packed)) {
    TYPE_ERR = 0x00,
} type_t;

typedef enum __attribute__((packed)) {
    ERR_CMD_UNKNOWN = 0x01,
    ERR_CMD_ARG_INVALID,
} err_t;

typedef enum __attribute__((packed)) {
    CMD_SYSTEM = 0x01,
    CMD_LED_DEFAULT = 0x02,
    CMD_ADC_READ = 0x03,
    CMD_TB6612FNG = 0x04,
} cmd_t;

typedef struct __attribute__((packed)) {
    uint8_t type[2];
    uint8_t id;
    uint8_t payload[5];
} packet_tx_t;

typedef struct __attribute__((packed)) {
    uint8_t id;
    cmd_t cmd;
    uint8_t args[8];
    uint8_t args_size;
} packet_rx_t;

#ifdef __cplusplus
}
#endif

#endif