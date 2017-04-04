#pragma once
#ifndef __FFBMngr
#define __FFBMngr

#include"stdint.h"
void FFBMngrInit();
void FFBMngrDataServ(uint8_t *data, uint32_t cmd, uint8_t size);
void FFBMngrEffRun(uint16_t deltaT, int32_t &Tx, int32_t &Ty);
void FFBMngrSetPos(int16_t x, int16_t y);

#endif // !__FFBMngr
