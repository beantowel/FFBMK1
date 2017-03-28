#pragma once
#ifndef __FFBMngr
#define __FFBMngr

#include"stdint.h"
void FFBMngrEffRun(uint16_t deltaT, int32_t &Tx, int32_t &Ty);
void FFBMngrSetPos(int16_t x, int16_t y);

#endif // !__FFBMngr
