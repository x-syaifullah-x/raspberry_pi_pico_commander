#ifndef PICO_RP2040_TEMPLATE_H
#define PICO_RP2040_TEMPLATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dispatcher.h"

#define INPUT_HID_REPORT_COUNT sizeof(packet_tx_t)
#define OUTPUT_HID_REPORT_COUNT sizeof(packet_rx_t)

static const uint8_t hid_descriptor_report[] = {
    HID_USAGE_PAGE_N(HID_USAGE_PAGE_VENDOR, 2),
    HID_USAGE(0x01),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
    /* Input */
    HID_USAGE(0x02),
    HID_LOGICAL_MIN(0x00),
    HID_LOGICAL_MAX_N(0xFF, 2),
    HID_REPORT_SIZE(8),
    HID_REPORT_COUNT(INPUT_HID_REPORT_COUNT),
    HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    /* Output */
    HID_USAGE(0x03),
    HID_LOGICAL_MIN(0x00),
    HID_LOGICAL_MAX_N(0xFF, 2),
    HID_REPORT_SIZE(8),
    HID_REPORT_COUNT(OUTPUT_HID_REPORT_COUNT),
    HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END,
};

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    (void)instance;

    return hid_descriptor_report;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    (void)report_id;
    (void)report_type;

    packet_rx_t packet_rx = {.id = buffer[0], .cmd = buffer[1], .args = {0x00}};
    packet_rx.args_size = bufsize - sizeof(packet_rx.id) - sizeof(packet_rx.cmd);
    memcpy(packet_rx.args, &buffer[sizeof(packet_rx.id) + sizeof(packet_rx.cmd)], packet_rx.args_size);
    packet_tx_t packet_tx = dispatch(packet_rx);
    if (tud_hid_ready())
        tud_hid_n_report(instance, 0x00, &packet_tx, sizeof(packet_tx));

    // clang-format off
    #if (CFG_TUD_CDC == 1)
        tud_cdc_write(&packet_tx, sizeof(packet_tx));
        tud_cdc_write_flush();
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