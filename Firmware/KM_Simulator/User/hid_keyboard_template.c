/*
 * Copyright (c) 2024, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "usbd_core.h"
#include "usbd_hid.h"

#define USBD_VID                         0x2341
#define USBD_PID                         0x8036
#define USBD_MAX_POWER                   100
#define USBD_LANGID_STRING               2052 // zh-hans

#define SDVX_EP                          0x81
#define SDVX_EP_SIZE                     4
#define SDVX_EP_INTERVAL                 1
#define SDVX_REPORT_DESC_SIZE            60

#define DATA_CONFIG_HID_EP               1
#define DATA_CONFIG_HID_EP_SIZE          64
#define DATA_CONFIG_HID_EP_INTERVAL      10
#define DATA_CONFIG_HID_REPORT_DESC_SIZE 23

#define USB_HID_CONFIG_DESC_SIZ          41

static const uint8_t hid_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0002, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_HID_CONFIG_DESC_SIZ, 0x01, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),

    /* 9 */
    /******************** Descriptor of SDVX interface ********************/
    0x09,                          /* bLength: Interface Descriptor size */
    USB_DESCRIPTOR_TYPE_INTERFACE, /* bDescriptorType: Interface descriptor type */
    0x00,                          /* bInterfaceNumber: Number of Interface */
    0x00,                          /* bAlternateSetting: Alternate setting */
    0x02,                          /* bNumEndpoints */
    0x03,                          /* bInterfaceClass: HID */
    0x00,                          /* bInterfaceSubClass : 1=BOOT, 0=no boot */
    0x00,                          /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
    0,                             /* iInterface: Index of string descriptor */
    /* 18 */
    /******************** Descriptor of SDVX ********************/
    0x09,                    /* bLength: HID Descriptor size */
    HID_DESCRIPTOR_TYPE_HID, /* bDescriptorType: HID */
    0x11,                    /* bcdHID: HID Class Spec release number */
    0x01,
    0x00,                  /* bCountryCode: Hardware target country */
    0x01,                  /* bNumDescriptors: Number of HID class descriptors to follow */
    0x22,                  /* bDescriptorType */
    SDVX_REPORT_DESC_SIZE, /* wItemLength: Total length of Report descriptor */
    0x00,
    /* 27 */
    /******************** Descriptor of SDVX ep IN ********************/
    0x07,                         /* bLength: Endpoint Descriptor size */
    USB_DESCRIPTOR_TYPE_ENDPOINT, /* bDescriptorType: */
    SDVX_EP,                      /* bEndpointAddress: Endpoint Address (IN 2) */
    0x03,                         /* bmAttributes: Interrupt endpoint */
    WBVAL(SDVX_EP_SIZE),          /* wMaxPacketSize: 4 Byte max */
    SDVX_EP_INTERVAL,             /* bInterval: Polling Interval */
    /* 34 */
    /******************** Descriptor of Data Config HID endpoint OUT ********************/
    0x07,                           /* bLength: Endpoint Descriptor size */
    USB_DESCRIPTOR_TYPE_ENDPOINT,   /* bDescriptorType: */
    DATA_CONFIG_HID_EP,             /* bEndpointAddress: Endpoint Address (OUT 1) */
    0x03,                           /* bmAttributes: Interrupt endpoint */
    WBVAL(DATA_CONFIG_HID_EP_SIZE), /* wMaxPacketSize: 4 Byte max */
    DATA_CONFIG_HID_EP_INTERVAL,    /* bInterval: Polling Interval */
    /* 41 */

    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT(USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'N', 0x00,                  /* wcChar0 */
    'i', 0x00,                  /* wcChar1 */
    't', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'e', 0x00,                  /* wcChar4 */
    'n', 0x00,                  /* wcChar5 */
    'e', 0x00,                  /* wcChar6 */
    'P', 0x00,                  /* wcChar7 */
    'L', 0x00,                  /* wcChar8 */
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
    24,                         /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'A', 0x00,                  /* wcChar0 */
    'g', 0x00,                  /* wcChar1 */
    '.', 0x00,                  /* wcChar2 */
    'S', 0x00,                  /* wcChar3 */
    '.', 0x00,                  /* wcChar4 */
    'D', 0x00,                  /* wcChar5 */
    '.', 0x00,                  /* wcChar6 */
    'V', 0x00,                  /* wcChar7 */
    '.', 0x00,                  /* wcChar8 */
    'X', 0x00,                  /* wcChar9 */
    '.', 0x00,                  /* wcChar10 */

    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    14,                         /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '1', 0x00,                  /* wcChar0 */
    '1', 0x00,                  /* wcChar1 */
    '4', 0x00,                  /* wcChar2 */
    '5', 0x00,                  /* wcChar3 */
    '1', 0x00,                  /* wcChar4 */
    '4', 0x00,                  /* wcChar5 */
#ifdef CONFIG_USB_HS
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0A,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x00,
    0x00,
#endif
    0x00};

