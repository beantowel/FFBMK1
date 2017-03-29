#ifndef __FFB_Mngr
#define __FFB_Mngr

#ifdef __cplusplus
 extern "C" {
#endif
#include "stdint.h"
/** @defgroup FFBMngr_CORE_Exported_Functions
  * @{
  */
void FFBMngrDataOutServ(uint8_t *data, uint16_t size);
void FFBMngrFeatureServ(uint8_t rptID, uint8_t dir, uint8_t *data);
void FFBMngrEffRun(uint16_t deltaT, int32_t *Tx, int32_t *Ty);
void FFBMngrInit(void);	 
uint8_t FFBMngrStat(void);
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /*__FFB_Mngr*/
