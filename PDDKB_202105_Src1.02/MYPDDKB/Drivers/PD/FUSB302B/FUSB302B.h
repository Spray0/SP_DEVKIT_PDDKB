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

#ifndef __FUSB302B_H
#define __FUSB302B_H

#include "bsi2c.h"


#define FUSB302B_I2CADDR 0x22 // FUSB302B I2C 地址
extern unsigned char is_timeout_active;
/* FUSB302B 寄存器 */
enum FUSB302B_RegAddr{
    REG_DeviceID=0x01,
    REG_Switches0,
    REG_Switches1,
    REG_Measure,
    REG_Slice,
    REG_Control0,
    REG_Control1,
    REG_Control2,
    REG_Control3,
    REG_Mask,
    REG_Power,
    REG_Reset,
    REG_OCPreg,
    REG_Maska,
    REG_Maskb,
    REG_Control4,
    REG_Status0a=0x3C,
    REG_Status1a=0x3D,
    REG_Interrupta=0x3E,
    REG_Interruptb=0X3F,
    REG_Status0=0x40,
    REG_Status1=0x41,
    REG_Interrupt=0x42,
    REG_FIFOs=0x43
};

enum CC_X{
    CCNULL=0,
    CC1,
    CC2
};

/// Tokens used in FUSB302B FIFO
enum token{
    txon = 0xa1,
    sop1 = 0x12,
    sop2 = 0x13,
    sop3 = 0x1b,
    reset1 = 0x15,
    reset2 = 0x16,
    packsym = 0x80,
    jam_crc = 0xff,
    eop = 0x14,
    txoff = 0xfe
};

extern enum CC_X __FUSB302B_NOW_MEASCC;

unsigned char FUSB302B_ResetAll(void);
unsigned char FUSB302B_Init(void);
unsigned char FUSB302B_Set_CC_MeasurementCH(enum CC_X CCnum);
unsigned char FUSB302B_Get_CC_MeasurementLv(void);
unsigned char FUSB302B_Set_CC12_MeasurementExchange();
void FUSB302B_Check_CC_Meas(void);
unsigned char FUSB302B_Is_CC_OK(void);
void FUSB302B_Retry_Wait(void);
unsigned char FUSB302B_Set_CC_COMP_mVotage(unsigned short mv);
unsigned char FUSB302B_PD_ConfigInterrupt(void);
unsigned char FUSB302B_PD_CheckInterrupt(void);
unsigned char FUSB302B_Check_VBUSOK(void);
void FUSB302B_Set_VBUS_Check(void);

void FUSB302B_Send_PDMessage(unsigned short *Header,unsigned int *Do,unsigned char DoNum);

extern char * FUSB302B_ID_Str[48];
unsigned char FUSB302B_READ_ID(void);


unsigned char FUSB302B_StartSink(void);

void start_timeout(unsigned int ms);
unsigned char has_timeout_expired(void);

#endif