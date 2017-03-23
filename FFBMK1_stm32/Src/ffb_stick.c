#include "ffb_stick.h"
#include "ffb_manager.h"
#include "usbd_pid.h"
#include "stm32f1xx_hal.h"

/*import variable */
extern TIM_HandleTypeDef htim3;
extern uint8_t actStat;
/* private Variable */
static int32_t X_Position, Y_Position; //Value -2048~2047:Angle -180~180
static int32_t X_Zero = 0, Y_Zero = 0;
static uint8_t HID_Button_Status = 0;
const int32_t Angle_Max = 30;
const int32_t Position_Gain = 180 / Angle_Max; //p*gain-> Value -2048~2047:Angle -30~30
const int32_t Pos_Max = 341; //pos Constrian value 2048 / 180 * 30 <= 341
const int32_t T_Max = 10000;

const int16_t HID_In_Report_len = 7;
uint8_t HID_In_Report[HID_In_Report_len];

/* private function prototype*/
int32_t truncVal(int32_t val, int32_t range);

/* function */
inline int32_t truncVal(int32_t val, int32_t range){
	if(val <= range || val >= -range) //in [-range, range]
		return val;
	else
		// value exceed
		return val > 0 ? range : -range;
}
void HID_GenerateInputRpt(uint32_t *adcValue) {
  int32_t x, y;
  X_Position = adcValue[0] - X_Zero;
  Y_Position = adcValue[1] - Y_Zero;
  X_Position = truncVal(X_Position, Pos_Max);
  Y_Position = truncVal(Y_Position, Pos_Max);
  x = X_Position * Position_Gain;
  y = Y_Position * Position_Gain;
  HID_In_Report[0] = 1; //Report ID==1
  HID_In_Report[1] = (uint8_t) (adcValue[2] >> 4); //throttle
  HID_In_Report[2] = (uint8_t) (x & 0x00ff); //x pos: -2047~2048
  HID_In_Report[3] = (uint8_t) (x >> 8);
  HID_In_Report[4] = (uint8_t) (y & 0x00ff); //y pos
  HID_In_Report[5] = (uint8_t) (y >> 8);
  HID_In_Report[6] = HID_Button_Status; //button

  USBD_PID_Send(HID_In_Report, HID_In_Report_len); //send In Report
}
void stick_Set_Acutator_PWM(int32_t PWMvalue, uint32_t axes) { //Direction Automatic Switch Enabled
	uint32_t PWMchannel,PWMchannel2,temp;
	//axes=0:X  axes=1:Y
	//Assume acutator in H driver: X(1,2) Y(3,4)
	if (axes==0){
		PWMchannel=TIM_CHANNEL_1;
		PWMchannel2=TIM_CHANNEL_2;
	}
	else{
		PWMchannel=TIM_CHANNEL_3;
		PWMchannel2=TIM_CHANNEL_4;
	};
	if (PWMvalue<0){ //swap channel, abs(pwmValue)
		temp=PWMchannel2;
		PWMchannel2=PWMchannel;
		PWMchannel=temp;
		PWMvalue=-PWMvalue;
	};
	TIM_OC_InitTypeDef sConfigOC; //Start selected arm/channel
	sConfigOC.OCMode=TIM_OCMODE_PWM1;
	sConfigOC.Pulse=(uint32_t) 0;
	sConfigOC.OCPolarity=TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode=TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&htim3,&sConfigOC,PWMchannel2); //shutdown channel2
	HAL_TIM_PWM_Stop(&htim3,PWMchannel2);

	sConfigOC.Pulse=(uint32_t) PWMvalue;
	HAL_TIM_PWM_ConfigChannel(&htim3,&sConfigOC,PWMchannel); //open channel1
	HAL_TIM_PWM_Start(&htim3,PWMchannel);
}
void stick_EffectExecuter(void) {
	static uint32_t Run_Time,pre_Run_Time=0;//,time_4ms=0;
	const int32_t PWM_pulseMax=800; // 180/n degree range
	int32_t x,y;
	Run_Time=HAL_GetTick(); //1ms 24-bit tick max=4.6h
	if(Run_Time > 0xffffff - 0xffff){
		Run_Time = 0;
		pre_Run_Time = 0;
	} //refresh after 4.4h

	if(actStat){
		//acting
		FFBMngrEffRun((uint16_t) Run_Time - pre_Run_Time, &x, &y);
		x = (int32_t) (x / (float) T_Max) * PWM_pulseMax;
		y = (int32_t) (y / (float) T_Max) * PWM_pulseMax;
		x = x % PWM_pulseMax;
		y = y % PWM_pulseMax; //make sure x,y < PWM_pulseMax
		//PC_12 On
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);
	}else{
		//defalut Spring P Control
		//direction: reverse to cartesian coordinates
		x = (int32_t) ((X_Position) / (float) Pos_Max) * PWM_pulseMax; //Simple Pars er
		y = (int32_t) ((Y_Position) / (float) Pos_Max) * PWM_pulseMax; //Propotion
		x = x % PWM_pulseMax;
		y = y % PWM_pulseMax; //make sure x,y < PWM_pulseMax
		//PC_12 Off
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
	}
	stick_Set_Acutator_PWM(x,0); //set axis 0
	stick_Set_Acutator_PWM(y,1); //set axis 1

	pre_Run_Time=Run_Time;
}
void stick_Position_Calibration(void) {
	X_Zero=X_Position;
	Y_Zero=Y_Position;
}
int32_t stick_Get_Position(uint8_t axis){
	if(axis == 0) return(X_Position);
	else return(Y_Position);
}
int32_t stick_Get_Positioon_Max(void){
	return Pos_Max;
}





