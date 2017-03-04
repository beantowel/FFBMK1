#include "stdint.h"
//little-endian sotred data
//data structure from <Device Class Definition for PID Devices> Figure[3:]
// struct _PID_Data_Input {
// 	uint8_t ReportID;
// 	uint8_t Throttle;
// 	uint16_t Xposition;
// 	uint16_t Yposition;
// 	uint8_t ButtonState; // 4-bit pad + button 4,3,2,1 state
// };
