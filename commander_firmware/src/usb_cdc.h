#ifndef PICO_RP2040_CDC_H
#define PICO_RP2040_CDC_H

#include "dispatcher.h"

#ifdef __cplusplus
extern "C" {
#endif

void tud_cdc_rx_cb(uint8_t itf) {
    packet_rx_t packet_rx = {.id = 0x00, .cmd = 0x00, .args = {0x00}, .args_size = 0x00};
    uint32_t count = tud_cdc_n_read(itf, &packet_rx, sizeof(packet_rx));
    packet_rx.args_size = count - sizeof(packet_rx.id) - sizeof(packet_rx.cmd);
    packet_tx_t packet_tx = dispatch(packet_rx);
    tud_cdc_n_write(itf, &packet_tx, sizeof(packet_tx));
    tud_cdc_n_write_flush(itf);

    // clang-format off
    #if (CFG_TUD_HID == 1)
        if (tud_hid_ready()){
            tud_hid_n_report(0x00, 0x00, &packet_tx, sizeof(packet_tx));
        }
    #endif

    #if (CFG_TUD_VENDOR == 1)
        tud_vendor_write(&packet_tx, sizeof(packet_tx));
        tud_vendor_write_flush();
    #endif
    // clang-format on
}

#ifdef __cplusplus
}
#endif

#endif