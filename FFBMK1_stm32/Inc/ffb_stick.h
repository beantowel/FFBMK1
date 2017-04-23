#ifndef __FFB_Stick
#define __FFB_Stick

#ifdef __cplusplus
 extern "C" {
#endif
#include "stdint.h"
extern const int32_t Pos_Max;

void stick_Init(void);
void HID_GenerateInputRpt(uint32_t *adcValue);
void stick_Set_Acutator_PWM(int32_t PWMvalue,uint8_t axes); //Direction Automatic Switch Enabled
void stick_EffectExecuter(void);
void stick_Position_Calibration(void);
int32_t stick_Get_Position(uint8_t axis);
int32_t stick_Get_Positioon_Max(void);
void stick_sendPos(void);

#ifdef __cplusplus
}
#endif

#endif /*__FFB_Stick*/



