#ifndef PICO_RP2040_HANDLERS_TB6612FNG_H
#define PICO_RP2040_HANDLERS_TB6612FNG_H

#include <stdint.h>

#include "hardware/pwm.h"
#include "src/packet/packet_rx.h"
#include "src/packet/packet_tx.h"
#include "src/pin.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TB6612FNG_PIN_STBY PIN_04

#define TB6612FNG_PIN_IN_1_A PIN_07
#define TB6612FNG_PIN_IN_2_A PIN_06
#define TB6612FNG_PIN_PWM_A PIN_05

#define TB6612FNG_PIN_IN_1_B PIN_09
#define TB6612FNG_PIN_IN_2_B PIN_10
#define TB6612FNG_PIN_PWM_B PIN_11

#define TB6612FNG_MOTOR_COUNT 0x02

#define TB6612FNG_PWM_WRAP 1000
#define TB6612FNG_PWM_CLKDIV 6.25F
#define TB6612FNG_PWM_DUTY(speed) (((speed) > 100 ? 100 : (speed)) * 10)

typedef enum __attribute__((packed)) {
    TB6612FNG_MOTOR_DIRECTION_STOP = 0x00,
    TB6612FNG_MOTOR_DIRECTION_FORWARD = 0x01,
    TB6612FNG_MOTOR_DIRECTION_REVERSE = 0x02,
    TB6612FNG_MOTOR_DIRECTION_BRAKE = 0x03,
} tb6612fng_direction_t;

typedef struct {
    status_err_t err;
    uint8_t direction;
    uint8_t speed;
} tb6612fng_motor_state_t;

typedef struct {
    uint8_t stby;
    uint8_t in_1;
    uint8_t in_2;
    uint8_t pwm;
} tb6612fng_pin_t;

tb6612fng_pin_t tb6612fng_pins[TB6612FNG_MOTOR_COUNT] = {
    {.stby = TB6612FNG_PIN_STBY, .in_1 = TB6612FNG_PIN_IN_1_A, .in_2 = TB6612FNG_PIN_IN_2_A, .pwm = TB6612FNG_PIN_PWM_A},
    {.stby = TB6612FNG_PIN_STBY, .in_1 = TB6612FNG_PIN_IN_1_B, .in_2 = TB6612FNG_PIN_IN_2_B, .pwm = TB6612FNG_PIN_PWM_B},
};

static inline uint16_t tb6612fng_get_pwm_level(uint slice_num, uint chan) {
    return chan == PWM_CHAN_A ? (pwm_hw->slice[slice_num].cc & 0xFFFF) : ((pwm_hw->slice[slice_num].cc >> 16) & 0xFFFF);
}

static inline uint8_t tb6612fng_get_speed(uint slice_num, uint chan) {
    uint16_t level = tb6612fng_get_pwm_level(slice_num, chan);
    return ((level * 100) + (TB6612FNG_PWM_WRAP / 2)) / TB6612FNG_PWM_WRAP;  // [0..100]
}

static inline uint8_t tb6612fng_get_speed_by_pin(uint8_t pwm_pin) {
    return tb6612fng_get_speed(pwm_gpio_to_slice_num(pwm_pin), pwm_gpio_to_channel(pwm_pin));
}

static inline uint8_t tb6612fng_get_direction(uint8_t in_1_pin, uint8_t in_2_pin) {
    if (gpio_get_dir(in_1_pin) != GPIO_OUT || gpio_get_dir(in_2_pin) != GPIO_OUT)
        return 0;
    bool p1 = gpio_get(in_1_pin);
    bool p2 = gpio_get(in_2_pin);
    if (p1) {
        if (p2)
            return 3;
        return 1;
    } else {
        if (p2)
            return 2;
        return 0;
    }
}

static inline bool tb6612fng_get_power(uint8_t stby_pin) {
    return (gpio_get_dir(stby_pin) == GPIO_OUT) && (gpio_get(stby_pin) == true);
}