// /* USB HID device Configuration Descriptor */
// static uint8_t hid_desc[9] __ALIGN_END = {
//     /* 18 */
//     0x09,                    /* bLength: HID Descriptor size */
//     HID_DESCRIPTOR_TYPE_HID, /* bDescriptorType: HID */
//     0x11,                    /* bcdHID: HID Class Spec release number */
//     0x01,
//     0x00,                          /* bCountryCode: Hardware target country */
//     0x01,                          /* bNumDescriptors: Number of HID class descriptors to follow */
//     0x22,                          /* bDescriptorType */
//     HID_KEYBOARD_REPORT_DESC_SIZE, /* wItemLength: Total length of Report descriptor */
//     0x00,
// };


static const uint8_t sdvx_report_desc[SDVX_REPORT_DESC_SIZE] = {
    0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
    0x09, 0x04, // Usage (Joystick)
    0xA1, 0x01, // Collection (Application)
    0x05, 0x01, //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x01, //   Usage (Pointer)
    0xA1, 0x00, //   Collection (Physical)
    0x09, 0x30, //     Usage (X)
    0x09, 0x31, //     Usage (Y)
    0x15, 0x00, //     Logical Minimum (0)
    0x25, 0xFF, //     Logical Maximum (-1)
    0x75, 0x08, //     Report Size (8)
    0x95, 0x02, //     Report Count (2)
    0x81, 0x06, //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,       //   End Collection
    0x05, 0x09, //   Usage Page (Button)
    0x19, 0x01, //   Usage Minimum (0x01)
    0x29, 0x07, //   Usage Maximum (0x07)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x10, //   Report Count (16)
    0x81, 0x06, //   Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF, // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x02,       //   Usage (0x02)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x40,       //   Report Count (64)
    0x91, 0x06,       //   Output (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,       // End Collection

    // 60 bytes
};

#define HID_STATE_IDLE 0
#define HID_STATE_BUSY 1

/*!< hid state ! Data can be sent only when state is idle  */
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[DATA_CONFIG_HID_EP_SIZE];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t send_buffer[SDVX_EP_SIZE];

static volatile uint8_t sdvx_state = HID_STATE_IDLE;

static void usbd_event_handler(uint8_t busid, uint8_t event)
{
    switch (event) {
        case USBD_EVENT_RESET:
            break;
        case USBD_EVENT_CONNECTED:
            break;
        case USBD_EVENT_DISCONNECTED:
            break;
        case USBD_EVENT_RESUME:
            break;
        case USBD_EVENT_SUSPEND:
            break;
        case USBD_EVENT_CONFIGURED:
            /* setup first out ep read transfer */
            usbd_ep_start_read(busid, DATA_CONFIG_HID_EP, read_buffer, DATA_CONFIG_HID_EP_SIZE);
            sdvx_state = HID_STATE_IDLE;
            break;
        case USBD_EVENT_SET_REMOTE_WAKEUP:
            break;
        case USBD_EVENT_CLR_REMOTE_WAKEUP:
            break;

        default:
            break;
    }
}

void data_config_hid_callback(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("actual out len:%ld\r\n", nbytes);
    usbd_ep_start_read(busid, ep, read_buffer, DATA_CONFIG_HID_EP_SIZE);

    extern void TranscendLights_Handle(uint8_t* data);
    TranscendLights_Handle(read_buffer);
}

void sdvx_callback(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    (void)busid;
    (void)ep;
    // USB_LOG_RAW("actual in len:%d\r\n", nbytes);
    sdvx_state = HID_STATE_IDLE;
}

static struct usbd_endpoint data_config_hid_out_ep = {
    .ep_addr = DATA_CONFIG_HID_EP,
    .ep_cb   = data_config_hid_callback};

static struct usbd_endpoint sdvx_ep = {
    .ep_addr = SDVX_EP,
    .ep_cb   = sdvx_callback};

struct usbd_interface intf0;

void hid_init(uint8_t busid, uint32_t reg_base)
{
    usbd_desc_register(busid, hid_descriptor);
    usbd_add_interface(busid, usbd_hid_init_intf(busid, &intf0, sdvx_report_desc, SDVX_REPORT_DESC_SIZE));
    usbd_add_endpoint(busid, &sdvx_ep);
    usbd_add_endpoint(busid, &data_config_hid_out_ep);
    usbd_initialize(busid, reg_base, usbd_event_handler);
}

int send_report(uint8_t busid, const uint8_t* data) {
    int ret = usbd_ep_start_write(busid, SDVX_EP, data, 4);
    if (ret < 0) {
        return ret;
    }
    sdvx_state = HID_STATE_BUSY;
    while (sdvx_state == HID_STATE_BUSY) {
    }
    return 0;
}