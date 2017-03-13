#ifndef __FFB_Mngr
#define __FFB_Mngr

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "math.h"
#include "string.h"
#include "stdint.h"
#include "pid_definition.h"
/** @defgroup FFBMngr_CORE_Exported_Functions
  * @{
  */ 
void FFBMngrDataServ(uint8_t *data, uint32_t cmd, uint8_t size);
void FFBMngrEffRun(uint16_t deltaT, int32_t *Tx, int32_t *Ty);
/**
  * @}
  */  

#ifdef __cplusplus
}
#endif

#endif /*__FFB_Mngr*/
