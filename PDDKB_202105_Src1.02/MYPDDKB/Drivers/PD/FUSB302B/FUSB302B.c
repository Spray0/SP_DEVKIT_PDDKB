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

#include "FUSB302B.h"

enum CC_X __FUSB302B_NOW_MEASCC=CCNULL; //当前配置成测量的CC通道
unsigned char is_timeout_active =0;
unsigned int timeout_expiration=0;


void start_timeout(unsigned int ms)
{
    is_timeout_active = 1;
    timeout_expiration = HAL_GetTick() + ms;
}
unsigned char has_timeout_expired(void)
{
    if (!is_timeout_active)
        return 0;

    unsigned int delta = timeout_expiration - HAL_GetTick();
    if (delta <= 0x8000000)
        return 0;

    is_timeout_active = 0;
    return 1;
}

/**
 * @brief 检查CC,返回CC状态
 * @return 1：有效 0：无效
*/
unsigned char FUSB302B_Is_CC_OK(void){
    unsigned char val;
    BSI2C_Read(FUSB302B_I2CADDR,REG_Status0,&val,1);
    return ((val&0b00000011)==0)?0:1;
}

/**
 * @brief CC处理
*/
void FUSB302B_Check_CC_Meas(void){
    unsigned char val;
    BSI2C_Read(FUSB302B_I2CADDR,REG_Status0,&val,1);
    if((val&0b00000011)==0){
        if(__FUSB302B_NOW_MEASCC==CC1){
            FUSB302B_Set_CC_MeasurementCH(CC2);
        }else FUSB302B_Set_CC_MeasurementCH(CC1);
        return ;
    }
    FUSB302B_PD_ConfigInterrupt();
}

/**
 * @brief 返回CC电压等级
 * @return  unsigned char
*/
unsigned char FUSB302B_Get_CC_MeasurementLv(void){
    unsigned char val;
    BSI2C_Read(FUSB302B_I2CADDR,REG_Status0,&val,1);
    unsigned char bc_lvl=val&0b00000011;
    return bc_lvl;
}

/**
 * @brief 设置CC比较电压 用于COMP
*/
unsigned char FUSB302B_Set_CC_COMP_mVotage(unsigned short mv){
    unsigned char val=mv/42;  // LSB is equivalent to 42 mV
    if(val>63)return 0;
    BSI2C_Write(FUSB302B_I2CADDR,REG_Measure,&val,1);
    return 1;
}

/**
 * @brief 检查VBUSOK
*/
unsigned char FUSB302B_Check_VBUSOK(void){
    unsigned char rr=0;
    BSI2C_Read(FUSB302B_I2CADDR,REG_Status0,&rr,1);
    return ((rr&0b10000000)>0)? 1:0; 
}

/**
 * @brief 开启检查VBUSOK
*/
void FUSB302B_Set_VBUS_Check(void){
    unsigned char wr=0b01110011;
    BSI2C_Write(FUSB302B_I2CADDR,REG_Measure,&wr,1);
}


/**
 * @brief 开启CC电压测量 
 * @param CCnum CC检测通道
*/
unsigned char FUSB302B_Set_CC_MeasurementCH(enum CC_X CCnum){
    unsigned char val;
    BSI2C_Read(FUSB302B_I2CADDR,REG_Switches0,&val,1);
    if(CCnum==CC2){
        val|= 0b00001000;
        val&=~0b00000100;
    }
    else if(CCnum==CC1){
        val|= 0b00000100;
        val&=~0b00001000;
    }
    BSI2C_Write(FUSB302B_I2CADDR,REG_Switches0,&val,1);
    start_timeout(30);
    __FUSB302B_NOW_MEASCC=CCnum;
    return 0;
}

/**
 * @brief 切换CC电压测量通道 
*/
unsigned char FUSB302B_Set_CC12_MeasurementExchange(){
    if(__FUSB302B_NOW_MEASCC==CC1)FUSB302B_Set_CC_MeasurementCH(CC2);
    else if(__FUSB302B_NOW_MEASCC==CC2)FUSB302B_Set_CC_MeasurementCH(CC1);
    return 0;
}

/**
 * @brief FUSB302B 完全复位 
*/
unsigned char FUSB302B_ResetAll(void){
    unsigned char val=0b00000011;
    if(BSI2C_Write(FUSB302B_I2CADDR,REG_Reset,&val,1)==1)return 1;
    return 0;
}

