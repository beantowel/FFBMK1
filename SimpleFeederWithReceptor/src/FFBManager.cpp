#include "FFBManager.h"
#include "deviceBridge.h"
#include "stdafx.h"
//#include "stdint.h"
#include "Math.h"
#include "public.h"
#include "vjoyinterface.h"
#define float32_t double
#define PI 3.14159265358979f
#define DebugPrint printf

const uint16_t maxEffN = 10, maxTypeN = 4, maxParaBlkBytes = 20, maxPos = 2048;
uint8_t effVis[maxEffN]; // 0:free 1:occupied
uint8_t effStat[maxEffN]; // 0:off 1:looping count
uint32_t effRunTime[maxEffN]; // Effect Run Time
uint8_t effRunning, actStat, effGlbGain;
//effect running flag, actuator enable flag, effect global gain (max==0xFF)
FFB_EFF_REPORT effRpt[maxEffN]; //effect report pool
uint8_t paraBlkPool[maxEffN * 2][maxParaBlkBytes]; //parameter block pool
uint8_t paraType[maxEffN * 2]; //parameter block type -1==not visited

void FFBMngrInit() {
	DebugPrint("FFBMngr Init Clear\n");
    effRunning = 0;
    actStat = 1;
    effGlbGain = 0xff;
    memset(effVis, 0, sizeof(effVis));
    memset(effStat, 0, sizeof(effStat));
    memset(effRunTime, 0, sizeof(effRunTime));
    memset(paraType, -1, sizeof(paraType));
}
uint8_t FFBMngrMalloc(uint8_t *data) {
    //return a effect block index,
    //if not avaliable or can't perform the effect,return -1
    //assume type==data[1]
    uint8_t effType = data[1];
    uint8_t i;
	DebugPrint("Eff Malloc type=0x%.2X\n", effType);
    //for(i = 0; i < maxTypeN; i++)
    //    if(effType == EffTypeArray[i]) break;
    //if(effType != EffTypeArray[i]) return -1; //type checking
    for(i = 1; i < maxEffN; i++)
        if(!effVis[i]) return i;
    return -1;
}
void FFBMngrDelete(uint8_t effBlkIdx) {
	DebugPrint("Delete Effect 0x%.2X\n", effBlkIdx);
    effVis[effBlkIdx] = 0;
	effRunTime[effBlkIdx] = 0;
	effStat[effBlkIdx] = 0;
    paraType[effBlkIdx * 2] = -1;
    paraType[effBlkIdx * 2 + 1] = -1;
}
void FFBMngrEffAcpt(uint8_t *data) {
	//data[0]==packetID
    uint8_t effBlkIdx = ((FFB_EFF_REPORT *) &data[1])->EffectBlockIndex;
    effRpt[effBlkIdx] = *((FFB_EFF_REPORT *) &data[1]);
    effVis[effBlkIdx] = 1; //set use flag and initialize
    effStat[effBlkIdx] = 0;
    effRunTime[effBlkIdx] = 0;
	DebugPrint("Accept Effect[%d]\neffVis[i]=%d\n", effBlkIdx, effVis[effBlkIdx]);
	DebugPrint("Effect Direction:0x%X Type:%d\n", effRpt[effBlkIdx].Direction, effRpt[effBlkIdx].EffectType);
}
void FFBMngrParaAcpt(uint8_t *data, uint8_t size, FFBPType type) {
	DebugPrint("Accepting Parameter\n");
    //data[0]==packetID, assume effIdx==data[1] offset==data[2]
    uint8_t effBlkIdx = data[1];
	uint8_t offset = paraType[effBlkIdx * 2] == 0xFF ? 0 : 1;
	offset = paraType[effBlkIdx * 2] == type ? 0 : offset;
	if (type == PT_CONDREP) {
		FFB_EFF_COND *pRpt = (FFB_EFF_COND *)&data[1];
		offset = pRpt->isY ? 1 : 0;
	}

    uint8_t idx = effBlkIdx * 2 + offset;
	DebugPrint("Accept Parameter[%d] offset=%d,Type[%d*2+%d]=0x%.2X\n", \
	effBlkIdx, offset, effBlkIdx, offset, paraType[idx]);

    paraType[idx] = type;
    for(uint8_t i = 1; i < size; i++) //may cause out range
        paraBlkPool[idx][i-1] = data[i];
}
void FFBMngrOpre(uint8_t *data) {
	//data[0]==packetID
    FFB_EFF_OP *effOp = (FFB_EFF_OP *) &data[1];
    uint8_t effBlkIdx = effOp->EffectBlockIndex;
	uint8_t operation = data[2];
    uint8_t loop = data[3];
	DebugPrint("Eff[%d] Operation: 0x%X\nLoop=%d\n", effBlkIdx, operation, loop);
    if(operation == EFF_STOP) {
        effStat[effBlkIdx] = 0;
        return;
    }
    if(operation == EFF_SOLO)
        for(uint8_t i = 1; i < maxEffN; i++)
            effStat[i] = 0;
    if(operation == EFF_SOLO || operation == EFF_START){
        effStat[effBlkIdx] = loop;
		effRunning = 1;
	}
}
void FFBMngrCtrl(uint8_t *data) {
    //assume ctrl==data[1] arrayN=1
	FFB_CTRL ctrl = (FFB_CTRL) data[1];
	DebugPrint("Eff Control:0x%.2X\n",(int) ctrl);
    switch(ctrl) {
    case CTRL_ENACT:
        actStat = 1;
        break;
    case CTRL_DISACT:
        actStat = 0;
        break;
    case CTRL_STOPALL:
        for(uint8_t i = 1; i < maxEffN; i++)
            effStat[i] = 0;
        break;
    case CTRL_DEVRST:
        FFBMngrInit();
        break;
    case CTRL_DEVPAUSE:
        effRunning = 0;
        break;
    case CTRL_DEVCONT:
        effRunning = 1;
        break;
    }
}
void FFBMngrGain(uint8_t *data) {
    //assume gain==data[1]
    uint8_t gain = data[1];
    effGlbGain = gain;
	DebugPrint("Device Gain=0x%.2X\n", gain);
}
void FFBMngrDataServ(uint8_t *data, uint32_t cmd, uint8_t size) {
    //Output Pipe data FFB Service routine
    //assume reportID==data[0]
    FFBPType packetID = (FFBPType) data[0];
    uint8_t effBlkIdx = -1, effType = 0;
	DebugPrint("\nFFBPacket:0x%.2X\nCmd:0x%X\n", data[0], cmd);
    if(cmd == 0x000B000F)
		packetID = (FFBPType)(packetID - 0x10);
    switch(packetID) {
    case PT_EFFREP:
        FFBMngrEffAcpt(data);
        break;
    case PT_ENVREP:
    case PT_CONDREP:
    case PT_PRIDREP:
    case PT_CONSTREP:
    case PT_RAMPREP:
        FFBMngrParaAcpt(data, size, packetID);
        break;
    case PT_EFOPREP:
        FFBMngrOpre(data);
        break;
    case PT_CTRLREP:
        FFBMngrCtrl(data);
        break;
    case PT_GAINREP:
        FFBMngrGain(data);
        break;
	case PT_NEWEFREP:
		effBlkIdx = FFBMngrMalloc(data);
		break;
	case PT_BLKFRREP:
		FFBMngrDelete(data[1]);
    };
}
uint8_t FFBFindOffset(uint8_t idx,FFBPType type){
    if(paraType[idx * 2] == type)
        return 0;
    else if(paraType[idx * 2 + 1] == type)
        return 1;
	DebugPrint("\ntype[%d*2]:0x%X, 0x%X; Expected: 0x%X\n", idx, paraType[idx * 2], paraType[idx * 2 + 1], type);
    return 0xFF; //error return 0xFF
}
void FFBMngrEnvlp(uint8_t effBlkIdx, float32_t &mltipler) {
    uint8_t offset = FFBFindOffset(effBlkIdx,  PT_ENVREP); //find parameter offset

    FFB_EFF_ENVLP *pRpt =(FFB_EFF_ENVLP *) &paraBlkPool[effBlkIdx * 2 + offset];
    FFB_EFF_REPORT *eRpt = &effRpt[effBlkIdx];
    uint32_t runtime, duration;
    float32_t lambda;
    const int16_t nrmlLevel=10000;

    duration = eRpt->Duration;
    runtime = effRunTime[effBlkIdx];
    lambda = 1.0; //envelope lambda
    mltipler = 1.0; //initialize mltipler

    if(runtime > duration - pRpt->FadeTime) {
        lambda = (duration - runtime) / ((float32_t) pRpt->FadeTime);
        mltipler = pRpt->FadeLevel / ((float32_t) nrmlLevel);
    }
    if(runtime < pRpt->AttackTime) {
        lambda = runtime / ((float32_t) pRpt->AttackTime); //hope won't overflow
        mltipler = pRpt->AttackLevel / ((float32_t) nrmlLevel);
    }
    mltipler += (1 - mltipler) * lambda;
}
void FFBMngrPrid(uint8_t effBlkIdx, int32_t &level, float32_t tfunc(uint32_t t,uint16_t p)) {
    uint8_t offset = FFBFindOffset(effBlkIdx, PT_PRIDREP); //find parameter offset

    FFB_EFF_PERIOD *pRpt =(FFB_EFF_PERIOD *) &paraBlkPool[effBlkIdx * 2 + offset];
    FFB_EFF_REPORT *eRpt = &effRpt[effBlkIdx];
	pRpt->Period = pRpt->Period == 0 ? 1 : pRpt->Period;
    uint32_t time = (pRpt->Phase + effRunTime[effBlkIdx]) % pRpt->Period; //assume time unit: ms

    level =(int16_t) pRpt->Offset;
    level +=(int32_t) ((int16_t) pRpt->Magnitude) * tfunc(time, pRpt->Period);
}
void FFBMngrCond(uint8_t effBlkIdx, int16_t pos, uint8_t isY, float32_t &mltipler){
    uint8_t offset;
	static uint8_t dir = 0;
    FFB_EFF_COND *pRpt = (FFB_EFF_COND *) &paraBlkPool[effBlkIdx * 2 + 0];
	offset = dir; dir = (dir == 0) ? 1 : 0;
	pRpt = (FFB_EFF_COND *)&paraBlkPool[effBlkIdx * 2 + offset];
    FFB_EFF_REPORT *eRpt = &effRpt[effBlkIdx];
    uint16_t CP = pRpt->CenterPointOffset;
    uint16_t dead = pRpt->DeadBand;

    mltipler = 0;
    if(pos < CP - dead){
		if(pRpt->NegSatur == 0) pRpt->NegSatur = 10000;
        mltipler = pRpt->NegCoeff * (pos - (CP - dead)) / ((float) maxPos) / (float) pRpt->NegSatur;
    }else if(pos > CP + dead){
		if(pRpt->PosSatur == 0) pRpt->PosSatur = 10000;
        mltipler = pRpt->PosCoeff * (pos - (CP + dead)) / ((float) maxPos) / (float) pRpt->PosSatur;
    }
	DebugPrint("Condition[%d] NegSatur=%d PosSatur=%d mltip=%f\n", offset, pRpt->NegSatur,\
	pRpt->PosSatur, mltipler);
}
void FFBMngrConstFoc(uint8_t idx, int32_t &Tx, int32_t &Ty) {
    float32_t mltipler;
    int32_t magnitude;
    FFBMngrEnvlp(idx, mltipler);

	DebugPrint("Type PT_CONSTREP:0x%X\n",PT_CONSTREP);
	FFBPType type = PT_CONSTREP;
    uint8_t offset = FFBFindOffset(idx, type);
    FFB_EFF_CONSTANT *con = (FFB_EFF_CONSTANT *) paraBlkPool[idx * 2 + offset];
    magnitude = offset == 0xFF ? 10000 : con->Magnitude;
	DebugPrint("ParaConst[%d*2+%d]: Magnitude:%d\n", idx, offset, magnitude);
    Tx = Ty =(int32_t) magnitude * mltipler;
	DebugPrint("ConstantForce[%d] envelope mltipler:%lf\n", idx, mltipler);
}
void FFBMngrRampFoc(uint8_t idx, int32_t &Tx, int32_t &Ty) {
    float32_t mltipler;
    FFBMngrEnvlp(idx, mltipler);

    uint8_t offset = FFBFindOffset(idx, PT_RAMPREP);
    FFB_EFF_RAMP *ram = (FFB_EFF_RAMP *) paraBlkPool[idx *2 + offset];
    // normalized start/end -10000--10000
    float32_t lambda = effRunTime[idx] / (float32_t) effRpt[idx].Duration;
    Tx = ram->Start;
    Tx += (ram->End - ram->Start) * lambda;
    Tx = Tx * mltipler;
    Ty = Tx;
	DebugPrint("Ramp mltipler:%f lambda:%f\n", mltipler, lambda);
	DebugPrint("Start:%d End:%d\n", ram->Start, ram->End);
}
float32_t FFBtriangle(uint32_t t,uint16_t period){
    float32_t phase = t / (float32_t) period;
    if(phase < 0.5){
        phase = phase * 2;
    }else{
        phase = (1 - phase) * 2;
    }
	phase = phase * 2 - 1;
	return phase;
}
void FFBMngrTrngl(uint8_t idx, int32_t &Tx, int32_t &Ty) {
    float32_t mltipler = 1.0;
    int32_t level;
    FFBMngrEnvlp(idx, mltipler);
    FFBMngrPrid(idx, level, FFBtriangle);
    Tx = Ty = (int32_t) level * mltipler;
	DebugPrint("Triangle mltipler:%f level:%d\n", mltipler, level);
}
float32_t FFBSine(uint32_t t, uint16_t period) {
	float32_t phase = t / (float32_t)period * 2 * PI;
	return sin(phase);
}
void FFBMngrSine(uint8_t idx, int32_t &Tx, int32_t &Ty) {
	float32_t mltipler = 1.0;
	int32_t level;
	FFBMngrEnvlp(idx, mltipler);
	FFBMngrPrid(idx, level, FFBSine);
	Tx = Ty = (int32_t)level * mltipler;
	DebugPrint("Sine mltipler:%f level:%d\n", mltipler, level);
}
void FFBMngrSprng(uint8_t idx, int32_t &Tx, int32_t &Ty) {
    Tx = Ty = 0; //to be done, need pos(&posx, &posy) as parameter to ger position
	float32_t mltipler = 0;
	PInputReport posHandle = DeviceGetPos();
	FFBMngrCond(idx, posHandle->x, 0, mltipler);
	Tx = 10000 * mltipler;
	FFBMngrCond(idx, posHandle->y, 0, mltipler);
	Ty = 10000 * mltipler;
	DebugPrint("Spring: PosX:%d,PosY:%d,Tx:%d,Ty:%d\n", posHandle->x, posHandle->y, Tx, Ty);
}
void FFBMngrEffRun(uint16_t deltaT, int32_t &Tx, int32_t &Ty) {
    //given deltaTime(ms),return Torque on x&y axes
    Tx = 0;
    Ty = 0;
    for(uint8_t i = 0; i < maxEffN; i++) {
        FFB_EFF_REPORT *eRpt = &effRpt[i];
		//if (effStat[i] > 0) DebugPrint("EffStat[%d] > 0\n", i);
        if(effVis[i] && effStat[i] > 0 && effRunning) {
			//DebugPrint("Eff Running: 0x%.2X\n",i);
            effRunTime[i] += deltaT; //update runtime
            uint16_t duration = eRpt->Duration;
			//DebugPrint("Effect Playing Duration:%d ms(0xFFFF==65535==Infinite)\n", duration);
			DebugPrint("Effect RunTime=%d ms\n", effRunTime[i]); //0xFFFF->infinite assume in million seconds (exponent(-3))
            if (effRunTime[i] > duration && duration != 0xFFFF) {
				DebugPrint("EffDone:%d\n",i);
                effStat[i]--;
				effRunTime[i] = 0;
                continue;
            }
            //Trigger Button Judge/Repeat Interval(to be done)

            int32_t FFBTx = 0, FFBTy = 0; //torque on x&y axes
            switch(eRpt->EffectType) {
            case ET_CONST:
                FFBMngrConstFoc(i, FFBTx, FFBTy);
                break;
            case ET_RAMP:
                FFBMngrRampFoc(i, FFBTx, FFBTy);
                break;
            case ET_TRNGL:
                FFBMngrTrngl(i, FFBTx, FFBTy);
                break;
            case ET_SPRNG:
                FFBMngrSprng(i, FFBTx, FFBTy);
                break;
			case ET_SINE:
				FFBMngrSine(i, FFBTx, FFBTy);
				break;
            }

            float32_t x, y; //normalized value max 0x7f?
            uint8_t gain = eRpt->Gain;
            const uint8_t maxGain = 0xFF;
            if(eRpt->Polar == 4) {
                uint8_t direction = eRpt->Direction; //normalized value 0x00-0xFF
                x = cos(float32_t (-direction * 2 * PI / 0xFF + PI / 2));
                y = sin(float32_t (-direction * 2 * PI / 0xFF + PI / 2));
				DebugPrint("Effect Direction:%f degree\n", direction * 360.0 / 0xFF);
            } else {
                x = eRpt->DirX / 0xFF;
                y = eRpt->DirY / 0xFF;
				DebugPrint("Effect Direction X:%d Y:%d\n", x, y);
            }
			if (eRpt->EffectType == ET_SPRNG) {
				x = 1;
				y = 1;
			}
            Tx +=(int32_t) (FFBTx * gain *  x / maxGain);
            Ty +=(int32_t) (FFBTy * gain *  y / maxGain);
			DebugPrint("FFBMngr Running:Type=%d\n Tx:%d Ty:%d\n Gain:%d\n"\
			,eRpt->EffectType, Tx, Ty, gain);
        }
    }
}