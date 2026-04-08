#ifndef PICO_RP2040_HANDLERS_BOOT_H
#define PICO_RP2040_HANDLERS_BOOT_H

#include "hardware/watchdog.h"
#include "packet/packet_rx.h"
#include "pico/bootrom.h"

#ifdef __cplusplus
extern "C" {
#endif

enum __attribute__((packed)) {
    REBOOT = 0x00,
    RESET_USB_BOOT,
};

static inline void handle_boot_rom(packet_rx_t rx) {
    switch (rx.args[0]) {
        case REBOOT:
            watchdog_reboot(0, 0, 0);
            break;
        case RESET_USB_BOOT:
            if (rx.args[1] > 2) return;
            reset_usb_boot(0, rx.args[1]);
            break;
    }
}

#ifdef __cplusplus
}
#endif

#endif