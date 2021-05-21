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

#include "pd_ufp.h"

struct __USBC USBC;

/**
 * @brief PD_UFP初始化
*/
unsigned char PD_UFP_Init(void){
    USBC.StateChanged_Flag=0;
    USBC.NeedToProcessMsg_Flag=0;
    FUSB302B_Init();
    FUSB302B_StartSink();
    return 0;
}

/**
 * @brief PD 电源数据 处理
*/
void PD_SrcCapMsg_Handle(unsigned char num){
    unsigned char cnt;
    USBC.Source_Capabilities_NUM=0;
    for(cnt=0;cnt<num;cnt++){
        USBC.DO_Raw[cnt]=*((unsigned int *)&USBC.Rx_Buf_Raw[cnt*4]);
        unsigned char type=(unsigned char)(USBC.DO_Raw[cnt]>>30);
        if(type==0){
            PD_FixedSupply_PDO_Process(&USBC.DO_Raw[cnt],&USBC.Source_Capabilities[cnt]);
            USBC.Source_Capabilities_NUM++;
        }
    }
}

/**
 * @brief PD_UFP 处理
*/
unsigned char PD_UFP_InLoop(void){
    // 中断处理
    if(BSINT_CUR())
    {
        #ifdef USE_LOG
            //LOG("[%d]============Interrupt============\r\n",BSTimeStamp);
        #endif
        FUSB302B_PD_CheckInterrupt();
    }
    // 事件超时处理
    if(has_timeout_expired()){
        switch (USBC.State)
        {
        case usb_null:
            FUSB302B_Check_CC_Meas();
            break;
        case usb_default:
            FUSB302B_Retry_Wait();
            #ifdef USE_LOG
                LOG("%lu: No CC activity,Reset...\r\n",BSTimeStamp);
            #endif
            break;
        case usb_retry_wait:
            FUSB302B_StartSink();
            break;
        default:
            break;
        }        
    }
    // 线缆拔出检查
    if(USBC.State==usb_pd_ready){
        if(FUSB302B_Check_VBUSOK()==0){
            #ifdef USE_LOG
                LOG("The cable has been pulled out\r\n");
            #endif
            FUSB302B_Retry_Wait();
        }
    }
    // 需要处理信息数据
    if(USBC.NeedToProcessMsg_Flag){
        USBC.NeedToProcessMsg_Flag=0;
        PD_MessageHeader_Process(&USBC.Header_Raw,&USBC.TempHeader);
        #ifdef USE_LOG
            //__LOG_PD_MessageInfo(USBC.TempHeader);
        #endif
        if(USBC.TempHeader.NumberOfDataObjects>0){
            switch (USBC.TempHeader.MessageType)
            {
            case __DMsg_Source_Capabilities:
                PD_SrcCapMsg_Handle((unsigned char)(USBC.TempHeader.NumberOfDataObjects));
                #ifdef USE_LOG
                    LOG("GET < __DMsg_Source_Capabilities\r\n");
                    __LOG_PD_Print_All_FSPDOInfo();
                #endif
                break;
            default:
                break;
            }
        }else{
            switch (USBC.TempHeader.MessageType)
            {
            case __CMsg_GoodCRC:
                #ifdef USE_LOG
                    LOG("GET < __CMsg_GoodCRC\r\n");
                #endif
                break;
            case __CMsg_Accept:
                #ifdef USE_LOG
                    LOG("GET < __CMsg_Accept\r\n");
                #endif
                break;
            case __CMsg_PS_RDY:
                #ifdef USE_LOG
                    LOG("GET < __CMsg_PS_RDY\r\n");
                    LOG("USBC PD Ready.\r\n");
                #endif
                if(USBC.State==usb_pd){
                    USBC.State=usb_pd_ready;
                    FUSB302B_Set_VBUS_Check();
                }
                USBC.Source_NowIndex=USBC.Source_RequestIndex;
                break;
            default:
                break;
            }
        }
    }
    // 如果USBC状态发生了改变
    if(USBC.StateChanged_Flag){
        USBC.StateChanged_Flag=0;
        switch (USBC.State)
        {
        case usb_null:
            break;
        case usb_default:
            #ifdef USE_LOG
                LOG("SNK detected. CC%d\r\n",__FUSB302B_NOW_MEASCC);
            #endif
            break;
        case usb_pd:
            #ifdef USE_LOG
                LOG("PD detected.\r\n");
            #endif
            if(INIT_PD_PWR==INIT_PD_SAFE5V)
            PD_RequestPower(USBC.Source_Capabilities[0].Voltage);
            if(INIT_PD_PWR==INIT_PD_MAXPWR)
            PD_RequestPower(USBC.Source_Capabilities[USBC.Source_Capabilities_NUM-1].Voltage);
            break;
        case usb_retry_wait:
            #ifdef USE_LOG
                LOG("Reset.\r\n");
            #endif
            break;
        default:
            break;
        }
    }
    
    return 0;
}


