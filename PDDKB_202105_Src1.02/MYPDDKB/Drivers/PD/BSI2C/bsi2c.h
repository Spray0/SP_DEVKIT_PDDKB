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

#ifndef __BSI2C_H
#define __BSI2C_H

#include "main.h"

#define LOG UART1_Printf
#define BSTimeStamp HAL_GetTick()

unsigned char BSI2C_Init(void);
unsigned char BSINT_Init(void);
unsigned char BSINT_CUR(void);
unsigned char BSI2C_Read(unsigned char DevAddr,unsigned short RegAddr,unsigned char *pData,unsigned char len);
unsigned char BSI2C_Write(unsigned char DevAddr,unsigned short RegAddr,unsigned char *pData,unsigned char len);

#endif