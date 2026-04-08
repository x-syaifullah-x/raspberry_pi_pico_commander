#include "bsp/board.h"
#include "pico/stdlib.h"
#include "src/dispatcher.h"
#include "src/handlers/led_default.h"
#include "src/packet/packet_rx.h"
#include "src/packet/packet_tx.h"
#include "src/usb.h"

int main(void) {
    tusb_init();

    // clang-format off
    #if defined(LIB_PICO_STDIO_USB)
        stdio_usb_init();
    #endif
    // clang-format on

    led_default_init();

    static packet_tx_t tx_buf[PACKET_RX_EP_CAPACITY];

    while (true) {
        tud_task();

        uint32_t rx_rb_count = packet_rx_rb_count(),
                 rx_count = TU_MIN(rx_rb_count, PACKET_RX_EP_CAPACITY),
                 tx_count = 0;

        if (!rx_count) continue;

        for (uint32_t i = 0; i < rx_count; i++) {
            packet_rx_t rx;
            if (packet_rx_rb_pop(&rx))
                tx_buf[tx_count++] = dispatch(rx);
        }

        // clang-format off
        #if CFG_TUD_VENDOR
            uint32_t packet_tx_size = tx_count * sizeof(packet_tx_t);
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