/**
 * @brief FUSB302B 初始化
*/
unsigned char FUSB302B_Init(void){
    USBC.Source_Capabilities_NUM=0;
    USBC.MessageID=0;
    unsigned char wchar=0;
    FUSB302B_ResetAll();
    HAL_Delay(10);
    
    wchar=0b00001111;
    BSI2C_Write(FUSB302B_I2CADDR,REG_Power,&wchar,1);
    //wchar=0b00000000;
    //BSI2C_Write(FUSB302B_I2CADDR,REG_Switches0,&wchar,1);
    wchar=0b11111111;
    BSI2C_Write(FUSB302B_I2CADDR,REG_Mask,&wchar,1);
    BSI2C_Write(FUSB302B_I2CADDR,REG_Maska,&wchar,1);
    BSI2C_Write(FUSB302B_I2CADDR,REG_Maskb,&wchar,1);
    return 0;
}

void FUSB302B_Retry_Wait(void)
{
    // Reset FUSB302
    FUSB302B_Init();
    USBC.State=usb_retry_wait;
    USBC.StateChanged_Flag=1;
    USBC.NeedToProcessMsg_Flag=0;
    USBC.Source_Capabilities_NUM=0;
    start_timeout(500);
}

/**
 * @brief 开始SNK模式
*/
unsigned char FUSB302B_StartSink(void){
    unsigned char wr=0b01100000;
    BSI2C_Write(FUSB302B_I2CADDR,REG_Slice,&wr,1);
    FUSB302B_Set_CC_MeasurementCH(CC1);
    USBC.State=usb_null;
    return 0;
}

/**
 * @brief 配置PD中断
*/
unsigned char FUSB302B_PD_ConfigInterrupt(void){
    unsigned char wr;
    wr=0b00000111;
    BSI2C_Write(FUSB302B_I2CADDR,REG_Control3,&wr,1);
    wr=0b10101111;
    BSI2C_Write(FUSB302B_I2CADDR,REG_Mask,&wr,1);
    wr=0b00000000;
    BSI2C_Write(FUSB302B_I2CADDR,REG_Maska,&wr,1);
    BSI2C_Write(FUSB302B_I2CADDR,REG_Maskb,&wr,1);
    wr=(__FUSB302B_NOW_MEASCC==CC1)?0b00000111:0b00001011;
    BSI2C_Write(FUSB302B_I2CADDR,REG_Switches0,&wr,1);
    wr=(__FUSB302B_NOW_MEASCC==CC1)?0b00100101:0b00100110;
    BSI2C_Write(FUSB302B_I2CADDR,REG_Switches1,&wr,1);
    wr=0b00000000;
    BSI2C_Write(FUSB302B_I2CADDR,REG_Control0,&wr,1);
    USBC.State=usb_default;
    USBC.StateChanged_Flag=1;
    start_timeout(500);
    return 0;
}

/**
 * @brief 读取FIFO数据到缓存
*/
unsigned char Read_Message(unsigned char* payload)
{
    // Read token and header
    unsigned char buf[3]={0,0,0};
    BSI2C_Read(FUSB302B_I2CADDR,REG_FIFOs,&buf[0],1);
    BSI2C_Read(FUSB302B_I2CADDR,REG_FIFOs,&buf[1],1);
    BSI2C_Read(FUSB302B_I2CADDR,REG_FIFOs,&buf[2],1);

    // Check for SOP token
    if ((buf[0] & 0xe0) != 0xe0) {
        //LOG("Flush RX FIFO\r\n");
        // Flush RX FIFO
        unsigned char wr=0;
        BSI2C_Read(FUSB302B_I2CADDR,REG_Control1,&wr,1);
        wr|=(1<<2);
        BSI2C_Write(FUSB302B_I2CADDR,REG_Control1,&wr,1);
        return 0;
    }
    USBC.Header_Raw=buf[2]*0x100+buf[1];
    unsigned char len = ((USBC.Header_Raw >> 12) & 0x07)*4+4;
    if(len>0)BSI2C_Read(FUSB302B_I2CADDR,REG_FIFOs,payload,len);
    USBC.NeedToProcessMsg_Flag=1;
    return len;
}

/**
 * @brief 数据接收检查
*/
unsigned char FUSB302B_PD_MsgCheck(){
    while (1)
    {
        unsigned char status1;
        BSI2C_Read(FUSB302B_I2CADDR,REG_Status1,&status1,1);
        if((status1&0b00100000)>0){
            //LOG("Msg FIFO empty.\r\n");
            break;
        }
        if(USBC.State==usb_default){
            USBC.State=usb_pd;
            is_timeout_active = 0; 
            USBC.StateChanged_Flag=1;
        }
        Read_Message(USBC.Rx_Buf_Raw);
    }
    return 0;
}