/**
 * @brief   PD 信息头处理
 */
void PD_MessageHeader_Process(unsigned short *header_raw,struct PD_Message_Header *Header){
    Header->rawdata=*header_raw;
    Header->Extended=((Header->rawdata)>> 15) & 0x01;
    Header->MessageID=((Header->rawdata)>> 9) & 0x07;
    Header->NumberOfDataObjects=((Header->rawdata)>> 12) & 0x07;
    Header->SpecificationRevision=((Header->rawdata)>> 6) & 0x03;
    Header->PortPowerRole=((Header->rawdata)>> 8) & 0x01;
    Header->PortDataRole=((Header->rawdata)>> 5) & 0x01;
    Header->MessageType=(Header->rawdata)&0x1F;
}


/**
 * @brief LOG打印 PD信息头信息
*/
void __LOG_PD_MessageInfo(struct PD_Message_Header header){
    
    LOG("PD_Message Header=0x%4x\r\n",header.rawdata);
    LOG("   Stamp      : %d\r\n",header.MessageID);
    LOG("   Power_Role : ");
    (header.PortPowerRole==__PPR_Sink)?LOG("Sink\r\n"):LOG("Source\r\n");
    LOG("   Data_Role  : ");
    (header.PortDataRole ==__PDR_DFP)?LOG("DFP\r\n"):LOG("UFP\r\n");
    LOG("   PD_Version : ");
    if(header.SpecificationRevision==__Revision_1_0)LOG("1.0\r\n");
    else if(header.SpecificationRevision==__Revision_2_0)LOG("2.0\r\n");
    else if(header.SpecificationRevision==__Revision_3_0)LOG("3.0\r\n");
    LOG("   Type       : ");
    if(header.Extended==__EMSG){
        LOG("Extended message\r\n");
    }else{
        if(header.NumberOfDataObjects==0){
            LOG("Control message\r\n");
            LOG("   Msg_Type   : ");
            switch (header.MessageType)
            {
            case __CMsg_GoodCRC: LOG("GoodCRC\r\n"); break;
            case __CMsg_GotoMin: LOG("GotoMin\r\n"); break;
            case __CMsg_Accept: LOG("Accept\r\n"); break;
            case __CMsg_Reject: LOG("Reject\r\n"); break;
            case __CMsg_Ping: LOG("Ping\r\n"); break;
            case __CMsg_PS_RDY: LOG("PS_RDY\r\n"); break;
            case __CMsg_Get_Source_Cap: LOG("Get_Source_Cap\r\n"); break;
            case __CMsg_Get_Sink_Cap: LOG("Get_Sink_Cap\r\n"); break;
            case __CMsg_DR_Swap: LOG("DR_Swap\r\n"); break;
            case __CMsg_PR_Swap: LOG("PR_Swap\r\n"); break;
            case __CMsg_VCONN_Swap: LOG("VCONN_Swap\r\n"); break;
            case __CMsg_Wait: LOG("Wait\r\n"); break;
            case __CMsg_Soft_Reset: LOG("Soft_Reset\r\n"); break;
            case __CMsg_Data_Reset: LOG("Data_Reset\r\n"); break;
            case __CMsg_Data_Reset_Complete: LOG("Data_Reset_Complete\r\n"); break;
            case __CMsg_Not_Supported: LOG("Not_Supported\r\n"); break;
            case __CMsg_Get_Source_Cap_Extended: LOG("Get_Source_Cap_Extended\r\n"); break;
            case __CMsg_Get_Status: LOG("Get_Status\r\n"); break;
            case __CMsg_FR_Swap: LOG("FR_Swap\r\n"); break;
            case __CMsg_Get_PPS_Status: LOG("Get_PPS_Status\r\n"); break;
            case __CMsg_Get_Country_Codes: LOG("Get_Country_Codes\r\n"); break;
            case __CMsg_Get_Sink_Cap_Extended: LOG("Get_Sink_Cap_Extended\r\n"); break;
            default: break;
            }
        }else{
            LOG("Data message\r\n");
            LOG("   DataObjNum : %d\r\n",header.NumberOfDataObjects);
            LOG("   Msg_Type   : ");
            switch (header.MessageType)
            {
            case __DMsg_Source_Capabilities: LOG("Source_Capabilities\r\n"); break;
            case __DMsg_Request: LOG("Request\r\n"); break;
            case __DMsg_BIST: LOG("BIST\r\n"); break;
            case __DMsg_Sink_Capabilities: LOG("Sink_Capabilities\r\n"); break;
            case __DMsg_Battery_Status: LOG("Battery_Status\r\n"); break;
            case __DMsg_Alert: LOG("Alert\r\n"); break;
            default: break;
            }
        }
    }
}

