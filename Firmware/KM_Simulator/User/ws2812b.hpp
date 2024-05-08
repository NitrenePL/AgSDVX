#include "debug.h"

#define NUM_LED 18   // 灯光数量
#define CODE0   0x80 // 0码 1000 0000
#define CODE1   0xFC // 1码 1111 1100

void SPI2_HD_Init()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    SPI_InitTypeDef SPI_InitStructure   = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStructure.SPI_Mode      = SPI_Mode_Master;

    SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL              = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA              = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
    SPI_Init(SPI2, &SPI_InitStructure);

    SPI_Cmd(SPI2, ENABLE);
}

/* Never USED */
void SPI1_HD_Init()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    SPI_InitTypeDef SPI_InitStructure   = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStructure.SPI_Mode      = SPI_Mode_Master;

    SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL              = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA              = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);
}

struct __attribute__((packed)) RGB {
    u8 R;
    u8 G;
    u8 B;
};

// not in use
// Low Light Level
extern const RGB WHITE  = {128, 128, 128};
extern const RGB RED    = {128, 0, 0};
extern const RGB GREEN  = {0, 128, 0};
extern const RGB BLUE   = {0, 0, 128};
extern const RGB PURPLE = {128, 0, 128};

// Medium Light Level
// extern const RGB WHITE    = {192, 192, 192};
// extern const RGB RED      = {192, 0, 0};
// extern const RGB GREEN    = {0, 192, 0};
// extern const RGB BLUE     = {0, 0, 192};
// extern const RGB PURPLE   = {192, 0, 192};

// High Light Level
// extern const RGB WHITE    = {255, 255, 255};
// extern const RGB RED      = {255, 0, 0};
// extern const RGB GREEN    = {0, 255, 0};
// extern const RGB BLUE     = {0, 0, 255};
// extern const RGB PURPLE   = {255, 0, 255};

extern const RGB NO_COLOR = {0, 0, 0};

static u8 LED_Buffer[NUM_LED * 24];

void rgb_SetColor(u16 LedId, RGB Color)
{

    u16 i;

    if (LedId > (NUM_LED)) {
        printf("Error:Out of Range!\r\n");
        return; // to avoid overflow
    }

    u8 *p = &LED_Buffer[LedId * 24];

    for (i = 0; i <= 7; i++) {
        p[i] = ((Color.G & (1 << (7 - i))) ? (CODE1) : CODE0);
    }
    for (i = 8; i <= 15; i++) {
        p[i] = ((Color.R & (1 << (15 - i))) ? (CODE1) : CODE0);
    }
    for (i = 16; i <= 23; i++) {
        p[i] = ((Color.B & (1 << (23 - i))) ? (CODE1) : CODE0);
    }
}

void Spi_Send(u8 *buf, u16 len)
{
    for (u16 i = 0; i < len; i++) {
        while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
        SPI_NSSInternalSoftwareConfig(SPI2, SPI_NSSInternalSoft_Set);
        SPI_I2S_SendData(SPI2, buf[i]);
    }
}

void LED_Send()
{
    const u16 len = NUM_LED * 24;
    u8 temp[len]  = {0};
    Spi_Send(temp, len);
    memcpy(temp, LED_Buffer, sizeof(LED_Buffer));
    Spi_Send(temp, len);
}
