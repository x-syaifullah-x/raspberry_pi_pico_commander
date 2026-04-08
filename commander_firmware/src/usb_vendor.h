#ifndef PICO_RP2040_USB_VENDOR_H
#define PICO_RP2040_USB_VENDOR_H

#include "dispatcher.h"

#ifdef __cplusplus
extern "C" {
#endif

void tud_vendor_rx_cb(uint8_t idx, const uint8_t *buffer, uint32_t bufsize) {
    (void)buffer;
    (void)bufsize;

    packet_rx_t packet_rx = {.id = 0x00, .cmd = 0x00, .args = {0x00}, .args_size = 0x00};
    uint32_t count = tud_vendor_n_read(idx, &packet_rx, sizeof(packet_rx));
    packet_rx.args_size = count - sizeof(packet_rx.id) - sizeof(packet_rx.cmd);
    packet_tx_t packet_tx = dispatch(packet_rx);

    tud_vendor_n_write(idx, &packet_tx, sizeof(packet_tx));
    tud_vendor_n_write_flush(idx);

    // clang-format off
    #if (CFG_TUD_CDC == 1)
        if(tud_cdc_ready()) {
            tud_cdc_write(&packet_tx, sizeof(packet_tx));
            tud_cdc_write_flush();
        }
    #endif

    #if (CFG_TUD_HID == 1)
        if (tud_hid_ready())
            tud_hid_n_report(0x00, 0x00, &packet_tx, sizeof(packet_tx));
    #endif
    // clang-format on
}

#ifdef __cplusplus
}
#endif

#endif