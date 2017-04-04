#include "device_bridge.h"
#include "usbd_pid.h"

static int16_t x = 10000, y = 10000;

void BridgeOutServ(uint8_t *data, uint16_t len){
	x = ((data[3] << 8) | data[2]);
	y = ((data[5] << 8) | data[4]);
	//echo data
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_10); //debug toggle PC_10
}
void BridgeGetXY(int32_t *Tx, int32_t *Ty){
	*Tx = (int32_t) x;
	*Ty = (int32_t) y;
}
