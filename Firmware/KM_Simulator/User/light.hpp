#pragma once

#include "stdbool.h"
#include "debug.h"
#include "ws2812b.hpp"
#include <cstring>


/*********************************************************************
 * @fn      TranscendLights_Init
 *
 * @brief   Initialize WS2812B, dev
 *
 * @return  none
 */
void TranscendLights_Init(void)
{

    rgb_SetColor(0, NO_COLOR);
    rgb_SetColor(1, NO_COLOR);
    // rgb_SetColor(0, RED);
    // rgb_SetColor(1, GREEN);
    rgb_SetColor(2, BLUE);
    rgb_SetColor(3, RED);
    rgb_SetColor(4, YELLOW);
    rgb_SetColor(5, PURPLE);

    LED_Send();
}

struct __attribute__((packed)) LedData {
    u8 type;
    u8 ledBrightness;
    RGB ledColors[6];
};

/*********************************************************************
 * @fn      TranscendLights_Handle
 *
 * @brief   Handle WS2812B with the data from USB.
 *
 * @return  none
 */
extern "C" void TranscendLights_Handle(u8* data)
{
    LedData led = {0};
    memcpy((u8 *)&led, data, sizeof(LedData));
    for (size_t i = 0; i < 6; i++) {
        rgb_SetColor(i, led.ledColors[i]);
    }
    LED_Send();
}