/**
 * @brief   PD 固定数据对象处理
 */
void PD_FixedSupply_PDO_Process(unsigned int *pdo_raw,struct PD_PDO_FixedSupply *pdo){
    pdo->rawdata=*pdo_raw;
    pdo->DualRole_Power=(((pdo->rawdata)>>29)&0x01)>0?1:0;
    pdo->USB_Suspend_Supported=(((pdo->rawdata)>>28)&0x01)>0?1:0;
    pdo->Unconstrained_Power=(((pdo->rawdata)>>27)&0x01)>0?1:0;
    pdo->USB_Communications_Capable=(((pdo->rawdata)>>26)&0x01)>0?1:0;
    pdo->DualRole_Data=(((pdo->rawdata)>>25)&0x01)>0?1:0;
    pdo->Unchunked_Extended_Messages_Supported=(((pdo->rawdata)>>24)&0x01)>0?1:0;
    pdo->Peak_Current=((pdo->rawdata)>>20)&0x03;
    pdo->Voltage=(((pdo->rawdata)>>10)&0x3ff)*50;
    pdo->Current=((pdo->rawdata)&0x3ff)*10;
}

/**
 * @brief LOG打印 PD固定输出电源数据对象
*/
void __LOG_PD_FSPDO_Info(struct PD_PDO_FixedSupply pdo){
    //LOG("FixedSupply PDO=0x%8x\r\n",pdo.rawdata);
    LOG("%5dmv %5dma\r\n",pdo.Voltage,pdo.Current);
}

/**
 * @brief LOG打印所有 PD固定输出电源数据对象
*/
void __LOG_PD_Print_All_FSPDOInfo(void){
    if(USBC.Source_Capabilities_NUM>0){
        unsigned char cnt;
        LOG("%d FSPDO Get:\r\n",USBC.Source_Capabilities_NUM);
        for(cnt=0;cnt<USBC.Source_Capabilities_NUM;cnt++)
        {
            //LOG("FixedSupply PDO[%d] = 0x%8x\r\n",cnt,USBC.Source_Capabilities[cnt].rawdata);
            LOG("   [%d] ",cnt+1);
            __LOG_PD_FSPDO_Info(USBC.Source_Capabilities[cnt]);
        }
    }else{
        LOG("No FSPDO.\r\n");
    }
}

