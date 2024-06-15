#include "debug.h"
#include "stdbool.h"
#include "usb_dc.h"
#include "usbd_core.h"
#include "hid_keyboard_template.h"
#include "light.hpp"
#include "controller.hpp"

extern volatile u8 Scan_Key_Status[7];
extern volatile s16 counter;

extern "C" void usb_dc_low_level_init(void)
{
    RCC_USBCLK48MConfig(RCC_USBCLK48MCLKSource_USBPHY);
    RCC_USBHSPLLCLKConfig(RCC_HSBHSPLLCLKSource_HSE);
    RCC_USBHSConfig(RCC_USBPLL_Div2);
    RCC_USBHSPLLCKREFCLKConfig(RCC_USBHSPLLCKREFCLK_4M);
    RCC_USBHSPHYPLLALIVEcmd(ENABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBHS, ENABLE);
    NVIC_EnableIRQ(USBHS_IRQn);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, ENABLE);
    // EXTEN->EXTEN_CTR |= EXTEN_USBD_PU_EN;
    NVIC_EnableIRQ(OTG_FS_IRQn);

    printf("usb init\n");

    Delay_Us(100);
}

extern "C" void usb_hc_low_level_init(void)
{
#ifdef CH32V30x_D8C
    RCC_USBCLK48MConfig(RCC_USBCLK48MCLKSource_USBPHY);
    RCC_USBHSPLLCLKConfig(RCC_HSBHSPLLCLKSource_HSE);
    RCC_USBHSConfig(RCC_USBPLL_Div2);
    RCC_USBHSPLLCKREFCLKConfig(RCC_USBHSPLLCKREFCLK_4M);
    RCC_USBHSPHYPLLALIVEcmd(ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBHS, ENABLE);
#else
    if (SystemCoreClock == 144000000) {
        RCC_OTGFSCLKConfig(RCC_OTGFSCLKSource_PLLCLK_Div3);
    } else if (SystemCoreClock == 96000000) {
        RCC_OTGFSCLKConfig(RCC_OTGFSCLKSource_PLLCLK_Div2);
    } else if (SystemCoreClock == 48000000) {
        RCC_OTGFSCLKConfig(RCC_OTGFSCLKSource_PLLCLK_Div1);
    }
#endif
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, ENABLE);
    NVIC_EnableIRQ(OTG_FS_IRQn);
    NVIC_EnableIRQ(USBHS_IRQn);
    static uint32_t wait_ct = 20000;
    while (wait_ct--) {
    }
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);

    /* Initialize GPIO for keyboard scan */
    KB_Scan_Init();
    printf("KB Scan OK!\r\n");

    /* Initialize timer for USB report*/
    TIM3_Init(1, SystemCoreClock / 10000 - 1);
    printf("TIM3 for USB OK!\r\n");

    // /* Initialize timer for Rainbow LED control*/
    // TIM2_Init(1000 - 1, 14400 - 1); // 1ms
    // printf("TIM2 for LED OK!\r\n");

    // /* Initialize USBHD interface to communicate with the host  */
    // USBHS_RCC_Init();
    // USBHS_Device_Init(ENABLE);

    /* Initialize Rotary_encoder */
    Rotary_TIM_Init();
    printf("Rotary OK!\r\n");

    /* Initialize SPI Half-Duplex-Mode for transmitting data to LED*/
    SPI2_HD_Init();
    printf("SPI2 HD OK!\r\n");

    /* Initialize WS2812b*/
    TranscendLights_Init();
    printf("TranscendLights OK!\r\n");

    printf("CherryUSB device hid keyboard example\n");

    hid_init(0, USBHS_BASE);

    while (!usb_device_is_configured(0)) {
    }

    printf("usb done\n");

    while (1) {
        Rotary_TIM_Scan();
        // KB_Scan();

        BTN_Scan();

    }
}
