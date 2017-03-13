#include "ffbManager.h"
#define PI 3.1415926535898f
//private variable
const uint16_t maxEffN = 10, maxTypeN = 4, maxParaBlkBytes = 20;
static uint8_t effVis[maxEffN]; // 0:free 1:occupied
static uint8_t effStat[maxEffN]; // 0:off 1:looping count
static uint32_t effRunTime[maxEffN]; // Effect Run Time
static uint8_t effRunning, actStat, effGlbGain;
//effect running flag, actuator enable flag, effect global gain (max==0xFF)
static PID_Set_Effect_Report effRpt[maxEffN]; //effect report pool
static uint8_t paraBlkPool[maxEffN * 2][maxParaBlkBytes]; //parameter block pool
static uint8_t paraType[maxEffN * 2]; //parameter block type -1==not visited
static int32_t PID_Tx, PID_Ty; //temp global variable
static float PID_mltipler;

//private function
void FFBMngrInit();
uint8_t FFBMngrMalloc(uint8_t *data);
void FFBMngrDelete(uint8_t effBlkIdx);
void FFBMngrEffAcpt(uint8_t *data);
void FFBMngrParaAcpt(uint8_t *data, uint8_t size, enum Report_ID_Enum type);
void FFBMngrOpre(uint8_t *data);
void FFBMngrCtrl(uint8_t *data);
void FFBMngrGain(uint8_t *data);
void FFBMngrBlkLd();
void FFBMngrStat();

uint8_t FFBFindOffset(uint8_t idx,enum Report_ID_Enum type);
void FFBMngrEnvlp(uint8_t effBlkIdx);
void FFBMngrPrid(uint8_t effBlkIdx, int32_t *level, float tfunc(uint32_t t,uint16_t p));
void FFBMngrCond(uint8_t effBlkIdx, int16_t pos);
void FFBMngrConstFoc(uint8_t idx);
void FFBMngrRampFoc(uint8_t idx);
float FFBtriangle(uint32_t t,uint16_t period);
void FFBMngrTrngl(uint8_t idx);
float FFBSine(uint32_t t, uint16_t period);
void FFBMngrSine(uint8_t idx);
void FFBMngrSprng(uint8_t idx);

