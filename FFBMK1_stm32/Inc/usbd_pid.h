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
#include "usbd_ioreq.h"
/** @defgroup USBD_PID_Exported_Defines
  * @{
  */
#define PID_EPIN_ADDR                 0x81
#define PID_EPIN_SIZE                 0x40
#define PID_EPOUT_ADDR                0x01
#define PID_EPOUT_SIZE                0x40
//FS-Device max size 0x40!!!2016.9.29

#define USB_PID_CONFIG_DESC_SIZ       41
#define USB_PID_DESC_SIZ              9
#define PID_FFBMK1_DESC_SIZE          68
/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
uint8_t USBD_PID_Send(uint8_t *data, uint16_t len);
uint8_t USBD_PID_ItfReq(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
uint8_t USBD_PID_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
uint8_t USBD_PID_Init(USBD_HandleTypeDef *pdev);
uint8_t USBD_PID_EP0_RxReady (USBD_HandleTypeDef *pdev);
/**
  * @}
  */
/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_PID_CfgDesc[USB_PID_CONFIG_DESC_SIZ]  __ALIGN_END =
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_PID_CONFIG_DESC_SIZ,
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
  HID_DESCRIPTOR_TYPE, /*bDescriptorType: PID*/
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

  PID_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  PID_EPIN_SIZE, /*wMaxPacketSize: 4 Byte max */
  0x00,
  HID_FS_BINTERVAL,          /*bInterval: Polling Interval (10 ms)*/
 /******************** Descriptor of Output endpoint ********************/
  /* 34 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

  PID_EPOUT_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  PID_EPOUT_SIZE, /*wMaxPacketSize: 16 Byte max */
  0x00,
  HID_FS_BINTERVAL,          /*bInterval: Polling Interval (10 ms)*/
  /* 41 */
} ;

/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_PID_Desc[USB_PID_DESC_SIZ]  __ALIGN_END  =
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
__ALIGN_BEGIN static uint8_t USBD_PID_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC]  __ALIGN_END =
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

__ALIGN_BEGIN static uint8_t PID_FFBStick_ReportDesc[PID_FFBMK1_DESC_SIZE]  __ALIGN_END =
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
0xc0,                     //End_Collection(),
//Total:713 Bytes
};

#ifdef __cplusplus
}
#endif

#endif  /* __USB_PID_H */