static inline tb6612fng_motor_state_t tb6612fng_run(tb6612fng_pin_t pin, tb6612fng_direction_t direction, uint8_t speed) {
    tb6612fng_motor_state_t motor = {.err = 0x00};
    if (!gpio_get(pin.stby)) {
        motor.err = STATUS_ERR_CMD_ARG_INVALID;
        return motor;
    }

    switch (direction) {
        case TB6612FNG_MOTOR_DIRECTION_STOP:
            gpio_put(pin.in_1, 0);
            gpio_put(pin.in_2, 0);
            motor.direction = TB6612FNG_MOTOR_DIRECTION_STOP;
            break;
        case TB6612FNG_MOTOR_DIRECTION_FORWARD:
            gpio_put(pin.in_1, 1);
            gpio_put(pin.in_2, 0);
            motor.direction = TB6612FNG_MOTOR_DIRECTION_FORWARD;
            break;
        case TB6612FNG_MOTOR_DIRECTION_REVERSE:
            gpio_put(pin.in_1, 0);
            gpio_put(pin.in_2, 1);
            motor.direction = TB6612FNG_MOTOR_DIRECTION_REVERSE;
            break;
        case TB6612FNG_MOTOR_DIRECTION_BRAKE:
            gpio_put(pin.in_1, 1);
            gpio_put(pin.in_2, 1);
            motor.direction = TB6612FNG_MOTOR_DIRECTION_BRAKE;
            break;
        default:
            motor.err = STATUS_ERR_CMD_ARG_INVALID;
            return motor;
    }

    uint slice_num = pwm_gpio_to_slice_num(pin.pwm);
    uint chann = pwm_gpio_to_channel(pin.pwm);
    pwm_set_chan_level(slice_num, chann, TB6612FNG_PWM_DUTY(speed));
    motor.speed = tb6612fng_get_speed(slice_num, chann);
    return motor;
}

static inline bool tb6612fng_set_power(bool on, tb6612fng_pin_t pins[TB6612FNG_MOTOR_COUNT]) {
    uint8_t pin_stby = pins[0].stby;
    if (on) {
        if (gpio_get_function(pin_stby) == GPIO_FUNC_NULL)
            gpio_init(pin_stby);
        if (gpio_get_dir(pin_stby) != GPIO_OUT)
            gpio_set_dir(pin_stby, GPIO_OUT);
        gpio_put(pin_stby, 1);

        for (uint8_t i = 0; i < TB6612FNG_MOTOR_COUNT; i++) {
            tb6612fng_pin_t pin = pins[i];
            if (gpio_get_function(pin.in_1) == GPIO_FUNC_NULL)
                gpio_init(pin.in_1);
            if (gpio_get_dir(pin.in_1) != GPIO_OUT)
                gpio_set_dir(pin.in_1, GPIO_OUT);
            if (gpio_get_function(pin.in_2) == GPIO_FUNC_NULL)
                gpio_init(pin.in_2);
            if (gpio_get_dir(pin.in_2) != GPIO_OUT)
                gpio_set_dir(pin.in_2, GPIO_OUT);
            if (gpio_get_function(pin.pwm) != GPIO_FUNC_PWM) {
                gpio_set_function(pin.pwm, GPIO_FUNC_PWM);
                uint slice_num = pwm_gpio_to_slice_num(pin.pwm);
                pwm_set_wrap(slice_num, TB6612FNG_PWM_WRAP);
                pwm_set_clkdiv(slice_num, TB6612FNG_PWM_CLKDIV);
                pwm_set_enabled(slice_num, true);
            }
        }
    } else {
        for (uint8_t i = 0; i < TB6612FNG_MOTOR_COUNT; i++) {
            tb6612fng_pin_t pin = pins[i];
            uint slice_num = pwm_gpio_to_slice_num(pin.pwm);
            uint chan = pwm_gpio_to_channel(pin.pwm);
            pwm_set_enabled(slice_num, false);
            pwm_set_chan_level(slice_num, chan, 0);
            gpio_deinit(pin.pwm);
            gpio_deinit(pin.in_1);
            gpio_deinit(pin.in_2);
        }
        gpio_deinit(pins[0].stby);
    }
    return on;
}

