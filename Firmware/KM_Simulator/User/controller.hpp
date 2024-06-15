#pragma once

/*******************************************************************************/
/* Header Files */
#include "stdbool.h"
#include "debug.h"
#include "string.h"
#include "boost/endian.hpp"
#include "ws2812b.hpp"
#include "hid_keyboard_template.h"


#define NUM_BTN 7
/*******************************************************************************/
/* Variable Definition */

/* Keyboard */
volatile bool KB_Scan_Done     = false;
volatile u8 Scan_Key_Status[7] = {0};

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

    TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel                   = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM3, ENABLE);
}

// void TIM2_Init(u16 arr, u16 psc)
// {
//     TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//     NVIC_InitTypeDef NVIC_InitStructure;

//     RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

//     TIM_TimeBaseStructure.TIM_Period        = arr;
//     TIM_TimeBaseStructure.TIM_Prescaler     = psc;
//     TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
//     TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
//     TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

// 	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
//     TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

//     NVIC_InitStructure.NVIC_IRQChannel                   = TIM2_IRQn;
//     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
//     NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 4;
//     NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;

//     NVIC_Init(&NVIC_InitStructure);
//     TIM_Cmd(TIM2, ENABLE);
// }

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

struct __attribute__((packed)) BTN {
    GPIO_TypeDef *GPIOx;
    uint16_t GPIO_Pin;

    u16 counterRelease;
    uint8_t state;
};

BTN btns[NUM_BTN] = {
    {GPIOC, GPIO_Pin_0, 0, 0},  // BT_A
    {GPIOC, GPIO_Pin_1, 0, 0},  // BT_B
    {GPIOB, GPIO_Pin_12, 0, 0}, // BT_C
    {GPIOB, GPIO_Pin_13, 0, 0}, // BT_D
    {GPIOC, GPIO_Pin_2, 0, 0},  // FX_L
    {GPIOB, GPIO_Pin_14, 0, 0}, // FX_R
    {GPIOB, GPIO_Pin_11, 0, 0}, // START
};

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

volatile u32 DEBOUNCE_CHECKS = 144;
void BTN_Scan(void)
{
    for (int i = 0; i < NUM_BTN; i++) {
        u8 currentReading = GPIO_ReadInputDataBit(btns[i].GPIOx, btns[i].GPIO_Pin);
        if (currentReading == 0) { // 低电平有效
            btns[i].state          = 1;
            btns[i].counterRelease = 0;
        } else {
            if (btns[i].counterRelease < DEBOUNCE_CHECKS) {
                btns[i].counterRelease++;
            } else {
                btns[i].state = 0;
            }
        }
        Scan_Key_Status[i] = btns[i].state;
    }
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
    u8 xAxis;
    u8 yAxis;
    u16 buttons;
};

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
    u8 rdata[4]   = {0};
    SDVXData data = {0};

    data.xAxis = (u8)((s16)(counter_left / ENCODER_SENSITIVITY) % 256);
    data.yAxis = (u8)((s16)(counter_right / ENCODER_SENSITIVITY) % 256);

    for (int i = 0; i < 7; i++) {
        if (Scan_Key_Status[i]) {
            data.buttons |= (uint16_t)1 << i;
        } else {
            data.buttons &= ~((uint16_t)1 << i);
        }
    }

    memcpy(rdata, &data, sizeof(SDVXData));
    send_report(0, rdata);
}

/*********************************************************************
 * @fn      HSBtoRGB
 *
 * @brief   change HSB to RGB
 *
 * @return  none
 */
RGB HSBtoRGB(u8 hue, u8 saturation, u8 brightness)
{
    RGB rgb;
    u8 region, remainder, p, q, t;

    if (saturation == 0) {
        rgb.R = brightness;
        rgb.G = brightness;
        rgb.B = brightness;
        return rgb;
    }

    region    = hue / 43;
    remainder = (hue - (region * 43)) * 6;

    p = (brightness * (255 - saturation)) >> 8;
    q = (brightness * (255 - ((saturation * remainder) >> 8))) >> 8;
    t = (brightness * (255 - ((saturation * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0:
            rgb.R = brightness;
            rgb.G = t;
            rgb.B = p;
            break;
        case 1:
            rgb.R = q;
            rgb.G = brightness;
            rgb.B = p;
            break;
        case 2:
            rgb.R = p;
            rgb.G = brightness;
            rgb.B = t;
            break;
        case 3:
            rgb.R = p;
            rgb.G = q;
            rgb.B = brightness;
            break;
        case 4:
            rgb.R = t;
            rgb.G = p;
            rgb.B = brightness;
            break;
        default:
            rgb.R = brightness;
            rgb.G = p;
            rgb.B = q;
            break;
    }

    // Halved to save power
    rgb.R /= 2;
    rgb.G /= 2;
    rgb.B /= 2;

    return rgb;
}

volatile bool RainbowFlag;

void gradientRainbowEffect()
{
    static u8 hue = 0;
    // printf("%d\r", RainbowFlag);
    if (RainbowFlag) {
        for (int i = 0; i < NUM_LED; i++) {
            u8 scaledHue = (i * 256 / NUM_LED) + hue;
            rgb_SetColor(i, HSBtoRGB(scaledHue, 255, 255));
        }
        hue++;
        LED_Send();
    }
}

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        /* Clear interrupt flag */
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

        /* Handle Rotary_encoder & Buttons */
        Rotary_Button_Handle();
    }
}

