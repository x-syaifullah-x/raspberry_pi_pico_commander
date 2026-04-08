#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_TUD_ENABLED 1
#define CFG_TUD_CDC 1
#define CFG_TUD_HID 1
#define CFG_TUD_VENDOR 1

#define CFG_TUSB_MCU OPT_MCU_RP2040
#define CFG_TUSB_DEBUG 0                         // 0=OFF| 1=ERR | 2=ERR + WARN | 3=ERR + WARN + INFO
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE)  // USB MODE(OPT_MODE_DEVICE | OPT_MODE_HOST | OPT_MODE_NONE)

#ifdef __cplusplus
}
#endif

#endif