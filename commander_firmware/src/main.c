#include "bsp/board.h"
#include "dispatcher.h"
#include "handlers/led_default.h"
#include "hardware/clocks.h"
#include "packet/packet_rx.h"
#include "packet/packet_tx.h"
#include "pico/stdlib.h"
#include "uart0.h"
#include "usb.h"

int main(void) {
    // set_sys_clock_khz(150000, true); // MIN=50000, MAX=180000

    tusb_init();

    // clang-format off
    #if defined(LIB_PICO_STDIO_USB)
        stdio_usb_init();
    #endif
    // clang-format on

    led_default_init();

    uart0_init(PICO_DEFAULT_UART_BAUD_RATE);

    static packet_tx_t tx_buf[PACKET_RX_EP_CAPACITY];

    while (true) {
        tud_task();

        packet_rx_t rx;
        uint32_t tx_count = 0;
        while (packet_rx_rb_pop(&rx)) {
            tx_buf[tx_count++] = dispatch(rx);
            // packet_tx_log(&tx_buf[tx_count - 1]);
            if (tx_count == PACKET_RX_EP_CAPACITY)
                break;
        }

        if (!tx_count) continue;

        uint32_t packet_tx_size = tx_count * sizeof(packet_tx_t);

        // clang-format off
        #if UART0_ENABLE
            uart_write_blocking(UART0_ID, (const uint8_t*)tx_buf, packet_tx_size);
        #endif

        #if CFG_TUD_VENDOR
            if (tud_vendor_mounted()) {
                uint32_t available = tud_vendor_write_available();
                uint32_t tx_size = TU_MIN(packet_tx_size, available) & ~(sizeof(packet_tx_t) - 1);
                if (tx_size > 0 && tud_vendor_write(tx_buf, tx_size))
                    tud_vendor_write_flush();
            }
        #endif

        #if CFG_TUD_HID
            if (tud_hid_ready())
                tud_hid_report(0, tx_buf, INPUT_HID_REPORT_COUNT);
        #endif
        // clang-format on
    }
}