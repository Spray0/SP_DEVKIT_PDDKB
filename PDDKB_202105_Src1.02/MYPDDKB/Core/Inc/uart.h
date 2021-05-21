#ifndef __UART_H
#define __UART_H
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "stm32f1xx_hal.h"

extern unsigned char UART1_aRxBuffer[1];
 
void UART1_Init(void);
int UART1_Printf(const char * fmt,...);
void UART_DMA_TX_CheckInLoop();

#endif