static inline packet_tx_t handle_tb6612fng_power(packet_rx_t packet_rx) {
    packet_tx_t packet_tx = {.id = packet_rx.id, .status = STATUS_OK, .cmd = packet_rx.cmd};
    switch (packet_rx.args[0]) {
        case 0:
            packet_tx.data[0] = tb6612fng_set_power(false, tb6612fng_pins);
            break;
        case 1:
            packet_tx.data[0] = tb6612fng_set_power(true, tb6612fng_pins);
            break;
        case 255:
            packet_tx.data[0] = tb6612fng_get_power(TB6612FNG_PIN_STBY);
            break;
        default:
            packet_tx.status = STATUS_ERR;
            packet_tx.data[0] = STATUS_ERR_CMD_ARG_INVALID;
            return packet_tx;
    }
    return packet_tx;
}

static inline packet_tx_t handle_tb6612fng_motor(packet_rx_t packet_rx, tb6612fng_pin_t pin) {
    packet_tx_t packet_tx = {.id = packet_rx.id, .status = STATUS_OK, .cmd = packet_rx.cmd};
    tb6612fng_motor_state_t m = tb6612fng_run(pin, packet_rx.args[0], packet_rx.args[1]);
    if (m.err) {
        packet_tx.status = STATUS_ERR;
        packet_tx.data[0] = m.err;
        return packet_tx;
    }
    packet_tx.data[0] = m.direction;
    packet_tx.data[1] = m.speed;
    return packet_tx;
}

static inline packet_tx_t tb6612fng_motor_state(packet_rx_t packet_rx) {
    packet_tx_t packet_tx = {.id = packet_rx.id, .status = STATUS_OK};
    switch (packet_rx.args[0]) {
        case CMD_TB6612FNG_MOTOR_1:
            packet_tx.cmd = CMD_TB6612FNG_MOTOR_1;
            packet_tx.data[0] = tb6612fng_get_direction(tb6612fng_pins[0].in_1, tb6612fng_pins[0].in_2);
            packet_tx.data[1] = tb6612fng_get_speed_by_pin(tb6612fng_pins[0].pwm);
            return packet_tx;
        case CMD_TB6612FNG_MOTOR_2:
            packet_tx.cmd = CMD_TB6612FNG_MOTOR_2;
            packet_tx.data[0] = tb6612fng_get_direction(tb6612fng_pins[1].in_1, tb6612fng_pins[1].in_2);
            packet_tx.data[1] = tb6612fng_get_speed_by_pin(tb6612fng_pins[1].pwm);
            return packet_tx;
        default:
            packet_tx.status = STATUS_ERR;
            packet_tx.data[0] = STATUS_ERR_CMD_ARG_INVALID;
            return packet_tx;
    }
}

static inline packet_tx_t handle_tb6612fng(packet_rx_t packet_rx) {
    switch (packet_rx.cmd) {
        case CMD_TB6612FNG_POWER:
            return handle_tb6612fng_power(packet_rx);
        case CMD_TB6612FNG_MOTOR_1:
            return handle_tb6612fng_motor(packet_rx, tb6612fng_pins[0]);
        case CMD_TB6612FNG_MOTOR_2:
            return handle_tb6612fng_motor(packet_rx, tb6612fng_pins[1]);
        case CMD_TB6612FNG_MOTOR_STATE:
            return tb6612fng_motor_state(packet_rx);
        default:
            return (packet_tx_t){.id = packet_rx.id, .status = STATUS_ERR, .cmd = packet_rx.cmd, .data = {STATUS_ERR_CMD_UNKNOWN}};
    }
}

#ifdef __cplusplus
}
#endif

#endif