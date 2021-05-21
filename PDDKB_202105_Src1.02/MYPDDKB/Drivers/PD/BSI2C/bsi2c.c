/*  
  USB Power Delivery Sink driver code
  Debug device: FUSB302B
  Refernece: 
              USB_PD_R3_0 V2.0 20190829.pdf
              FUSB302B-D.pdf
  Spray0 
  Copyright © 2021 Spray0,All Rights Reserved.
  <javen.chan@outlook.com>
  2021-5 Ver1.0.2 Changzhou.China
*/

#include "bsi2c.h"


/**
 * @brief 板级支持 INT 初始化
*/
unsigned char BSINT_Init(void){
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    return 0;
}

/**
 * @brief 板级支持 INT 检查
*/
unsigned char BSINT_CUR(void){
    return (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5)==GPIO_PIN_RESET)?1:0;
}

/**
 * @brief 板级支持 I2C 初始化
*/
unsigned char BSI2C_Init(void){
    I2C1_Init();
    return 0;
}

/**
 * @brief 板级支持 I2C 从设备寄存器读取数据
*/
unsigned char BSI2C_Read(unsigned char DevAddr,unsigned short RegAddr,unsigned char *pData,unsigned char len){
    if(HAL_I2C_Mem_Read(&hi2c1,(DevAddr<<1)+1,RegAddr,I2C_MEMADD_SIZE_8BIT,pData,len,1000)==HAL_OK)
    return 0;
    else
    return 1;
}

/**
 * @brief 板级支持 I2C 向设备寄存器写入数据
*/
unsigned char BSI2C_Write(unsigned char DevAddr,unsigned short RegAddr,unsigned char *pData,unsigned char len){
    if(HAL_I2C_Mem_Write(&hi2c1,(DevAddr<<1),RegAddr,I2C_MEMADD_SIZE_8BIT,pData,len,1000)==HAL_OK)
    return 0;
    else
    return 1;
}