#ifndef __hid_keyboard_template_H
#define __hid_keyboard_template_H

#ifdef __cplusplus
extern "C" {
#endif


extern void hid_init(uint8_t busid, uint32_t reg_base);
extern int send_report(uint8_t busid, const uint8_t* data);

#ifdef __cplusplus
}
#endif

#endif /*__hid_keyboard_template */
