#ifndef PICO_RP2040_PACKET_H
#define PICO_RP2040_PACKET_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum __attribute__((packed)) {
    STATUS_OK = 0x00,
    STATUS_ERR = 0x01,
} status_t;

typedef enum __attribute__((packed)) {
    STATUS_ERR_CMD_UNKNOWN = 0x00,
    STATUS_ERR_CMD_NOT_IMPLEMENTED,
    STATUS_ERR_CMD_ARG_INVALID,
    STATUS_ERR_CMD_ARG_SIZE_INVALID,
} status_err_t;

typedef enum __attribute__((packed)) {
    CMD_UNKNOWN = 0,
    CMD_SYSTEM_REBOOT,
    CMD_SYSTEM_END = 19,

    CMD_LED_DEFAULT_BEGIN = 20,
    CMD_LED_DEFAULT,
    CMD_LED_DEFAULT_END = 39,

    CMD_ADC_BEGIN = 40,
    CMD_ADC_DMA,
    CMD_ADC_READ_CH0,
    CMD_ADC_READ_CH1,
    CMD_ADC_READ_CH2,
    CMD_ADC_READ_CH3,
    CMD_ADC_READ_CH4,
    CMD_ADC_END = 59,

    CMD_TB6612FNG_BEGIN = 60,
    CMD_TB6612FNG_POWER,
    CMD_TB6612FNG_MOTOR_1,
    CMD_TB6612FNG_MOTOR_2,
    CMD_TB6612FNG_MOTOR_STATE,
    CMD_TB6612FNG_STATE,
    CMD_TB6612FNG_END = 79,
} cmd_t;

typedef enum __attribute__((packed)) {
    ID_DEVICE = 0x00,
    ID_HOST = 0x01,
} id_t;

#ifdef __cplusplus
}
#endif

#endif