/**
 * @brief PD中断处理
*/
unsigned char FUSB302B_PD_CheckInterrupt(void){
    
    unsigned char Interrupt;
    unsigned char Interrupta;
    unsigned char Interruptb;
    unsigned char msg_get=0;
    BSI2C_Read(FUSB302B_I2CADDR,REG_Interrupt,&Interrupt,1);
    //LOG("Interrupt. 0x%2x\r\n",Interrupt);
    BSI2C_Read(FUSB302B_I2CADDR,REG_Interrupta,&Interrupta,1);
    //LOG("Interrupta. 0x%2x\r\n",Interrupta);
    BSI2C_Read(FUSB302B_I2CADDR,REG_Interruptb,&Interruptb,1);
    //LOG("Interruptb. 0x%2x\r\n",Interruptb);
    if((Interrupta&0b00000001)>0){
        //LOG("Need to reset.\r\n");
        //FUSB302B_Retry_Wait();
        while (1);
        return 0;
    }
    /*
    if((Interrupta&0b00010000)>0){
        LOG("Retry failed.\r\n");
    }
    if((Interrupta&0b00000100)>0){
        LOG("TX ack.\r\n");
    }*/
    if((Interrupt&0b01000000)>0){
        msg_get=1;
    }
    /*
    if((Interrupt&0b00010000)>0){
        LOG("CRC ok.\r\n");
    }
    if((Interruptb&0b00000001)>0){
        LOG("Good CRC sent.\r\n");
    }*/
    if(msg_get){
        FUSB302B_PD_MsgCheck();
    }
    return 0;
}

static const char* const PRODUCT_IDS[] = { "FUSB302BMPX/VMPX/UCX", "FUSB302B01MPX", "FUSB302B10MPX", "FUSB302B11MPX" };
static const char* VERSIONS = "????????ABCDEFGH";
char * FUSB302B_ID_Str[48];
/**
 * @brief FUSB302B读取ID
*/
unsigned char FUSB302B_READ_ID(void)
{
    unsigned char device_id;
    if(BSI2C_Read(FUSB302B_I2CADDR,REG_DeviceID,&device_id,1)==1)return 1;
    unsigned char version_id = device_id >> 4;
    unsigned char product_id = (device_id >> 2) & 0x03;
    unsigned char revision_id = device_id & 0x03;

    strcpy((char*)FUSB302B_ID_Str, PRODUCT_IDS[product_id]);
    char piece[8] = " ._rev.";
    piece[1] = VERSIONS[version_id];
    piece[6] = 'A' + revision_id;
    strcat((char*)FUSB302B_ID_Str, piece);
 
    return 0;
}

/**
 * @brief FUSB302B发送PD信息
*/
void FUSB302B_Send_PDMessage(unsigned short *Header,unsigned int *Do,unsigned char DoNum){
    unsigned char wchar=0b00001111;
    BSI2C_Write(FUSB302B_I2CADDR,REG_Power,&wchar,1);
    wchar=0x40;
    BSI2C_Write(FUSB302B_I2CADDR,REG_Control0,&wchar,1);
    unsigned char buf[40];
    unsigned char buf_cnt=0;
    unsigned char data_num=6;
    buf[buf_cnt]=sop1;buf_cnt++;
    buf[buf_cnt]=sop1;buf_cnt++;
    buf[buf_cnt]=sop1;buf_cnt++;
    buf[buf_cnt]=sop2;buf_cnt++;
    buf[buf_cnt]=packsym|data_num;buf_cnt++;
    buf[buf_cnt]=(*Header)&0xff;
    buf_cnt++;
    buf[buf_cnt]=(*Header)>>8;
    buf_cnt++;
    if(DoNum>0){
        buf[buf_cnt]=(*Do)&0xff;
        buf_cnt++;
        buf[buf_cnt]=((*Do)>>8)&0xff;
        buf_cnt++;
        buf[buf_cnt]=((*Do)>>16)&0xff;
        buf_cnt++;
        buf[buf_cnt]=((*Do)>>24)&0xff;
        buf_cnt++;
    }
    buf[buf_cnt]=jam_crc;buf_cnt++;
    buf[buf_cnt]=eop;buf_cnt++;
    buf[buf_cnt]=txoff;buf_cnt++;
    buf[buf_cnt]=txon;buf_cnt++;
    BSI2C_Write(FUSB302B_I2CADDR,REG_FIFOs,buf,buf_cnt);
    //LOG("Message send done [%d].\r\n",buf_cnt);
}
