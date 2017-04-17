#ifndef __DEVICE_BRIDGE
#define __DEVICE_BRIDGE

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#define DEVICE_BRIDGE_ON 0

void BridgeOutServ(uint8_t *data, uint16_t len);
void BridgeGetXY(int32_t *Tx, int32_t *Ty);
	 
#ifdef __cplusplus
}
#endif

#endif /*__DEVICE_BRIDGE*/
