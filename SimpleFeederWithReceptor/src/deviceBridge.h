#pragma once
#include <tchar.h>
#include <wtypes.h>
#include <stdint.h>

typedef struct _InputReport {
	BYTE throttle;
	int16_t x;
	int16_t y;
	BYTE button;
} InputReport, *PInputReport;

int Bridge_Main(void);
void DeviceWrite(BYTE *data, DWORD len);
void FormOutReport(BYTE *data,int32_t Tx,int32_t Ty);
PInputReport DeviceGetPos();
