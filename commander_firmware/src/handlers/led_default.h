#ifndef PICO_RP2040_LED_DEFAULT_H
#define PICO_RP2040_LED_DEFAULT_H

#include "../include/packet.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "pico/time.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute__((packed)) {
    uint8_t mode;
    uint16_t delay_ms;
} led_t;

#define DEFAULT_DELAY_MS 1000

#define LED_DEFAULT_DELAY_STEP_MS 100
#define LED_DEFAULT_TIMER_DELAY_US_TO_MS(t) ((t).delay_us / 1000)

#define LED_DEFAULT_OFF 0x00
#define LED_DEFAULT_ON 0x01
#define LED_DEFAULT_BLINK 0x02

volatile bool led_blink_stat = false;

struct repeating_timer timer;

static inline bool led_blink(struct repeating_timer *t) {
    (void)t;

    static bool state = false;
    gpio_put(PICO_DEFAULT_LED_PIN, (state = !state));
    return true;
}

static inline led_t led_default(bool on) {
    led_blink_stat = false;
    uint32_t irq_flags = save_and_disable_interrupts();
    cancel_repeating_timer(&timer);
    restore_interrupts(irq_flags);
    gpio_put(PICO_DEFAULT_LED_PIN, on);
    return (led_t){.mode = gpio_get(PICO_DEFAULT_LED_PIN), .delay_ms = 0x00};
}

static inline led_t led_default_blink(int32_t delay_ms) {
    uint32_t irq_flags = save_and_disable_interrupts();
    cancel_repeating_timer(&timer);
    restore_interrupts(irq_flags);
    led_blink_stat = add_repeating_timer_ms(delay_ms, led_blink, NULL, &timer);
    uint16_t timer_ms = LED_DEFAULT_TIMER_DELAY_US_TO_MS(timer);
    return (led_t){.mode = LED_DEFAULT_BLINK, .delay_ms = timer_ms};
}

static inline led_t led_default_status() {
    led_t res = {.mode = 0x00, .delay_ms = 0x00};
    if (led_blink_stat) {
        res.mode = LED_DEFAULT_BLINK;
        uint16_t timer_ms = LED_DEFAULT_TIMER_DELAY_US_TO_MS(timer);
        res.delay_ms = timer_ms;
    } else
        res.mode = gpio_get(PICO_DEFAULT_LED_PIN);
    return res;
}

static inline packet_tx_t handle_led_default(packet_rx_t packet_rx) {
    packet_tx_t packet_tx = {.type = {packet_rx.cmd, 0x00}, .id = packet_rx.id, .payload = {0x00}};
    if (packet_rx.args_size == 0) {
        packet_tx.type[0] = TYPE_ERR;
        packet_tx.payload[0] = ERR_CMD_ARG_INVALID;
        return packet_tx;
    }
    led_t led;
    switch (packet_rx.args[0]) {
        case LED_DEFAULT_OFF:
        case LED_DEFAULT_ON:
            led = led_default(packet_rx.args[0]);
            break;
        case LED_DEFAULT_BLINK:;
            int32_t delay_ms = DEFAULT_DELAY_MS;
            if (packet_rx.args_size == 2) {
                uint8_t delay_step = packet_rx.args[1];
                if (!delay_step) {
                    packet_tx.type[0] = TYPE_ERR;
                    packet_tx.payload[0] = ERR_CMD_ARG_INVALID;
                    return packet_tx;
                }
                delay_ms = delay_step * LED_DEFAULT_DELAY_STEP_MS;
            }
            led = led_default_blink(delay_ms);
            break;
        case 0xFF:
            led = led_default_status();
            break;
        default:
            packet_tx.type[0] = TYPE_ERR;
            packet_tx.payload[0] = ERR_CMD_ARG_INVALID;
            return packet_tx;
    }

    packet_tx.payload[0] = led.mode;
    packet_tx.payload[1] = (led.delay_ms >> 0) & 0xFF;
    packet_tx.payload[2] = (led.delay_ms >> 8) & 0xFF;
    return packet_tx;
}

#ifdef __cplusplus
}
#endif

#endif