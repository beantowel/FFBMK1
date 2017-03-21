/**
  ******************************************************************************
  * @file    usbd_hid.c
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   This file provides the HID core functions.
  *
  * @verbatim
  *
  *          ===================================================================
  *                                HID Class  Description
  *          ===================================================================
  *           This module manages the HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - The Mouse protocol
  *             - Usage Page : Generic Desktop
  *             - Usage : Joystick
  *             - Collection : Application
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_hid.h"
#include "usbd_pid.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_HID
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_HID_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_HID_Private_Defines
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_HID_Private_Macros
  * @{
  */
/**
  * @}
  */




/** @defgroup USBD_HID_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_HID_Init (USBD_HandleTypeDef *pdev,
                               uint8_t cfgidx);

static uint8_t  USBD_HID_DeInit (USBD_HandleTypeDef *pdev,
                                 uint8_t cfgidx);

static uint8_t  USBD_HID_Setup (USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req);

static uint8_t  *USBD_HID_GetCfgDesc (uint16_t *length);

static uint8_t  *USBD_HID_GetDeviceQualifierDesc (uint16_t *length);

static uint8_t  USBD_HID_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_HID_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t USBD_HID_EP0_RxReady (USBD_HandleTypeDef *pdev);
/**
  * @}
  */

/** @defgroup USBD_HID_Private_Variables
  * @{
  */

USBD_ClassTypeDef  USBD_HID =
{
  USBD_HID_Init,
  USBD_HID_DeInit,
  USBD_HID_Setup,
  NULL, /*EP0_TxSent*/
  USBD_HID_EP0_RxReady, /*EP0_RxReady*/
  USBD_HID_DataIn, /*DataIn*/
  USBD_HID_DataOut, /*DataOut*/
  NULL, /*SOF */
  NULL, /*IsoINIncomplete*/
  NULL, /*IsoOUTIncomplete*/
  USBD_HID_GetCfgDesc, /*HSConfigDescriptor*/
  USBD_HID_GetCfgDesc, /*FSConfigDescriptor*/
  USBD_HID_GetCfgDesc, /*OtherSpeedConfigDescriptor*/
  USBD_HID_GetDeviceQualifierDesc,
};

/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_CfgDesc[USB_HID_CONFIG_DESC_SIZ]  __ALIGN_END =
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_HID_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x01,         /*bNumInterfaces: 1 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
  0xA0,         /*bmAttributes: bus powered and Support Remote Wake-up (2016.8.13:CubeMXbug:originaly set as 0xE0)*/
  0x96,         /*MaxPower 300 mA: this current is used for detecting Vbus*/

  /************** Descriptor of FFBMK1 interface ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  0x00,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x02,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: HID*/
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of Joystick Mouse HID ********************/
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  LOBYTE(PID_FFBMK1_DESC_SIZE),/*wItemLength: Total length of Report descriptor*/
  HIBYTE(PID_FFBMK1_DESC_SIZE),
  /******************** Descriptor of Input endpoint ********************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

  HID_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  HID_EPIN_SIZE, /*wMaxPacketSize: 4 Byte max */
  0x00,
  HID_FS_BINTERVAL,          /*bInterval: Polling Interval (10 ms)*/
 /******************** Descriptor of Output endpoint ********************/
  /* 34 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

  HID_EPOUT_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  HID_EPOUT_SIZE, /*wMaxPacketSize: 16 Byte max */
  0x00,
  HID_FS_BINTERVAL,          /*bInterval: Polling Interval (10 ms)*/
  /* 41 */
} ;

/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_Desc[USB_HID_DESC_SIZ]  __ALIGN_END  =
{
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  LOBYTE(PID_FFBMK1_DESC_SIZE),/*wItemLength: Total length of Report descriptor*/
  HIBYTE(PID_FFBMK1_DESC_SIZE),
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC]  __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