void FFBMngrInit() {
    //DebugPrint("FFBMngr Init Clear\n");
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
    //uint8_t effType = data[1];
    uint8_t i;
    //DebugPrint("Eff Malloc type=0x%.2X\n", effType);
    //for(i = 0; i < maxTypeN; i++)
    //    if(effType == EffTypeArray[i]) break;
    //if(effType != EffTypeArray[i]) return -1; //type checking
    for(i = 0; i < maxEffN; i++)
        if(!effVis[i]) return i;
    return 0xFF;
}
void FFBMngrDelete(uint8_t effBlkIdx) {
        //DebugPrint("Delete Effect 0x%.2X\n", effBlkIdx);
        effVis[effBlkIdx] = 0;
        effRunTime[effBlkIdx] = 0;
        effStat[effBlkIdx] = 0;
        paraType[effBlkIdx * 2] = 0xff;
        paraType[effBlkIdx * 2 + 1] = 0xff;
}
void FFBMngrEffAcpt(uint8_t *data) {
    //data[0]==packetID
    uint8_t effBlkIdx = ((PID_Set_Effect_Report *) &data[1])->pid_effect_block_index;
    effRpt[effBlkIdx] = *((PID_Set_Effect_Report *) &data[1]);
    effVis[effBlkIdx] = 1; //set use flag and initialize
    effStat[effBlkIdx] = 0;
    effRunTime[effBlkIdx] = 0;
    //DebugPrint("Accept Effect[%d]\neffVis[i]=%d\n", effBlkIdx, effVis[effBlkIdx]);
    //DebugPrint("Effect Direction:0x%X Type:%d\n", effRpt[effBlkIdx].Direction, effRpt[effBlkIdx].PID_Effect_Type);
}
void FFBMngrParaAcpt(uint8_t *data, uint8_t size, enum Report_ID_Enum type) {
    //DebugPrint("Accepting Parameter\n");
    //data[0]==packetID, assume effIdx==data[1] offset==data[2]
    uint8_t effBlkIdx = data[1];
    uint8_t offset = paraType[effBlkIdx * 2] == 0xFF ? 0 : 1;
    offset = paraType[effBlkIdx * 2] == type ? 0 : offset;

    uint8_t idx = effBlkIdx * 2 + offset;
    paraType[idx] = type;
    //DebugPrint("Accept Parameter[%d] offset=%d,Type[%d*2+%d]=0x%.2X\n",\
     effBlkIdx, offset, effBlkIdx, offset, paraType[idx]);
    for(uint8_t i = 1; i < size; i++) //may cause out range
        paraBlkPool[idx][i-1] = data[i];
}
void FFBMngrOpre(uint8_t *data) {
    //data[0]==packetID
    PID_Effect_Operation_Report *effOp = (PID_Effect_Operation_Report *) &data[1];
    uint8_t effBlkIdx = effOp->pid_effect_block_index;
    uint8_t operation = data[2];
    uint8_t loop = data[3];
    //DebugPrint("Eff[%d] Operation: 0x%X\nLoop=%d\n", effBlkIdx, operation, loop);
    if(operation == PID_Op_Effect_Stop) {
        effStat[effBlkIdx] = 0;
        return;
    }
    if(operation == PID_Op_Effect_Start_Solo)
        for(uint8_t i = 0; i < maxEffN; i++)
            effStat[i] = 0;
    if(operation == PID_Op_Effect_Start_Solo || operation == PID_Op_Effect_Start){
        effStat[effBlkIdx] = loop;
        effRunning = 1;
    }
}
void FFBMngrCtrl(uint8_t *data) {
    //assume ctrl==data[1] arrayN=1
    enum PID_PID_Device_Control_Enum ctrl = (enum PID_PID_Device_Control_Enum) data[1];
    //DebugPrint("Eff Control:0x%.2X\n",(int) ctrl);
    switch(ctrl) {
    case PID_DC_Enable_Actuators:
        actStat = 1;
        break;
    case PID_DC_Disable_Actuators:
        actStat = 0;
        break;
    case PID_DC_Stop_All_Effects:
        for(uint8_t i = 0; i < maxEffN; i++)
            effStat[i] = 0;
        break;
    case PID_DC_Device_Reset:
        FFBMngrInit();
        break;
    case PID_DC_Device_Pause:
        effRunning = 0;
        break;
    case PID_DC_Device_Continue:
        effRunning = 1;
        break;
    }
}
void FFBMngrGain(uint8_t *data) {
    //assume gain==data[1]
    uint8_t gain = data[1];
    effGlbGain = gain;
    //DebugPrint("Device Gain=0x%.2X\n", gain);
}
void FFBMngrBlkLd(){
    //to be done
}
void FFBMngrStat(){
    //to be done
}
void FFBMngrDataServ(uint8_t *data, uint32_t cmd, uint8_t size) {
    //Output Pipe data FFB Service routine
    //assume reportID==data[0]
    enum Report_ID_Enum packetID = (enum Report_ID_Enum) data[0];
    uint8_t effBlkIdx = 0xff, effType = 0;
    //DebugPrint("\nFFBPacket:0x%.2X\nCmd:0x%X\n", data[0], cmd);
    switch(packetID) {
    case ID_PID_Set_Effect_Report:
        FFBMngrEffAcpt(data);
        break;
    case ID_PID_Set_Envelope_Report:
    case ID_PID_Set_Condition_Report:
    case ID_PID_Set_Periodic_Report:
    case ID_PID_Set_Constant_Force_Report:
    case ID_PID_Set_Ramp_Force_Report:
        FFBMngrParaAcpt(data, size, packetID);
        break;
    case ID_PID_Effect_Operation_Report:
        FFBMngrOpre(data);
        break;
    case ID_PID_PID_Device_Control_Report:
        FFBMngrCtrl(data);
        break;
    case ID_PID_Device_Gain_Report:
        FFBMngrGain(data);
        break;
    case ID_PID_Create_New_Effect_Report:
        effBlkIdx = FFBMngrMalloc(data);
        break;
    case ID_PID_PID_Block_Free_Report:
        FFBMngrDelete(data[1]);
        break;
    case ID_PID_PID_Block_Load_Report:
        FFBMngrBlkLd();
        break;
    case ID_PID_PID_State_Report:
        FFBMngrStat();
        break;
    };
}
uint8_t FFBFindOffset(uint8_t idx,enum Report_ID_Enum type){
    if(paraType[idx * 2] == type)
        return 0;
    else if(paraType[idx * 2 + 1] == type)
        return 1;
    //DebugPrint("\ntype[%d*2]:0x%X, 0x%X; Expected: 0x%X\n", idx, paraType[idx * 2], paraType[idx * 2 + 1], type);
    return 0xFF; //error return 0xFF
}
void FFBMngrEnvlp(uint8_t effBlkIdx) {
    uint8_t offset = FFBFindOffset(effBlkIdx,  ID_PID_Set_Envelope_Report); //find parameter offset

    PID_Set_Envelope_Report *pRpt =(PID_Set_Envelope_Report *) &paraBlkPool[effBlkIdx * 2 + offset];
    PID_Set_Effect_Report *eRpt = &effRpt[effBlkIdx];
    uint32_t runtime, pid_duration;
    float lambda;
    const int16_t nrmlLevel=10000;

    pid_duration = eRpt->pid_duration;
    runtime = effRunTime[effBlkIdx];
    lambda = 1.0; //envelope lambda
    PID_mltipler = 1.0; //initialize PID_mltipler

    if(runtime > pid_duration - pRpt->pid_fade_time) {
        lambda = (pid_duration - runtime) / ((float) pRpt->pid_fade_time);
        PID_mltipler = pRpt->pid_attack_level / ((float) nrmlLevel);
    }
    if(runtime < pRpt->pid_attack_time) {
        lambda = runtime / ((float) pRpt->pid_attack_time); //hope won't overflow
        PID_mltipler = pRpt->pid_attack_level / ((float) nrmlLevel);
    }
    PID_mltipler += (1 - PID_mltipler) * lambda;
}
void FFBMngrPrid(uint8_t effBlkIdx, int32_t *level, float tfunc(uint32_t t,uint16_t p)) {
    uint8_t offset = FFBFindOffset(effBlkIdx, ID_PID_Set_Periodic_Report); //find parameter offset

    PID_Set_Periodic_Report *pRpt =(PID_Set_Periodic_Report *) &paraBlkPool[effBlkIdx * 2 + offset];
    pRpt->pid_period = pRpt->pid_period == 0 ? 1 : pRpt->pid_period;
    uint32_t time = (pRpt->pid_phase + effRunTime[effBlkIdx]) % pRpt->pid_period; //assume time unit: ms

    *level =(int16_t) pRpt->pid_offset;
    *level +=(int32_t) ((int16_t) pRpt->pid_magnitude) * tfunc(time, pRpt->pid_period);
}
void FFBMngrCond(uint8_t effBlkIdx, int16_t pos){
    uint8_t offset = FFBFindOffset(effBlkIdx, ID_PID_Set_Condition_Report); //find parameter offset

    PID_Set_Condition_Report *pRpt =(PID_Set_Condition_Report *) &paraBlkPool[effBlkIdx * 2 + offset];
    //PID_Set_Effect_Report *eRpt = &effRpt[effBlkIdx];
    uint16_t CP = pRpt->pid_cp_offset;
    uint16_t dead = pRpt->pid_dead_band;

    PID_mltipler = 0;
    if(pos < CP - dead){
        PID_mltipler = pRpt->pid_negative_coefficient * (pos - (CP - dead)) / pRpt->pid_negative_saturation;
    }else if(pos > CP + dead){
        PID_mltipler = pRpt->pid_positive_saturation * (pos - (CP + dead)) / pRpt->pid_positive_saturation;
    }
}
void FFBMngrConstFoc(uint8_t idx) {
    int32_t magnitude;
    FFBMngrEnvlp(idx);

    //DebugPrint("Type ID_PID_Set_Constant_Force_Report:0x%X\n",ID_PID_Set_Constant_Force_Report);
    enum Report_ID_Enum type = ID_PID_Set_Constant_Force_Report;
    uint8_t offset = FFBFindOffset(idx, type);
    PID_Set_Constant_Force_Report *con = (PID_Set_Constant_Force_Report *) paraBlkPool[idx * 2 + offset];
    magnitude = offset == 0xFF ? 10000 : con->pid_magnitude;
    //DebugPrint("ParaConst[%d*2+%d]: Magnitude:%d\n", idx, offset, magnitude);
    PID_Tx = PID_Ty =(int32_t) magnitude * PID_mltipler;
    //DebugPrint("ConstantForce[%d] envelope PID_mltipler:%lf\n", idx, PID_mltipler);
}
void FFBMngrRampFoc(uint8_t idx) {
    FFBMngrEnvlp(idx);

    uint8_t offset = FFBFindOffset(idx, ID_PID_Set_Ramp_Force_Report);
    PID_Set_Ramp_Force_Report *ram = (PID_Set_Ramp_Force_Report *) paraBlkPool[idx *2 + offset];
    // normalized start/end -10000--10000
    float lambda = effRunTime[idx] / (float) effRpt[idx].pid_duration;
    PID_Tx = ram->pid_ramp_start;
    PID_Tx += (ram->pid_ramp_end - ram->pid_ramp_start) * lambda;
    PID_Tx = PID_Tx * PID_mltipler;
    PID_Ty = PID_Tx;
    //DebugPrint("Ramp PID_mltipler:%f lambda:%f\n", PID_mltipler, lambda);
    //DebugPrint("Start:%d End:%d\n", ram->Start, ram->End);
}
float FFBtriangle(uint32_t t,uint16_t period){
    float phase = t / (float) period;
    if(phase < 0.5){
        phase = phase * 2;
    }else{
        phase = (1 - phase) * 2;
    }
    phase = phase * 2 - 1;
    return phase;
}
void FFBMngrTrngl(uint8_t idx) {
    PID_mltipler = 1.0;
    int32_t level;
    FFBMngrEnvlp(idx);
    FFBMngrPrid(idx, &level, FFBtriangle);
    PID_Tx = PID_Ty = (int32_t) level * PID_mltipler;
    //DebugPrint("Triangle PID_mltipler:%f level:%d\n", PID_mltipler, level);
}
float FFBSine(uint32_t t, uint16_t period) {
    float phase = t / (float)period * 2 * PI;
    return sin(phase);
}
void FFBMngrSine(uint8_t idx) {
    PID_mltipler = 1.0;
    int32_t level;
    FFBMngrEnvlp(idx);
    FFBMngrPrid(idx, &level, FFBSine);
    PID_Tx = PID_Ty = (int32_t)level * PID_mltipler;
    //DebugPrint("Sine PID_mltipler:%f level:%d\n", PID_mltipler, level);
}
void FFBMngrSprng(uint8_t idx) {
    PID_Tx = PID_Ty = 0; //to be done, need pos(&posx, &posy) as parameter to ger position
}
void FFBMngrEffRun(uint16_t deltaT, int32_t *Tx, int32_t *Ty) {
    //given deltaTime(ms),return Torque on x&y axes
    *Tx = 0;
    *Ty = 0;
    for(uint8_t i = 0; i < maxEffN; i++) {
        PID_Set_Effect_Report *eRpt = &effRpt[i];
        //if (effStat[i] > 0) //DebugPrint("EffStat[%d] > 0\n", i);
        if(effVis[i] && effStat[i] > 0 && effRunning) {
            ////DebugPrint("Eff Running: 0x%.2X\n",i);
            effRunTime[i] += deltaT; //update runtime
            uint16_t pid_duration = eRpt->pid_duration;
            ////DebugPrint("Effect Playing pid_duration:%d ms(0xFFFF==65535==Infinite)\n", pid_duration);
            //DebugPrint("Effect RunTime=%d ms\n", effRunTime[i]); //0xFFFF->infinite assume in million seconds (exponent(-3))
            if (effRunTime[i] > pid_duration && pid_duration != 0xFFFF) {
                //DebugPrint("EffDone:%d\n",i);
                effStat[i]--;
                effRunTime[i] = 0;
                continue;
            }
            //Trigger Button Judge/Repeat Interval(to be done)

            //PID_Tx = 0, PID_Ty = 0; //torque on x&y axes
                        //Use Global variable PID_Tx&PID_Ty as return
            switch(eRpt->PID_Effect_Type.pid_effect_type_enum_0) {
            case PID_ET_Constant_Force:
                FFBMngrConstFoc(i);
                break;
            case PID_ET_Ramp:
                FFBMngrRampFoc(i);
                break;
            case PID_ET_Triangle:
                FFBMngrTrngl(i);
                break;
            case PID_ET_Spring:
                FFBMngrSprng(i);
                break;
            case PID_ET_Sine:
                FFBMngrSine(i);
                break;
            }

            float x, y; //normalized value max 0x7f?
            uint8_t gain = eRpt->pid_gain;
            const uint8_t maxGain = 0xFF;
            if(eRpt->pid_direction_enable > 0) { //<USB PID Definition> massed there up
                uint8_t direction = eRpt->PID_Direction.Pointer_ID.x_id; //normalized value 0x00-0xFF
                x = cos((float) (-direction * 2 * PI / 0xFF + PI / 2));
                y = sin((float) (-direction * 2 * PI / 0xFF + PI / 2));
                //DebugPrint("Effect Direction:%f degree\n", direction * 360.0 / 0xFF);
            } else {
                x = eRpt->PID_Direction.Pointer_ID.x_id / 0xFF;
                y = eRpt->PID_Direction.Pointer_ID.y_id / 0xFF;
                //DebugPrint("Effect Direction X:%d Y:%d\n", x, y);
            }
            *Tx +=(int32_t) (PID_Tx * gain *  x / maxGain);
            *Ty +=(int32_t) (PID_Ty * gain *  y / maxGain);
            //DebugPrint("FFBMngr Running:Type=%d\n PID_Tx:%d PID_Ty:%d\n Gain:%d\n"\
            ,eRpt->PID_Effect_Type, PID_Tx, PID_Ty, gain);
        }
    }
}
