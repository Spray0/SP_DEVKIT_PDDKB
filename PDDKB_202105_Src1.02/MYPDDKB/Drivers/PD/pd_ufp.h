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

#ifndef __PD_UFP_H
#define __PD_UFP_H
#include "main.h"

#define USE_LOG 1

#define INIT_PD_SAFE5V 0            // 初始化PD电源安全5V
#define INIT_PD_MAXPWR 1            // 初始化PD电源最高档
#define INIT_PD_PWR INIT_PD_SAFE5V  // 初始化PD电源选择

/* USB 接口状态 */
enum USBC_STATE{
    usb_null,       //状态：无线缆插入,等待线缆插入
    usb_default,    //状态：线缆插入，等待事件触发
    usb_pd,         //状态：线缆插入，PD事件已触发，等待电源配置
    usb_pd_ready,   //状态：线缆插入，电源配置完毕
    usb_retry_wait
};
/* USB 接口插入正反方向 */
enum USBC_CABLE_SIDE{
    NULL_SIDE,
    FRONT_SIDE,
    REVERSE_SIDE
};

unsigned char PD_UFP_Init(void);
unsigned char PD_UFP_InLoop(void);


/*
    PD信息头 信息类型
*/
enum PD_Header_MessageType{
    __MsgReserved,
    __CMsg_GoodCRC,
    __CMsg_GotoMin,
    __CMsg_Accept,
    __CMsg_Reject,
    __CMsg_Ping,
    __CMsg_PS_RDY,
    __CMsg_Get_Source_Cap,
    __CMsg_Get_Sink_Cap,
    __CMsg_DR_Swap,
    __CMsg_PR_Swap,
    __CMsg_VCONN_Swap,
    __CMsg_Wait,
    __CMsg_Soft_Reset,
    __CMsg_Data_Reset,
    __CMsg_Data_Reset_Complete,
    __CMsg_Not_Supported,
    __CMsg_Get_Source_Cap_Extended,
    __CMsg_Get_Status,
    __CMsg_FR_Swap,
    __CMsg_Get_PPS_Status,
    __CMsg_Get_Country_Codes,
    __CMsg_Get_Sink_Cap_Extended,
    __DMsg_Source_Capabilities=1,
    __DMsg_Request,
    __DMsg_BIST,
    __DMsg_Sink_Capabilities,
    __DMsg_Battery_Status,
    __DMsg_Alert
};
/*
    PD规格版本
*/
enum PD_Header_SpecificationRevision{
    __Revision_1_0,
    __Revision_2_0,
    __Revision_3_0,
    __Revision_Reserved
};
/* 
    端口电源角色
*/
enum PD_Header_PortPowerRole{
    __PPR_Sink,
    __PPR_Source
};
/* 
    端口数据角色 
*/
enum PD_Header_PortDataRole{
    __PDR_UFP,
    __PDR_DFP
};
/* 
    其他类型标记 
*/
enum PD_Header_Extended{
    __CMSG_DMSG,    // Control Message or Data Message
    __EMSG          // Extended Message
};
/* 
    PD 信息头结构 
*/
struct PD_Message_Header{
    /* 原始数据 */
    unsigned short rawdata;
    /* 是否是其他类型数据包 */
    enum PD_Header_Extended Extended;
    /* 数据对象数量  0 ：Control Message 1-7：Data Message */
    unsigned char NumberOfDataObjects;
    /* 信息ID Stamp 0-7 复位值0，自增 */
    unsigned char MessageID;
    /* 端口电源角色 */
    enum PD_Header_PortPowerRole PortPowerRole;
    /* PD规格版本 */
    enum PD_Header_SpecificationRevision SpecificationRevision;
    /* 端口数据角色 */
    enum PD_Header_PortDataRole PortDataRole;
    /* 信息类型 */
    enum PD_Header_MessageType MessageType;
};

/**
 * @brief 接收电源数据对象 固定供电
 */
struct PD_PDO_FixedSupply{
    /* 原始数据 */
    unsigned int rawdata;
    /* 双口电源 */
    unsigned char DualRole_Power;
    /* USB 挂起支持 */
    unsigned char USB_Suspend_Supported;
    /* 无约束电源 */
    unsigned char Unconstrained_Power;
    /* USB通信能力 */
    unsigned char USB_Communications_Capable;
    /* 双口数据 */
    unsigned char DualRole_Data;
    /* 扩展信息支持 */
    unsigned char Unchunked_Extended_Messages_Supported;
    /* 峰值电流 */
    unsigned char Peak_Current;
    /* 电压mv */
    unsigned short Voltage;
    /* 电流ma */
    unsigned short Current;
};

/**
 * @brief 发送电源数据对象 固定供电
 */
struct PD_RDO_FixedSupply{
    /* 原始数据 */
    unsigned int rawdata;
    /* PDO对象索引 */
    unsigned char ObjectPosition;
    /* 回馈标记 */
    unsigned char GiveBack_Flag;
    /* 错误配置能力 */
    unsigned char Capability_Mismatch;
    /* USB通信能力 */
    unsigned char USB_Communications_Capable;
    /* 无USB挂起 */
    unsigned char No_USBSuspend;
    /* 扩展信息支持 */
    unsigned char Unchunked_Extended_Messages_Supported;
    /* 工作电流 */
    unsigned short OperatingCurrent;
    /* 最大工作电流 */
    unsigned short Max_OperatingCurrent;
};

/* USB 接口*/
struct __USBC
{
    enum USBC_STATE State;
    unsigned char StateChanged_Flag;
    unsigned char NeedToProcessMsg_Flag;
    enum USBC_CABLE_SIDE Side;
    struct PD_PDO_FixedSupply Source_Capabilities[7];
    struct PD_Message_Header TempHeader;
    unsigned char  Source_Capabilities_NUM;
    unsigned char  Source_NowIndex;
    unsigned char  Source_RequestIndex;
    unsigned int DO_Raw[8];
    unsigned short Header_Raw;
    unsigned char Rx_Buf_Raw[64];
    unsigned char MessageID;
};
extern struct __USBC USBC;




void PD_MessageHeader_Process(unsigned short *header_raw,struct PD_Message_Header *Header);
void __LOG_PD_MessageInfo(struct PD_Message_Header header);
void PD_FixedSupply_PDO_Process(unsigned int *pdo_raw,struct PD_PDO_FixedSupply *pdo);
void __LOG_PD_FSPDO_Info(struct PD_PDO_FixedSupply pdo);
void __LOG_PD_Print_All_FSPDOInfo(void);
void PD_RequestPower(unsigned short voltage);
#endif