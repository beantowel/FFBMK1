#include "deviceBridge.h"
#include <stdio.h>
#include <tchar.h>
#include <wtypes.h>
#include <Windows.h>
#include <iostream>
extern "C" {
#include <hidsdi.h>
#include <setupapi.h>
}
using namespace std;
#define     USB_VID			0x0483
#define     USB_PID			0x572B
#define		REPORT_COUNT	64
#define tprintf //printf
#define t1printf printf

HANDLE       hDev;
InputReport  inputReport;

int Bridge_Main(void);
DWORD WINAPI DeviceBridgeStart(LPVOID args);
HANDLE OpenMyHIDDevice(int overlapped);
void DeviceRead();
void DeviceWrite(BYTE *data, DWORD len);
PInputReport DeviceGetPos();

int Bridge_Main() {
	cout << "Device Bridge handle Start" << endl;
	HANDLE hThread = CreateThread(NULL, 0, DeviceBridgeStart, NULL, 0, NULL); //start bridge thread
	return 0;
}
void DeviceRead() {
	BYTE         recvDataBuf[1024];
	DWORD        bytesCnt;
	recvDataBuf[0] = 1;

	if (!ReadFile(hDev, recvDataBuf, 0x40 + 1, &bytesCnt, NULL)) { // read inputs
		tprintf("read data error! %d\n", GetLastError());
		return;
	}

	inputReport.throttle = recvDataBuf[1];
	inputReport.x = recvDataBuf[2] | (recvDataBuf[3] << 8);
	inputReport.y = recvDataBuf[4] | (recvDataBuf[5] << 8);
	inputReport.button = recvDataBuf[5];
	tprintf("read data:\n throttle=%d, x=%d, y=%d\n",inputReport.throttle, inputReport.x, inputReport.y);
}
void DeviceWrite(BYTE *data, DWORD len) {
	DWORD        bytesCnt = len;
	t1printf("writing data to device... cnt=%d\n", len);
	//write output data
	if (!WriteFile(hDev, data, len, &bytesCnt, NULL)) {
		t1printf("write data error! %d\n", GetLastError());
	    return;
	} else {
		t1printf("done writing data, cnt:%d\n", bytesCnt);
	}
}
DWORD WINAPI DeviceBridgeStart(LPVOID args) {
	hDev = OpenMyHIDDevice(0); //open device overlap == 0
	if (hDev == INVALID_HANDLE_VALUE) {
		tprintf("INVALID_HANDLE_VALUE\n");
		return 0;
	}
	cout << "DeviceBridge Start Succeed!" << endl;
	while(TRUE){
		DeviceRead();
		Sleep(5); //FS Interval == 10 > 5 ms
	}
	return 0;
}
HANDLE OpenMyHIDDevice(int overlapped) {
	HANDLE hidHandle = NULL;
	GUID hidGuid;
	HidD_GetHidGuid(&hidGuid);
	HDEVINFO hDevInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));
	if (hDevInfo == INVALID_HANDLE_VALUE) {
		return INVALID_HANDLE_VALUE;
	}
	SP_DEVICE_INTERFACE_DATA devInfoData;
	devInfoData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	int deviceNo = 0; //start enumeration
	SetLastError(NO_ERROR);
	while (GetLastError() != ERROR_NO_MORE_ITEMS) {
		if (!SetupDiEnumInterfaceDevice(hDevInfo, 0, &hidGuid, deviceNo, &devInfoData))
			continue; //enumeration
		ULONG  requiredLength = 0;
		SetupDiGetInterfaceDeviceDetail(hDevInfo,
			&devInfoData,
			NULL,
			0,
			&requiredLength,
			NULL); //get requiredLength first
		PSP_INTERFACE_DEVICE_DETAIL_DATA pdevDetail = \
			(SP_INTERFACE_DEVICE_DETAIL_DATA *)malloc(requiredLength);
		pdevDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
		if (!SetupDiGetInterfaceDeviceDetail(hDevInfo,
			&devInfoData,
			pdevDetail,
			requiredLength,
			NULL,
			NULL)) { //get interface device
			free(pdevDetail);
			SetupDiDestroyDeviceInfoList(hDevInfo);
			return INVALID_HANDLE_VALUE;
		}
		if (overlapped) { //create file
			hidHandle = CreateFile(pdevDetail->DevicePath,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_OVERLAPPED,
				NULL);
		}
		else {
			hidHandle = CreateFile(pdevDetail->DevicePath,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);
		}
		free(pdevDetail);
		if (hidHandle == INVALID_HANDLE_VALUE) { //failed to create file
			SetupDiDestroyDeviceInfoList(hDevInfo);
			return INVALID_HANDLE_VALUE;
		}
		_HIDD_ATTRIBUTES hidAttributes; //get attributes
		if (!HidD_GetAttributes(hidHandle, &hidAttributes)) {
			CloseHandle(hidHandle);
			SetupDiDestroyDeviceInfoList(hDevInfo);
			return INVALID_HANDLE_VALUE;
		}
		if (USB_VID == hidAttributes.VendorID
			&& USB_PID == hidAttributes.ProductID) {
			tprintf("target device found:vid=%d,pid=%d\n", USB_VID, USB_PID);
			break;
		}
		else {
			CloseHandle(hidHandle);
			++deviceNo; //continue enumeration
		}
	}
	SetupDiDestroyDeviceInfoList(hDevInfo);
	return hidHandle;
}
PInputReport DeviceGetPos() {
	return &inputReport;
}
void FormOutReport(BYTE *data, int32_t Tx, int32_t Ty) {
	data[0] = 8; //rptID
	data[1] = 1; //nonsense
	data[2] = LOBYTE(Tx);
	data[3] = HIBYTE(Tx);
	data[4] = LOBYTE(Ty);
	data[5] = HIBYTE(Ty);
}
