#ifndef PICO_RP2040_UART0_H
#define PICO_RP2040_UART0_H

#include "handlers/boot.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/uart.h"
#include "packet/packet_rx.h"
#include "pin.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UART0_ENABLE 0

#define UART0_ID uart0
#define UART0_IRQ UART0_IRQ
#define UART0_RX_PIN PIN_02
#define UART0_TX_PIN PIN_01

void uart0_irq_handler(void) {
    static packet_rx_t rx;
    static uint32_t rx_len = 0;
    uint8_t* rx_ptr = (uint8_t*)&rx;

    while (uart_is_readable(UART0_ID)) {
        rx_ptr[rx_len++] = uart_getc(UART0_ID);
        if (rx_len == sizeof(packet_rx_t)) {
            if (rx.cmd == CMD_SYSTEM_REBOOT)
                handle_boot_rom(rx);
            packet_rx_rb_push(&rx);
            rx_len = 0;
        }
    }
}

// void uart0_irq_handler(void) {
//     static packet_tx_t tx;
//     static uint32_t tx_len = 0;
//     uint8_t* tx_ptr = (uint8_t*)&tx;

//     while (uart_is_readable(UART0_ID)) {
//         tx_ptr[tx_len++] = uart_getc(UART0_ID);
//         if (tx_len == sizeof(packet_tx_t)) {
//             packet_tx_log(&tx);
//             tx_len = 0;
//         }
//     }
// }

void uart0_init(uint32_t baudrate) {
#if UART0_ENABLE
    if (uart_is_enabled(UART0_ID)) return;

    uart_init(UART0_ID, baudrate);

    gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);

    uart_set_fifo_enabled(UART0_ID, true);

    irq_set_exclusive_handler(UART0_IRQ, uart0_irq_handler);
    irq_set_enabled(UART0_IRQ, true);

    uart_set_irq_enables(UART0_ID, true, false);
#else
    (void)baudrate;
#endif
}

#ifdef __cplusplus
}
#endif

#endif