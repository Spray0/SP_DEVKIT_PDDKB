#include "led.h"


/*  RGB_LED 初始化IO */
void RGBLED_Init(){

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_Initure;

    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;
    GPIO_Initure.Pull=GPIO_PULLUP;
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;

    GPIO_Initure.Pin=GPIO_PIN_14;
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
    GPIO_Initure.Pin=GPIO_PIN_15;
    HAL_GPIO_Init(GPIOA,&GPIO_Initure); 
    GPIO_Initure.Pin=GPIO_PIN_3;
    HAL_GPIO_Init(GPIOB,&GPIO_Initure); 

    GPIO_Initure.Pin=GPIO_PIN_1;
    GPIO_Initure.Pull=GPIO_PULLDOWN; 
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);

    LED_RED_OFF;
    LED_GREEN_OFF;
    LED_BLUE_OFF;
}

/*  RGB_LED 控制  */
void RGBLED_Disp(enum color Color){
    (Color&0b100)?LED_RED_ON:LED_RED_OFF;
    (Color&0b010)?LED_GREEN_ON:LED_GREEN_OFF;
    (Color&0b001)?LED_BLUE_ON:LED_BLUE_OFF;
}