/**
 * @brief 计算PD数据头原始数据
*/
void PD_MakeHeaderRaw(struct PD_Message_Header *Header){
    Header->rawdata=0;
    Header->rawdata|=((Header->NumberOfDataObjects)<<12);
    Header->rawdata|=((Header->MessageID)<<9);
    Header->rawdata|=((Header->PortPowerRole)<<8);
    Header->rawdata|=((Header->SpecificationRevision)<<6);
    Header->rawdata|=((Header->PortDataRole)<<5);
    Header->rawdata|=(Header->MessageType);
}
/**
 * @brief 计算PD RDO 对象原始数据
*/
void PD_MakeRDORaw(struct PD_RDO_FixedSupply *RDO){
    RDO->rawdata=0;
    RDO->rawdata|=((RDO->ObjectPosition)<<28);
    RDO->rawdata|=((RDO->GiveBack_Flag)<<27);
    RDO->rawdata|=((RDO->Capability_Mismatch)<<26);
    RDO->rawdata|=((RDO->USB_Communications_Capable)<<25);
    RDO->rawdata|=((RDO->No_USBSuspend)<<24);
    RDO->rawdata|=((RDO->Unchunked_Extended_Messages_Supported)<<23);
    RDO->rawdata|=((RDO->OperatingCurrent/10)<<10);
    RDO->rawdata|=(RDO->Max_OperatingCurrent/10);
}

/**
 * @brief 请求PD固定输出电源数据对象
*/
void PD_RequestPower(unsigned short voltage){
    UART1_Printf("reqv=%d\r\n",voltage);
    unsigned char cnt=0;
    unsigned char index=0;
    for(cnt=0;cnt<USBC.Source_Capabilities_NUM;cnt++){
        if(voltage==USBC.Source_Capabilities[cnt].Voltage){
            index=cnt+1;
            break;
        }
    }
    if(index==0){
        #ifdef USE_LOG
            LOG("Unsupported voltage,set to be safe_5v.\r\n");
        #endif
        index=1;
    }
    //创建信息头
    struct PD_Message_Header RequestHeader={
        .rawdata=0,
        .Extended=__CMSG_DMSG,
        .NumberOfDataObjects=1,
        .MessageID=USBC.MessageID,
        .PortPowerRole=__PPR_Sink,
        .SpecificationRevision=__Revision_3_0,
        .PortDataRole=__PDR_UFP,
        .MessageType=__DMsg_Request,
    };
    UART1_Printf("msgid=%d\r\n",USBC.MessageID);
    USBC.MessageID++;
    if(USBC.MessageID==8)USBC.MessageID=0;
    PD_MakeHeaderRaw(&RequestHeader);
    //创建RDO
    struct PD_RDO_FixedSupply RequestDO={
        .rawdata=0,
        .ObjectPosition=index,
        .GiveBack_Flag=0,
        .Capability_Mismatch=0,
        .USB_Communications_Capable=1,
        .No_USBSuspend=1,
        .Unchunked_Extended_Messages_Supported=0,
        .OperatingCurrent=USBC.Source_Capabilities[index-1].Current,
        .Max_OperatingCurrent=USBC.Source_Capabilities[index-1].Current+5,
    };
    PD_MakeRDORaw(&RequestDO);
    FUSB302B_Send_PDMessage(&RequestHeader.rawdata,&RequestDO.rawdata,RequestHeader.NumberOfDataObjects);
    USBC.Source_RequestIndex=index;
    #ifdef USE_LOG
        LOG("Request power (%dmv,%dma).\r\n",USBC.Source_Capabilities[index-1].Voltage,RequestDO.OperatingCurrent);
    #endif
}

