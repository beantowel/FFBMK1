/*
  specific functions for pid device,
  to replenish MXCube customed usbd_hid
*/
#include "usbd_pid.h"
#include "usbd_hid.h"
#include "ffb_manager.h"

extern USBD_HandleTypeDef hUsbDeviceFS;
const uint8_t HID_Out_Report_Maxlen = 0x20;
uint8_t HID_Out_Report[HID_Out_Report_Maxlen];


uint8_t USBD_PID_Init (USBD_HandleTypeDef *pdev)
{
	/*add FFBmngrInit to HID_Init*/
	FFBMngrInit();
	return USBD_OK;
}
uint8_t USBD_PID_DataOut (USBD_HandleTypeDef *pdev,uint8_t epnum)
{
	/*data out Stage hook*/
	if(epnum == HID_EPOUT_ADDR){
		uint16_t len = USBD_GetRxCount(&hUsbDeviceFS, epnum);
		FFBMngrDataOutServ(HID_Out_Report, len);
	}
	else{
		USBD_ErrLog("Out on unexpected Endpoint");
	}
	return USBD_OK;
}
uint8_t USBD_PID_Send (uint8_t *data, uint16_t len)
{
	/*Interface for ffb_manager*/
	USBD_HID_SendReport(&hUsbDeviceFS, data, len);
	return USBD_OK;
}
uint8_t USBD_PID_EP0_RxReady (USBD_HandleTypeDef *pdev)
{
	/*control pipe receive ready*/
	((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = HID_IDLE;
	return USBD_OK;
}
uint8_t USBD_PID_ItfReq(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	/*Class Request Handler:Get_Report && Set_Report*/
	switch(req->bRequest)
	{
		case HID_REQ_GET_REPORT:
			FFBMngrDataInServ(HIBYTE(req->wValue));
			break;
		case HID_REQ_SET_REPORT:
			USBD_LL_PrepareReceive(pdev, HID_EPOUT_ADDR, HID_Out_Report, req->wLength);
			break;
	}
	((USBD_HID_HandleTypeDef *)pdev->pClassData)->state = HID_IDLE;
  return USBD_OK;
}






















