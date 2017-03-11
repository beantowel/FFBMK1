/**
  ******************************************************************************
  * @file    usbd_pid.h
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_PID_H
#define __USB_PID_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

//PID usages from Device Class Definition for Physical Interface Devices (PID) Version 1.0
//#define PID_Undefined 0x00
//#define PID_Physical_Interface_Device 0x01
//#define PID_Normal 0x20
//	 
//#define PID_Set_Effect_Report 0x21
//#define PID_Effect_Block_Index 0x22
//#define PID_Parameter_Block_Offset 0x23
//#define PID_ROM_Flag 0x24	 
//#define PID_Effect_Type 0x25
//#define PID_ET_Constant_Force 0x26
//#define PID_ET_Ramp 0x27
//#define PID_ET_Custom_Force_Data 0x28
//#define PID_ET_Square 0x30
//#define PID_ET_Sine 0x31
//#define PID_ET_Triangle 0x32
//#define PID_ET_Sawtooth_Up 0x33
//#define PID_ET_Sawtooth_Down 0x34
//#define PID_ET_Spring 0x40
//#define PID_ET_Damper 0x41
//#define PID_ET_Inertia 0x42
//#define PID_ET_Friction 0x43
//#define PID_Duration 0x50
//#define PID_Sample_Period 0x51
//#define PID_Gain 0x52
//#define PID_Trigger_Button 0x53
//#define PID_Trigger_Repeat_Interval 0x54
//#define PID_Axes_Enable 0x55
//#define PID_Direction_Enable 0x56
//#define PID_Direction 0x57
//#define PID_Type_Specific_Block_Offset 0x58
//#define PID_Block_Type 0x59

//#define PID_Set_Envelope_Report 0x5A
//#define PID_Attack_Level 0x5B
//#define PID_Attack_Time 0x5C
//#define PID_Fade_Level 0x5D
//#define PID_Fade_Time 0x5E

//#define PID_Set_Condition_Report 0x5F
//#define PID_CP_Offset 0x60
//#define PID_Positive_Coefficient 0x61
//#define PID_Negative_Coefficient 0x62
//#define PID_Positive_Saturation 0x63
//#define PID_Negative_Saturation 0x64
//#define PID_Dead_Band 0x65

//#define PID_Download_Force_Sample 0x66
//#define PID_Isoch_Custom_Force_Enable 0x67
//#define PID_Custom_Force_Data_Report 0x68
//#define PID_Custom_Force_Data 0x69
//#define PID_Custom_Force_Vendor_Defined_Data 0x6A

//#define PID_Set_Custom_Force_Report 0x6B
//#define PID_Custom_Force_Data_Offset 0x6C
//#define PID_Sample_Count 0x6D

//#define PID_Set_Periodic_Report 0x6E
//#define PID_Offset 0x6F
//#define PID_Magnitude 0x70
//#define PID_Period 0x72

//#define PID_Set_Constant_Force_Report 0x73

//#define PID_Set_Ramp_Force_Report 0x74
//#define PID_Ramp_Start 0x75
//#define PID_Ramp_End 0x76

//#define PID_Effect_Operation_Report 0x77
//#define PID_Effect_Operation 0x78
//#define PID_Op_Effect_Start 0x79
//#define PID_Op_Effect_Start_Solo 0x7A
//#define PID_Op_Effect_Stop 0x7B
//#define PID_Loop_Count 0x7C

//#define PID_Device_Gain_Report 0x7D
//#define PID_Device_Gain 0x7E

//#define PID_PID_Pool_Report 0x7F
//#define PID_RAM_Pool_Size 0x80
//#define PID_ROM_Pool_Size 0x81
//#define PID_ROM_Effect_Block_Count 0x82
//#define PID_Simultaneous_Effects_Max 0x83
//#define PID_Pool_Alignment 0x84

//#define PID_PID_Pool_Move_Report 0x85
//#define PID_Move_Source 0x86
//#define PID_Move_Destination 0x87
//#define PID_Move_Length 0x88

//#define PID_PID_Block_Load_Report 0x89
//#define PID_Block_Load_Status 0x8B
//#define PID_Block_Load_Success 0x8C
//#define PID_Block_Load_Full 0x8D
//#define PID_Block_Load_Error 0x8E
//#define PID_Block_Handle 0x8F

//#define PID_PID_Block_Free_Report 0x90
//#define PID_Type_Specific_Block_Handle 0x91
//#define PID_PID_State_Report 0x92
//#define PID_Effect_Playing 0x94

//#define PID_PID_Device_Control_Report 0x95
//#define PID_PID_Device_Control 0x96
//#define PID_DC_Enable_Actuators 0x97
//#define PID_DC_Disable_Actuators 0x98
//#define PID_DC_Stop_All_Effects 0x99
//#define PID_DC_Device_Reset 0x9A
//#define PID_DC_Device_Pause 0x9B
//#define PID_DC_Device_Continue 0x9C
//#define PID_Device_Paused 0x9F
//#define PID_Actuators_Enabled 0xA0
//#define PID_Safety_Switch 0xA4
//#define PID_Actuator_Override_Switch 0xA5
//#define PID_Actuator_Power 0xA6
//#define PID_Start_Delay 0xA7
//#define PID_Parameter_Block_Size 0xA8
//#define PID_Device_Managed_Pool 0xA9
//#define PID_Shared_Parameter_Blocks 0xAA

//#define PID_Create_New_Effect_Report 0xAB
//#define PID_RAM_Pool_Available 0xAC

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */ 
uint8_t USBD_PID_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
/**
  * @}
  */ 

#ifdef __cplusplus
}
#endif

#endif  /* __USB_PID_H */
