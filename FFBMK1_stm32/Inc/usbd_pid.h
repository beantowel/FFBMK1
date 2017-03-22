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

#ifdef __cplusplus
}
#endif

#endif  /* __USB_PID_H */