__ALIGN_BEGIN static uint8_t HID_FFBStick_ReportDesc[PID_FFBMK1_DESC_SIZE]  __ALIGN_END =
{
0x05,0x01,                //Usage_Page(Generic_Desktop_ID),
0x15,0x00,                //Logical_Minimum(0),
0x09,0x04,                //Usage(Joystick_ID),
0xa1,0x01,                //Collection(Clc_Application),
0x85,0x01,                //    Report_ID(1), //FFBMK1 JoyStick Inputs interrupt in pipe->default 0
0x05,0x02,                //    Usage_Page(Simulation_ID),
0x09,0xbb,                //    Usage(Throttle_ID), //Throttle
0x15,0x81,                //    Logical_Minimum(-127),
0x25,0x7f,                //    Logical_Maximum(127),
0x75,0x08,                //    Report_Size(8),
0x95,0x01,                //    Report_Count(1),
0x81,0x02,                //    Input(IOF_Variable),
0x05,0x01,                //    Usage_Page(Generic_Desktop_ID),
0x09,0x01,                //    Usage(Pointer_ID),
0xa1,0x00,                //    Collection(Clc_Physical), //X&Y
0x09,0x30,                //        Usage(X_ID),
0x09,0x31,                //        Usage(Y_ID),
0x16,0x00,0xf8,           //        Logical_Minimum(-2048),
0x26,0xff,0x07,           //        Logical_Maximum(2047),
0x75,0x10,                //        Report_Size(16),
0x95,0x02,                //        Report_Count(2),
0x81,0x02,                //        Input(IOF_Variable),
0xc0,                     //    End_Collection(),
0x05,0x09,                //    Usage_Page(Button_ID), //Buttons
0x19,0x01,                //    Usage_Minimum(Button1_ID),
0x29,0x04,                //    Usage_Maximum(Button4_ID),
0x25,0x01,                //    Logical_Maximum(1),
0x15,0x00,                //    Logical_Minimum(0),
0x95,0x04,                //    Report_Count(4),
0x75,0x01,                //    Report_Size(1),
0x35,0x00,                //    Physical_Minimum(0),
0x81,0x02,                //    Input(IOF_Variable),
0x81,0x03,                //    Input(IOF_Constant), //4-bit pad
0x05,0x0f,                //    Usage_Page(Physical_Interface), //FFBMK1 JoyStick Outpus
0x09,0xab,                //    Usage(PID_Create_New_Effect_Report),
0xa1,0x02,                //    Collection(Clc_Logical),
0x09,0x25,                //        Usage(PID_Effect_Type),
0xa1,0x02,                //        Collection(Clc_Logical),
0x09,0x26,                //            Usage(PID_ET_Constant_Force),
0x09,0x31,                //            Usage(PID_ET_Sine),
0x09,0x32,                //            Usage(PID_ET_Triangle),
0x09,0x40,                //            Usage(PID_ET_Spring),
0x09,0x27,                //            Usage(PID_ET_Ramp),
0x25,0x05,                //            Logical_Maximum(5),
0x15,0x01,                //            Logical_Minimum(1),
0x75,0x08,                //            Report_Size(8),
0x95,0x01,                //            Report_Count(1),
0x91,0x01,                //            Output(IOF_Array),
0xc0,                     //        End_Collection(),
0x05,0x01,                //        Usage_Page(Generic_Desktop_ID),
0x09,0x3b,                //        Usage(Byte_Count),
0x15,0x00,                //        Logical_Minimum(0),
0x26,0xff,0x00,           //        Logical_Maximum(255),
0x91,0x02,                //        Output(IOF_Variable),
0xc0,                     //    End_Collection(),
0x05,0x0f,                //    Usage_Page(Physical_Interface),
0x09,0x89,                //    Usage(PID_PID_Block_Load_Report),
0xa1,0x02,                //    Collection(Clc_Logical),
0x85,0x02,                //        Report_ID(2), //input rpt id 1
0x09,0x22,                //        Usage(PID_Effect_Block_Index),
0x26,0xff,0x00,           //        Logical_Maximum(255),
0x75,0x08,                //        Report_Size(8),
0x95,0x01,                //        Report_Count(1),
0x81,0x02,                //        Input(IOF_Variable),
0x09,0x8b,                //        Usage(PID_Block_Load_Status),
0xa1,0x02,                //        Collection(Clc_Logical),
0x09,0x8c,                //            Usage(PID_Block_Load_Success),
0x09,0x8d,                //            Usage(PID_Block_Load_Full),
0x09,0x8e,                //            Usage(PID_Block_Load_Error),
0x25,0x03,                //            Logical_Maximum(3),
0x15,0x01,                //            Logical_Minimum(1),
0x81,0x01,                //            Input(IOF_Array),
0xc0,                     //        End_Collection(),
0x09,0xac,                //        Usage(PID_RAM_Pool_Available),
0x15,0x00,                //        Logical_Minimum(0),
0x28,0xff,0xff,0x00,0x00, //        Logical_Maximum(65535),
0x75,0x10,                //        Report_Size(16),
0x81,0x02,                //        Input(IOF_Variable),
0xc0,                     //    End_Collection(),
0x09,0x21,                //    Usage(PID_Set_Effect_Report), //Output
0xa1,0x02,                //    Collection(Clc_Logical),
0x85,0x03,                //        Report_ID(3),
0x09,0x22,                //        Usage(PID_Effect_Block_Index),
0x26,0xff,0x00,           //        Logical_Maximum(255),
0x15,0x00,                //        Logical_Minimum(0),
0x75,0x08,                //        Report_Size(8),
0x95,0x01,                //        Report_Count(1),
0x91,0x02,                //        Output(IOF_Variable),
0x09,0x25,                //        Usage(PID_Effect_Type),
0xa1,0x02,                //        Collection(Clc_Logical),
0x09,0x26,                //            Usage(PID_ET_Constant_Force),
0x09,0x27,                //            Usage(PID_ET_Ramp),
0x09,0x32,                //            Usage(PID_ET_Triangle),
0x09,0x40,                //            Usage(PID_ET_Spring),
0x25,0x04,                //            Logical_Maximum(4),
0x15,0x01,                //            Logical_Minimum(1),
0x91,0x01,                //            Output(IOF_Array),
0xc0,                     //        End_Collection(),
0x09,0x50,                //        Usage(PID_Duration),
0x09,0x54,                //        Usage(PID_Trigger_Repeat_Interval),
0x15,0x00,                //        Logical_Minimum(0),
0x26,0x10,0x27,           //        Logical_Maximum(10000),
0x46,0x10,0x27,           //        Physical_Maximum(10000),
0x75,0x10,                //        Report_Size(16),
0x95,0x02,                //        Report_Count(2),
0x66,0x03,0x10,           //        Unit(Eng_Lin_Time),
0x55,0x0d,                //        Unit_Exponent(-3),
0x91,0x02,                //        Output(IOF_Variable),
0x55,0x0a,                //        Unit_Exponent(-6),
0x09,0x51,                //        Usage(PID_Sample_Period),
0x95,0x01,                //        Report_Count(1),
0x91,0x02,                //        Output(IOF_Variable),
0x45,0x00,                //        Physical_Maximum(0),
0x55,0x00,                //        Unit_Exponent(0),
0x65,0x00,                //        Unit(Unit_None),
0x09,0x52,                //        Usage(PID_Gain),
0x09,0x53,                //        Usage(PID_Trigger_Button),
0x25,0x7f,                //        Logical_Maximum(127),
0x75,0x08,                //        Report_Size(8),
0x95,0x02,                //        Report_Count(2),
0x91,0x02,                //        Output(IOF_Variable),
0x09,0x55,                //        Usage(PID_Axes_Enable),
0xa1,0x02,                //        Collection(Clc_Logical),
0x05,0x01,                //            Usage_Page(Generic_Desktop_ID),
0x09,0x01,                //            Usage(Pointer_ID),
0xa1,0x00,                //            Collection(Clc_Physical),
0x09,0x30,                //                Usage(X_ID),
0x09,0x31,                //                Usage(Y_ID),
0x25,0x01,                //                Logical_Maximum(1),
0x75,0x04,                //                Report_Size(4),
0x91,0x02,                //                Output(IOF_Variable),
0xc0,                     //            End_Collection(),
0xc0,                     //        End_Collection(),
0x05,0x0f,                //        Usage_Page(Physical_Interface),
0x09,0x56,                //        Usage(PID_Direction_Enable),
0x25,0x01,                //        Logical_Maximum(1),
0x75,0x08,                //        Report_Size(8),
0x95,0x01,                //        Report_Count(1),
0x91,0x02,                //        Output(IOF_Variable),
0x09,0x57,                //        Usage(PID_Direction),
0xa1,0x02,                //        Collection(Clc_Logical),
0x05,0x01,                //            Usage_Page(Generic_Desktop_ID),
0x09,0x01,                //            Usage(Pointer_ID),
0xa1,0x00,                //            Collection(Clc_Physical),
0x09,0x30,                //                Usage(X_ID),
0x09,0x31,                //                Usage(Y_ID),
0x15,0x00,                //                Logical_Minimum(0),
0x26,0xff,0x00,           //                Logical_Maximum(255),
0x46,0x68,0x01,           //                Physical_Maximum(360),
0x65,0x14,                //                Unit(Eng_Rot_Angular_Pos),
0x95,0x02,                //                Report_Count(2),
0x91,0x02,                //                Output(IOF_Variable),
0x65,0x00,                //                Unit(Unit_None),
0x45,0x00,                //                Physical_Maximum(0),
0xc0,                     //            End_Collection(),
0xc0,                     //        End_Collection(),
0xc0,                     //    End_Collection(),
0x05,0x0f,                //    Usage_Page(Physical_Interface),
0x09,0x5a,                //    Usage(PID_Set_Envelope_Report), //envelope
0xa1,0x02,                //    Collection(Clc_Logical),
0x85,0x04,                //        Report_ID(4),
0x09,0x22,                //        Usage(PID_Effect_Block_Index),
0x26,0xff,0x00,           //        Logical_Maximum(255),
0x15,0x00,                //        Logical_Minimum(0),
0x75,0x08,                //        Report_Size(8),
0x95,0x01,                //        Report_Count(1),
0x91,0x02,                //        Output(IOF_Variable),
0x09,0x23,                //        Usage(PID_Parameter_Block_Offset),
0x25,0x01,                //        Logical_Maximum(1),
0x91,0x02,                //        Output(IOF_Variable),
0x09,0x5b,                //        Usage(PID_Attack_Level),
0x09,0x5d,                //        Usage(PID_Fade_Level),
0x26,0x10,0x27,           //        Logical_Maximum(10000),
0x95,0x02,                //        Report_Count(2),
0x75,0x10,                //        Report_Size(16),
0x91,0x02,                //        Output(IOF_Variable),
0x09,0x5c,                //        Usage(PID_Attack_Time),
0x09,0x5e,                //        Usage(PID_Fade_Time),
0x46,0x10,0x27,           //        Physical_Maximum(10000),
0x66,0x03,0x10,           //        Unit(Eng_Lin_Time),
0x55,0x0d,                //        Unit_Exponent(-3), //check here whether mytool works well
0x75,0x10,                //        Report_Size(16),
0x91,0x02,                //        Output(IOF_Variable),
0x45,0x00,                //        Physical_Maximum(0),
0x65,0x00,                //        Unit(Unit_None),
0x55,0x00,                //        Unit_Exponent(0),
0xc0,                     //    End_Collection(),
0x09,0x5f,                //    Usage(PID_Set_Condition_Report), //condition
0xa1,0x02,                //    Collection(Clc_Logical)
0x85,0x05,                //        Report_ID(5),
0x09,0x22,                //        Usage(PID_Effect_Block_Index),
0x26,0xff,0x00,           //        Logical_Maximum(255),
0x15,0x00,                //        Logical_Minimum(0),
0x75,0x08,                //        Report_Size(8),
0x95,0x01,                //        Report_Count(1),
0x91,0x02,                //        Output(IOF_Variable),
0x09,0x23,                //        Usage(PID_Parameter_Block_Offset),
0x25,0x01,                //        Logical_Maximum(1),
0x91,0x02,                //        Output(IOF_Variable),
0x75,0x10,                //        Report_Size(16),
0x26,0x10,0x27,           //        Logical_Maximum(10000),
0x16,0xf0,0xd8,           //        Logical_Minimum(-10000),
0x09,0x60,                //        Usage(PID_CP_Offset),
0x09,0x61,                //        Usage(PID_Positive_Coefficient),
0x09,0x62,                //        Usage(PID_Negative_Coefficient),
0x09,0x63,                //        Usage(PID_Positive_Saturation),
0x09,0x64,                //        Usage(PID_Negative_Saturation),
0x09,0x65,                //        Usage(PID_Dead_Band),
0x95,0x06,                //        Report_Count(6),
0x91,0x02,                //        Output(IOF_Variable),
0x15,0x00,                //        Logical_Minimum(0),
0xc0,                     //    End_Collection(),
0x09,0x6e,                //    Usage(PID_Set_Periodic_Report), //periodic
0xa1,0x02,                //    Collection(Clc_Logical),
0x85,0x06,                //        Report_ID(6),
0x09,0x22,                //        Usage(PID_Effect_Block_Index),
0x26,0xff,0x00,           //        Logical_Maximum(255),
0x15,0x00,                //        Logical_Minimum(0),
0x75,0x08,                //        Report_Size(8),
0x95,0x01,                //        Report_Count(1),
0x91,0x02,                //        Output(IOF_Variable),
0x09,0x23,                //        Usage(PID_Parameter_Block_Offset),
0x25,0x01,                //        Logical_Maximum(1),
0x91,0x02,                //        Output(IOF_Variable),
0x75,0x10,                //        Report_Size(16),
0x09,0x70,                //        Usage(PID_Magnitude),
0x09,0x6f,                //        Usage(PID_Offset),
0x09,0x71,                //        Usage(PID_Phase),
0x26,0x10,0x27,           //        Logical_Maximum(10000),
0x16,0xf0,0xd8,           //        Logical_Minimum(-10000),
0x95,0x03,                //        Report_Count(3),
0x91,0x02,                //        Output(IOF_Variable),
0x09,0x72,                //        Usage(PID_Period),
0x46,0x10,0x27,           //        Physical_Maximum(10000),
0x15,0x00,                //        Logical_Minimum(0),
0x66,0x03,0x10,           //        Unit(Eng_Lin_Time),
0x55,0x0d,                //        Unit_Exponent(-3),
0x95,0x01,                //        Report_Count(1),
0x91,0x02,                //        Output(IOF_Variable),
0x45,0x00,                //        Physical_Maximum(0),
0x65,0x00,                //        Unit(Unit_None),
0x55,0x00,                //        Unit_Exponent(0),
0xc0,                     //    End_Collection(),
0x09,0x73,                //    Usage(PID_Set_Constant_Force_Report),
0xa1,0x02,                //    Collection(Clc_Logical),
0x85,0x07,                //        Report_ID(7),
0x09,0x22,                //        Usage(PID_Effect_Block_Index),
0x26,0xff,0x00,           //        Logical_Maximum(255),
0x15,0x00,                //        Logical_Minimum(0),
0x75,0x08,                //        Report_Size(8),
0x95,0x01,                //        Report_Count(1),
0x91,0x02,                //        Output(IOF_Variable),
0x09,0x23,                //        Usage(PID_Parameter_Block_Offset),
0x25,0x01,                //        Logical_Maximum(1),
0x91,0x02,                //        Output(IOF_Variable),
0x09,0x70,                //        Usage(PID_Magnitude),
0x75,0x10,                //        Report_Size(16),
0x26,0x10,0x27,           //        Logical_Maximum(10000),
0x91,0x02,                //        Output(IOF_Variable),
0xc0,                     //    End_Collection(),
0x09,0x74,                //    Usage(PID_Set_Ramp_Force_Report),
0xa1,0x02,                //    Collection(Clc_Logical),
0x85,0x08,                //        Report_ID(8),
0x09,0x22,                //        Usage(PID_Effect_Block_Index),
0x26,0xff,0x00,           //        Logical_Maximum(255),
0x15,0x00,                //        Logical_Minimum(0),
0x75,0x08,                //        Report_Size(8),
0x95,0x01,                //        Report_Count(1),
0x91,0x02,                //        Output(IOF_Variable),
0x09,0x23,                //        Usage(PID_Parameter_Block_Offset),
0x25,0x01,                //        Logical_Maximum(1),
0x91,0x02,                //        Output(IOF_Variable),
0x09,0x75,                //        Usage(PID_Ramp_Start),
0x09,0x76,                //        Usage(PID_Ramp_End),
0x26,0x10,0x27,           //        Logical_Maximum(10000),
0x16,0xf0,0xd8,           //        Logical_Minimum(-10000),
0x95,0x02,                //        Report_Count(2),
0x75,0x10,                //        Report_Size(16),
0x91,0x02,                //        Output(IOF_Variable),
0x15,0x00,                //        Logical_Minimum(0),
0xc0,                     //    End_Collection(),
0x09,0x77,                //    Usage(PID_Effect_Operation_Report),
0xa1,0x02,                //    Collection(Clc_Logical),
0x85,0x09,                //        Report_ID(9),
0x09,0x22,                //        Usage(PID_Effect_Block_Index),
0x26,0xff,0x00,           //        Logical_Maximum(255),
0x75,0x08,                //        Report_Size(8),
0x95,0x01,                //        Report_Count(1),
0x91,0x02,                //        Output(IOF_Variable),
0x09,0x78,                //        Usage(PID_Effect_Operation),
0xa1,0x02,                //        Collection(Clc_Logical),
0x09,0x79,                //            Usage(PID_Op_Effect_Start),
0x09,0x7a,                //            Usage(PID_Op_Effect_Start_Solo),
0x09,0x7b,                //            Usage(PID_Op_Effect_Stop),
0x15,0x01,                //            Logical_Minimum(1),
0x25,0x03,                //            Logical_Maximum(3),
0x91,0x01,                //            Output(IOF_Array),
0xc0,                     //        End_Collection(),
0x09,0x7c,                //        Usage(PID_Loop_Count),
0x15,0x00,                //        Logical_Minimum(0),
0x26,0xff,0x00,           //        Logical_Maximum(255),
0x91,0x02,                //        Output(IOF_Variable),
0xc0,                     //    End_Collection(),
0x09,0x92,                //    Usage(PID_PID_State_Report), //Input rpt 2
0xa1,0x02,                //    Collection(Clc_Logical),
0x85,0x0a,                //        Report_ID(10),
0x09,0x22,                //        Usage(PID_Effect_Block_Index),
0x81,0x02,                //        Input(IOF_Variable),
0x15,0x01,                //        Logical_Minimum(1),
0x25,0x04,                //        Logical_Maximum(4),
0x09,0x94,                //        Usage(PID_Effect_Playing),
0x09,0xa0,                //        Usage(PID_Actuators_Enabled),
0x09,0xa4,                //        Usage(PID_Safety_Switch),
0x09,0xa6,                //        Usage(PID_Actuator_Power),
0x75,0x01,                //        Report_Size(1),
0x95,0x04,                //        Report_Count(4),
0x81,0x02,                //        Input(IOF_Variable),
0x81,0x03,                //        Input(IOF_Constant), //4-bit pad
0xc0,                     //    End_Collection(),
0x09,0x95,                //    Usage(PID_PID_Device_Control_Report),
0xa1,0x02,                //    Collection(Clc_Logical),
0x85,0x0b,                //        Report_ID(11),
0x09,0x96,                //        Usage(PID_PID_Device_Control),
0xa1,0x02,                //        Collection(Clc_Logical),
0x09,0x97,                //            Usage(PID_DC_Enable_Actuators),
0x09,0x98,                //            Usage(PID_DC_Disable_Actuators),
0x09,0x99,                //            Usage(PID_DC_Stop_All_Effects),
0x09,0x9a,                //            Usage(PID_DC_Device_Reset),
0x09,0x9b,                //            Usage(PID_DC_Device_Pause),
0x09,0x9c,                //            Usage(PID_DC_Device_Continue),
0x15,0x01,                //            Logical_Minimum(1),
0x25,0x06,                //            Logical_Maximum(6),
0x75,0x08,                //            Report_Size(8),
0x95,0x01,                //            Report_Count(1),
0x91,0x01,                //            Output(IOF_Array),
0xc0,                     //        End_Collection(),
0xc0,                     //    End_Collection(),
0x09,0x7d,                //    Usage(PID_Device_Gain_Report),
0xa1,0x02,                //    Collection(Clc_Logical),
0x85,0x0c,                //        Report_ID(12),
0x09,0x7e,                //        Usage(PID_Device_Gain),
0x15,0x00,                //        Logical_Minimum(0),
0x26,0xff,0x00,           //        Logical_Maximum(255),
0xb1,0x02,                //        Feature(IOF_Variable),
0xc0,                     //    End_Collection(),
0x09,0x90,                //    Usage(PID_PID_Block_Free_Report),
0xa1,0x02,                //    Collection(Clc_Logical),
0x85,0x0d,                //        Report_ID(13),
0x09,0x22,                //        Usage(PID_Effect_Block_Index),
0x91,0x02,                //        Output(IOF_Variable),
0xc0,                     //    End_Collection(),
0xc0,                     //End_Collection(),
//Total:713 Bytes ?
};

