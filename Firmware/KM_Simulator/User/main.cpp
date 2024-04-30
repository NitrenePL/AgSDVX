#include "controller.hpp"
#include "debug.h"
#include "ch32v30x_usbhs_device.h"
#include "stdbool.h"

extern volatile u8 Scan_Key_Status[7];
extern volatile s16 counter;

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
    printf("TIM3 OK!\r\n");

    /* Initialize USBHD interface to communicate with the host  */
    USBHS_RCC_Init();
    USBHS_Device_Init(ENABLE);

    /* Initialize Rotary_encoder */
    Rotary_TIM_Init();
    printf("Rotary OK!\r\n");

    while (1) {
        Rotary_TIM_Scan();
        KB_Scan();
    }
}
