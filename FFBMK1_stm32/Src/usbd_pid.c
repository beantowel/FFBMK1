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
const int16_t HID_In_Report_len = 7;
uint8_t HID_In_Report[HID_In_Report_len];
uint8_t reqInvalid = 0;

uint8_t USBD_PID_Init (USBD_HandleTypeDef *pdev)
{
	/*add FFBmngrInit to HID_Init*/
	FFBMngrInit();
	return USBD_OK;
}
uint8_t USBD_PID_DataOut (USBD_HandleTypeDef *pdev,uint8_t epnum)
{
	/*EP1 out pipe data out Stage hook*/
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_11); //toggle PC11
	if(reqInvalid) return USBD_OK;
	if(epnum == HID_EPOUT_ADDR){
		uint16_t len = USBD_GetRxCount(&hUsbDeviceFS, epnum);
		FFBMngrDataOutServ(HID_Out_Report, len);
	}
	else{
		USBD_ErrLog("Out on unexpected Endpoint");
	}
	return USBD_OK;
}
//uint8_t USBD_PID_DataIn (USBD_HandleTypeDef *pdev,uint8_t epnum){
//	if(epnum == HID_EPIN_ADDR){
//		USBD_PID_Send(HID_In_Report, HID_In_Report_len); //Send Report
//	}
//	return USBD_OK;
//}
uint8_t USBD_PID_EP0_RxReady (USBD_HandleTypeDef *pdev)
{
	/*control pipe receive ready*/
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_11); //toggle PC11
	if(reqInvalid) return USBD_OK;
	FFBMngrFeatureServ(HID_Out_Report[0], 0, HID_Out_Report); //host to device
	return USBD_OK;
}
uint8_t USBD_PID_ItfReq(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	/*Class Request Handler:Get_Report && Set_Report*/
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_10);
	uint8_t ReportType = HIBYTE(req->wValue);
	uint8_t ReportID = LOBYTE(req->wValue);
	//uint8_t dir = req->bmRequest & 0x80;
	reqInvalid = 0;
	switch(req->bRequest )
	{
		case HID_REQ_GET_REPORT:
			switch(ReportType){
				case 0x01: //Input
					FFBMngrDataInServ(ReportID);
					break;
				case 0x03: //Feature
					FFBMngrFeatureServ(ReportID, 1, NULL); //device to host
				default:
					break;
			}
			break;
		case HID_REQ_SET_REPORT:
			if(req->wLength == 0) break; //don not receive 0-length out
			switch(ReportType){
				case 0x02: //Output EP1 Out
					USBD_LL_PrepareReceive(pdev, 0x01, HID_Out_Report, req->wLength);
					break;
				case 0x03: //Feature EP0 Out
					USBD_CtlPrepareRx(&hUsbDeviceFS, HID_Out_Report, req->wLength);
					break;
			}
			break;
		default:
			reqInvalid = 1;
			if(req->wLength > 0){ //in case of invalid reports
				switch(ReportType){
					case 0x02: //Output EP1 Out
						USBD_LL_PrepareReceive(pdev, 0x01, HID_Out_Report, req->wLength);
						break;
					case 0x03: //Feature EP0 Out
						USBD_CtlPrepareRx(&hUsbDeviceFS, HID_Out_Report, req->wLength);
						break;
					default:
						break;
				}
			}
			break;
	}
  return USBD_OK;
}
uint8_t USBD_PID_Send (uint8_t *data, uint16_t len)
{
	/*Interface for ffb_manager*/
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_4);
	USBD_HID_SendReport(&hUsbDeviceFS, data, len);
	return USBD_OK;
}
uint8_t USBD_PID_Send_EP0 (uint8_t *data, uint16_t len)
{
  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_4);
	USBD_CtlSendData (&hUsbDeviceFS, data, len);
  return USBD_OK;
}






