/**
  * @}
  */

/** @defgroup USBD_HID_Private_Functions
  * @{
  */

/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_Init (USBD_HandleTypeDef *pdev,
                               uint8_t cfgidx)
{
  uint8_t ret = 0;

  /* Open EP IN */
  USBD_LL_OpenEP(pdev,
                 HID_EPIN_ADDR,
                 USBD_EP_TYPE_INTR,
                 HID_EPIN_SIZE);
	USBD_LL_OpenEP(pdev,
								 HID_EPOUT_ADDR,
								 USBD_EP_TYPE_INTR,
								 HID_EPOUT_SIZE);

  pdev->pClassData = USBD_malloc(sizeof (USBD_HID_HandleTypeDef));

  if(pdev->pClassData == NULL)
  {
    ret = 1;
  }
  else
  {
    ((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = HID_IDLE;
  }
	USBD_PID_Init(pdev);
  return ret;
}

/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_DeInit (USBD_HandleTypeDef *pdev,
                                 uint8_t cfgidx)
{
  /* Close HID EPs */
  USBD_LL_CloseEP(pdev,
                  HID_EPIN_ADDR);
	USBD_LL_CloseEP(pdev,
									HID_EPOUT_ADDR);

  /* FRee allocated memory */
  if(pdev->pClassData != NULL)
  {
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }

  return USBD_OK;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_HID_Setup (USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req)
{
  uint16_t len = 0;
  uint8_t  *pbuf = NULL;
  USBD_HID_HandleTypeDef     *hhid = (USBD_HID_HandleTypeDef*) pdev->pClassData;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :
    switch (req->bRequest)
    {


    case HID_REQ_SET_PROTOCOL:
      hhid->Protocol = (uint8_t)(req->wValue);
      break;

    case HID_REQ_GET_PROTOCOL:
      USBD_CtlSendData (pdev,
                        (uint8_t *)&hhid->Protocol,
                        1);
      break;

    case HID_REQ_SET_IDLE:
      hhid->IdleState = (uint8_t)(req->wValue >> 8);
      break;

    case HID_REQ_GET_IDLE:
      USBD_CtlSendData (pdev,
                        (uint8_t *)&hhid->IdleState,
                        1);
      break;

    case HID_REQ_SET_REPORT: //beantowel
		case HID_REQ_GET_REPORT:
			USBD_PID_ItfReq(pdev, req);
			break;
    }
    break;

  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR:
      if( req->wValue >> 8 == HID_REPORT_DESC)
      {
        len = MIN(PID_FFBMK1_DESC_SIZE , req->wLength);
        pbuf = HID_FFBStick_ReportDesc;
      }
      else if( req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
      {
        pbuf = USBD_HID_Desc;
        len = MIN(USB_HID_DESC_SIZ , req->wLength);
      }

      USBD_CtlSendData (pdev,
                        pbuf,
                        len);

      break;

    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&hhid->AltSetting,
                        1);
      break;

    case USB_REQ_SET_INTERFACE :
      hhid->AltSetting = (uint8_t)(req->wValue);
      break;
    }
  }
  return USBD_OK;
}

/**
  * @brief  USBD_HID_SendReport
  *         Send HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t USBD_HID_SendReport     (USBD_HandleTypeDef  *pdev,
                                 uint8_t *report,
                                 uint16_t len)
{
  USBD_HID_HandleTypeDef     *hhid = (USBD_HID_HandleTypeDef*)pdev->pClassData;

  if (pdev->dev_state == USBD_STATE_CONFIGURED )
  {
    if(hhid->state == HID_IDLE)
    {
      hhid->state = HID_BUSY;
      USBD_LL_Transmit (pdev,
                        HID_EPIN_ADDR,
                        report,
                        len);
    }
  }
  return USBD_OK;
}

/**
  * @brief  USBD_HID_GetPollingInterval
  *         return polling interval from endpoint descriptor
  * @param  pdev: device instance
  * @retval polling interval
  */
uint32_t USBD_HID_GetPollingInterval (USBD_HandleTypeDef *pdev)
{
  uint32_t polling_interval = 0;

  /* HIGH-speed endpoints */
  if(pdev->dev_speed == USBD_SPEED_HIGH)
  {
   /* Sets the data transfer polling interval for high speed transfers.
    Values between 1..16 are allowed. Values correspond to interval
    of 2 ^ (bInterval-1). This option (8 ms, corresponds to HID_HS_BINTERVAL */
    polling_interval = (((1 <<(HID_HS_BINTERVAL - 1)))/8);
  }
  else   /* LOW and FULL-speed endpoints */
  {
    /* Sets the data transfer polling interval for low and full
    speed transfers */
    polling_interval =  HID_FS_BINTERVAL;
  }

  return ((uint32_t)(polling_interval));
}

/**
  * @brief  USBD_HID_GetCfgDesc
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_HID_GetCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_HID_CfgDesc);
  return USBD_HID_CfgDesc;
}


/**
  * @brief  USBD_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_HID_DataIn (USBD_HandleTypeDef *pdev,
                              uint8_t epnum)
{

  /* Ensure that the FIFO is empty before a new transfer, this condition could
  be caused by  a new transfer before the end of the previous transfer */
  ((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = HID_IDLE;
  return USBD_OK;
}



/**
  * @brief  USBD_HID_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_HID_DataOut (USBD_HandleTypeDef *pdev,
                              uint8_t epnum)
{
//  /* Ensure that the FIFO is empty before a new transfer, this condition could
//  be caused by  a new transfer before the end of the previous transfer */
	USBD_PID_DataOut(pdev, epnum);
	((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = HID_IDLE;
  return USBD_OK;
}

static uint8_t USBD_HID_EP0_RxReady (USBD_HandleTypeDef *pdev)
{
	USBD_PID_EP0_RxReady(pdev);
	return USBD_OK;
}

/**
* @brief  DeviceQualifierDescriptor
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t  *USBD_HID_GetDeviceQualifierDesc (uint16_t *length)
{
  *length = sizeof (USBD_HID_DeviceQualifierDesc);
  return USBD_HID_DeviceQualifierDesc;
}

/**
  * @}
  */


/**
  * @}
  */


/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
