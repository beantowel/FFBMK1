/**
  ******************************************************************************
  * @file    usbd_pid.c
  * @author  beantowel
  * @version V1.0
  * @date    114-November-2016
  * @brief   This file provides the PID(Physical Interface Device) core functions.
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_pid.h"
#include "usbd_hid.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "stm32f103xe.h"

uint8_t Receive_Buffer[128];
uint8_t USBD_PID_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);


/**
  * @brief  USBD_PID_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
uint8_t  USBD_PID_DataOut (USBD_HandleTypeDef *pdev, 
                              uint8_t epnum)
{  
  /* Ensure that the FIFO is empty before a new transfer, this condition could 
  be caused by  a new transfer before the end of the previous transfer */
	//USBD_HID_HandleTypeDef     *hhid = (USBD_HID_HandleTypeDef*)pdev->pClassData;
//	uint16_t len;
//	len=USBD_GetRxCount(pdev,HID_EPOUT_ADDR);
//	
//  ((USBD_HID_HandleTypeDef *)pdev->pClassData)->pidstate = PID_IDLE;
//	 USBD_LL_PrepareReceive(pdev, HID_EPOUT_ADDR , Receive_Buffer,len);
//	
//	USBD_HID_SendReport(pdev,Receive_Buffer,len);
//	HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_2);
//	HAL_Delay(100);
  return USBD_OK;
}













