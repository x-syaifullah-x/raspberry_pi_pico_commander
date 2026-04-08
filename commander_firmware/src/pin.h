#ifndef PICO_RP2040_PIN_H
#define PICO_RP2040_PIN_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum __attribute__((packed)) {
    PIN_01 = 0,   // GP0
    PIN_02 = 1,   // GP1
    PIN_04 = 2,   // GP2
    PIN_05 = 3,   // GP3
    PIN_06 = 4,   // GP4
    PIN_07 = 5,   // GP5
    PIN_09 = 6,   // GP6
    PIN_10 = 7,   // GP7
    PIN_11 = 8,   // GP8
    PIN_12 = 9,   // GP9
    PIN_14 = 10,  // GP10
    PIN_15 = 11,  // GP11
    PIN_16 = 12,  // GP12
    PIN_17 = 13,  // GP13
    PIN_19 = 14,  // GP14
    PIN_20 = 15,  // GP15
    PIN_21 = 16,  // GP16
    PIN_22 = 17,  // GP17
    PIN_24 = 18,  // GP18
    PIN_25 = 19,  // GP19
    PIN_26 = 20,  // GP20
    PIN_27 = 21,  // GP21
    PIN_29 = 22,  // GP22
    PIN_31 = 26,  // GP26
    PIN_32 = 27,  // GP27
    PIN_34 = 28,  // GP28
    PIN_COUNT,
} pin_t;

#ifdef __cplusplus
}
#endif

#endif