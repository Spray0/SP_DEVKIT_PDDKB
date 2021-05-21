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

#include "main.h"

#define MCU_Device "STM32F103T8U6"
const char PDD__SoftWareVersion[]={1,0,2}; // 软件版本号
void SystemClock_Config(void);
void PrintMenu(void);
void LED_Ctr(void);
void BUTTON_Init(void);
void BUTTON_Ctr(void);

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART1)//如果是串口1
	{
    /*
		//aRxBuffer[0] 
    if(UART1_aRxBuffer[0]=='\r'){
      	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = '\n' ; 
    }
    	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = UART1_aRxBuffer[0] ;   
    */
	}
}

#define USBCPWR_5VOUT_ON HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET)
#define USBCPWR_5VOUT_OFF  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET)
/**
 * @brief Main Func
*/
int main(void)
{
  SystemClock_Config();
  HAL_Init();
  RGBLED_Init();
  BUTTON_Init();
  UART1_Init();
  BSI2C_Init();
  BSINT_Init();

  if(FUSB302B_READ_ID()){
    RGBLED_Disp(red);
    Error_Handler();
  }
  
  PD_UFP_Init();
  
  USBCPWR_5VOUT_OFF;
  
  PrintMenu();

  while (1)
  {
    UART_DMA_TX_CheckInLoop();
    PD_UFP_InLoop();
    LED_Ctr();
    BUTTON_Ctr();
  }
}


/**
 * @brief Led control
*/
void LED_Ctr(){
  switch (USBC.State)
    {
    case usb_null:
      USBCPWR_5VOUT_ON;
      RGBLED_Disp(off);
      break;
    case usb_default:
      USBCPWR_5VOUT_OFF;
      break;
    case usb_pd:
      RGBLED_Disp(green);
      break;
    case usb_pd_ready:
      RGBLED_Disp(cyan);
      break;
    case usb_retry_wait:
      RGBLED_Disp(red);
      break;
    default:
      break;
    }        
}

unsigned char Button_down=1;
/**
 * @brief Button ctr
*/
void BUTTON_Ctr(){
  //if(USBC.Source_Capabilities_NUM>1){
    if(1){
    unsigned char now=(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6)==GPIO_PIN_RESET)?0:1;
    if((Button_down)&&(now==0)){
      //UART1_Printf("Button_down!\r\n");
      unsigned char index=USBC.Source_NowIndex;
      index++;
      if(index>USBC.Source_Capabilities_NUM)index=1;
      PD_RequestPower(USBC.Source_Capabilities[index-1].Voltage);
    }
    Button_down=now;
  }
}

/**
 * @brief print menu
*/
void PrintMenu(){
  UART1_Printf("===================================\r\n");
  UART1_Printf(" USB Power Delivery Sink Demo\r\n");
  UART1_Printf(" MCU ");UART1_Printf(MCU_Device);UART1_Printf(" @ ");
  UART1_Printf("%dKHz\r\n",(unsigned int)HAL_RCC_GetSysClockFreq()/1000);
  UART1_Printf(" PDC %s\r\n",FUSB302B_ID_Str);
  UART1_Printf(" VER v%d.%d%d Spray0\r\n",PDD__SoftWareVersion[0],PDD__SoftWareVersion[1],PDD__SoftWareVersion[2]);
  UART1_Printf("===================================\r\n");
}

/**
 * @brief Button_init
*/
void BUTTON_Init(){
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
