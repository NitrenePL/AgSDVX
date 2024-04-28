#pragma once

/*******************************************************************************/
/* Header Files */
#include "ch32v30x_usbhs_device.h"
#include "stdbool.h"
#include "debug.h"
#include "string.h"
#include "usbd_desc.h"
#include "boost/endian.hpp"

/*******************************************************************************/
/* Variable Definition */

/* Keyboard */
volatile bool KB_Scan_Done     = false;
volatile u8 Scan_Key_Status[7] = {0};
volatile u16 btns              = 0;

/* Rotary */
volatile s16 counter_left           = 0;
volatile s16 counter_right          = 0;
volatile double ENCODER_SENSITIVITY = 1.5625;

// encoder sensitivity = number of positions per rotation (400) / number of positions for HID report (256)  = 400 / 256

/*******************************************************************************/
/* Interrupt Function Declaration */
extern "C" {
void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
}

void TIM3_Init(uint16_t arr, uint16_t psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseStructure.TIM_Period        = arr;
    TIM_TimeBaseStructure.TIM_Prescaler     = psc;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel                   = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM3, ENABLE);
}

/*
    BT_A    PC0
    BT_B    PC1
    BT_C    PB12
    BT_D    PB13

    FX_L    PC2
    FX_R    PB14

    START   PB11

    Encoder_L   PB8(white)  PB9(green)
    Encoder_R   PA2(white)  PA3(green)
*/

void KB_Scan_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/*********************************************************************
 * @fn      KB_Scan
 *
 * @brief   Perform keyboard scan.
 *
 * @return  none
 */
void KB_Scan(void)
{
    Scan_Key_Status[0] = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0) == 0 ? true : false;  // BT_A
    Scan_Key_Status[1] = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 0 ? true : false;  // BT_B
    Scan_Key_Status[2] = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 0 ? true : false; // BT_C
    Scan_Key_Status[3] = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 0 ? true : false; // BT_D
    Scan_Key_Status[4] = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2) == 0 ? true : false;  // FX_L
    Scan_Key_Status[5] = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) == 0 ? true : false; // FX_R
    Scan_Key_Status[6] = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0 ? true : false; //  START
}

/*********************************************************************
 * @fn      Rotary_Left_TIM_Init
 *
 * @brief   Initialize Left Rotary TIM Encoder Mode.
 *
 * @return  none
 */
void Rotary_Left_TIM_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin         = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode        = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed       = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};
    TIM_TimeBaseStructure.TIM_ClockDivision       = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode         = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period              = 65536 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler           = 1 - 1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter   = 0;
    TIM_TimeBaseInit(TIM10, &TIM_TimeBaseStructure);

    TIM_ICInitTypeDef TIM_ICInitStructure;
    TIM_ICStructInit(&TIM_ICInitStructure);

    TIM_ICInitStructure.TIM_Channel    = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICFilter   = 0xF;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInit(TIM10, &TIM_ICInitStructure);

    TIM_ICInitStructure.TIM_Channel    = TIM_Channel_2;
    TIM_ICInitStructure.TIM_ICFilter   = 0xF;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInit(TIM10, &TIM_ICInitStructure);

    TIM_EncoderInterfaceConfig(TIM10, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);

    TIM_Cmd(TIM10, ENABLE);
}

/*********************************************************************
 * @fn      Rotary_Right_TIM_Init
 *
 * @brief   Initialize Right Rotary TIM Encoder Mode.
 *
 * @return  none
 */
void Rotary_Right_TIM_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin         = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode        = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed       = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};
    TIM_TimeBaseStructure.TIM_ClockDivision       = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode         = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period              = 65536 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler           = 1 - 1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter   = 0;
    TIM_TimeBaseInit(TIM9, &TIM_TimeBaseStructure);

    TIM_ICInitTypeDef TIM_ICInitStructure;
    TIM_ICStructInit(&TIM_ICInitStructure);

    TIM_ICInitStructure.TIM_Channel    = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICFilter   = 0xF;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInit(TIM9, &TIM_ICInitStructure);

    TIM_ICInitStructure.TIM_Channel    = TIM_Channel_2;
    TIM_ICInitStructure.TIM_ICFilter   = 0xF;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInit(TIM9, &TIM_ICInitStructure);

    TIM_EncoderInterfaceConfig(TIM9, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);

    TIM_Cmd(TIM9, ENABLE);
}

/*********************************************************************
 * @fn      Rotary_TIM_Init
 *
 * @brief   Initialize Rotary.
 *
 * @return  none
 */
void Rotary_TIM_Init()
{
    Rotary_Left_TIM_Init();
    Rotary_Right_TIM_Init();
};

struct __attribute__((packed)) SDVXData {
    u16 buttons;
    u8 xAxis;
    u8 yAxis;
    u8 zAxis;
};

__attribute__((aligned(4))) uint8_t HID_Report_Buffer[5];    // HID Report Buffer
volatile uint8_t HID_Set_Report_Flag = SET_REPORT_DEAL_OVER; // HID SetReport flag

void Rotary_TIM_Scan(void)
{
    counter_left  = TIM_GetCounter(TIM10);
    counter_right = TIM_GetCounter(TIM9);
}

/*********************************************************************
 * @fn      Rotary_Button_Handle
 *
 * @brief   Update USB data of Buttons and Rotary.
 *
 * @return  none
 */
void Rotary_Button_Handle()
{
    u8 rdata[5]   = {0};
    SDVXData data = {0};

    data.xAxis = (u8)((s16)(counter_left / ENCODER_SENSITIVITY) % 256);
    data.yAxis = (u8)((s16)(counter_right / ENCODER_SENSITIVITY) % 256);
    data.zAxis = 0;

    for (int i = 0; i < 7; i++) {
        if (Scan_Key_Status[i]) {
            data.buttons |= (uint16_t)1 << i;
        } else {
            data.buttons &= ~((uint16_t)1 << i);
        }
    }

    memcpy(rdata, &data, sizeof(SDVXData));
    USBHS_Endp_DataUp(DEF_UEP2, rdata, 5, DEF_UEP_CPY_LOAD);
}

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        /* Clear interrupt flag */
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

        /* Handle keyboard scan */
        if (USBHS_DevEnumStatus) {
            /* Handle Rotary_encoder & Buttons */
            Rotary_Button_Handle();
        }
    }
}
