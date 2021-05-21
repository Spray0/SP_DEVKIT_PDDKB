#include "uart.h"

unsigned char UART1_aRxBuffer[1];
UART_HandleTypeDef UART1_Handler; 
DMA_HandleTypeDef hdma_usart1_tx;

/**
 * @brief UART1 初始化 N.8.1 115200
*/
void UART1_Init(void)
{	
	UART1_Handler.Instance=USART1;					    
	UART1_Handler.Init.BaudRate=115200;				   
	UART1_Handler.Init.WordLength=UART_WORDLENGTH_8B; 
	UART1_Handler.Init.StopBits=UART_STOPBITS_1;	   
	UART1_Handler.Init.Parity=UART_PARITY_NONE;		   
	UART1_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;  
	UART1_Handler.Init.Mode=UART_MODE_TX_RX;		   
	HAL_UART_Init(&UART1_Handler);		

	HAL_UART_Receive_IT(&UART1_Handler, (unsigned char *)UART1_aRxBuffer, 1);
}

/**
 * @brief 串口1中断服务程序
*/
void USART1_IRQHandler(void)                	
{ 
	unsigned int timeout=0;
	
	HAL_UART_IRQHandler(&UART1_Handler);
	
    while (HAL_UART_GetState(&UART1_Handler) != HAL_UART_STATE_READY)
	{
	    timeout++;
        if(timeout>HAL_MAX_DELAY) break;		
	}
     
	timeout=0;
	while(HAL_UART_Receive_IT(&UART1_Handler, (unsigned char *)UART1_aRxBuffer, 1) != HAL_OK)
	{
	    timeout++;
	    if(timeout>HAL_MAX_DELAY) break;	
	}
} 

/*
*******************************************************************************************************************************
*/

#define DMATX_BUF_MAXLEN 1024 //DMA发送缓冲数据长度
/**
 * @brief DMA发送缓冲结构体
*/
struct DMATX_BUF
{
	unsigned char BUF[DMATX_BUF_MAXLEN];
	unsigned short cnt;
}DMATX_BUFA,DMATX_BUFB;
enum DMATX_BUF_NAME{
	BUFA,
	BUFB
};
unsigned char UART1TX_DMA_Busy=0; //DMA发送忙标记
enum DMATX_BUF_NAME UART1TX_DMABUF_Name=BUFB; //DMA当前发送的队列名
/**
 * @brief DMA发送控制
*/
void UART_DMA_TX_CheckInLoop(){
	// 如果DMA发送空闲、上次队列发送完毕
	if(UART1TX_DMA_Busy==0){
		if(UART1TX_DMABUF_Name==BUFB){
			if(DMATX_BUFA.cnt>0){
				HAL_UART_Transmit_DMA(&UART1_Handler,DMATX_BUFA.BUF,DMATX_BUFA.cnt);
				UART1TX_DMA_Busy=1;
				UART1TX_DMABUF_Name=BUFA;
				DMATX_BUFB.cnt=0;
			}
		}
		if(UART1TX_DMABUF_Name==BUFA){
			if(DMATX_BUFB.cnt>0){
				HAL_UART_Transmit_DMA(&UART1_Handler,DMATX_BUFB.BUF,DMATX_BUFB.cnt);
				UART1TX_DMA_Busy=1;
				UART1TX_DMABUF_Name=BUFB;
				DMATX_BUFA.cnt=0;
			}
		}
	}
}
/**
 * @brief str数据写入发送缓冲
*/
int send_string_to_buf(char * str)
{	
	//往BUFA写入
	while(*str!='\0'){
		if(UART1TX_DMABUF_Name==BUFB){
			DMATX_BUFA.BUF[DMATX_BUFA.cnt]=*str;
			DMATX_BUFA.cnt++;
		}
		if(UART1TX_DMABUF_Name==BUFA){
			DMATX_BUFB.BUF[DMATX_BUFB.cnt]=*str;
			DMATX_BUFB.cnt++;
		}
		if(DMATX_BUFA.cnt==DMATX_BUF_MAXLEN){
			//写满
			while (UART1TX_DMA_Busy);
			HAL_UART_Transmit_DMA(&UART1_Handler,DMATX_BUFA.BUF,DMATX_BUFA.cnt);
			UART1TX_DMA_Busy=1;
			UART1TX_DMABUF_Name=BUFA;
			DMATX_BUFB.cnt=0;
			return 0;
		}
		if(DMATX_BUFB.cnt==DMATX_BUF_MAXLEN){
			//写满
			while (UART1TX_DMA_Busy);
			HAL_UART_Transmit_DMA(&UART1_Handler,DMATX_BUFB.BUF,DMATX_BUFB.cnt);
			UART1TX_DMA_Busy=1;
			UART1TX_DMABUF_Name=BUFB;
			DMATX_BUFA.cnt=0;
			return 0;
		}
		str++;
	}	
	return 0;
}

/**
 * @brief UART1_Printf
*/
int UART1_Printf(const char * fmt,...)             		                    
{
	__va_list arg_ptr; 
	char buf[128];
  		
	memset(buf,'\0',sizeof(buf));

	va_start(arg_ptr,fmt);
	vsprintf(buf,fmt,arg_ptr);
	va_end(arg_ptr);

	send_string_to_buf(buf);

	return 0;
}

/*
*******************************************************************************************************************************
*/
