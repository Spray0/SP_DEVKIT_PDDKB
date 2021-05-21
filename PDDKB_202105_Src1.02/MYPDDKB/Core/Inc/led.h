#ifndef __LED_H
#define __LED_H
#include "stm32f1xx_hal.h"

enum color {
    white = 0b111,
    yellow = 0b110,
    purple = 0b101,
    red = 0b100,
    cyan = 0b011,
    green = 0b010,
    blue = 0b001,
    off = 0b000
};


void RGBLED_Init(void);
void RGBLED_Disp(enum color Color);


#define LED_RED_OFF HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET)
#define LED_RED_ON HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET)
#define LED_GREEN_OFF HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET)
#define LED_GREEN_ON HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET)
#define LED_BLUE_OFF HAL_GPIO_WritePin(GPIOA,GPIO_PIN_14,GPIO_PIN_SET)
#define LED_BLUE_ON HAL_GPIO_WritePin(GPIOA,GPIO_PIN_14,GPIO_PIN_RESET)

#endif