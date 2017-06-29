/*
  Force Feed Back Manager for PID Device
  Handles the communication between host and device
  parse the data/controls to 2-axis normalized Torque
*/
#include "ffb_manager.h"
#include "usbd_pid.h"
#include "pid_definition.h"
#include "ffb_stick.h"
#include "device_bridge.h"
#include "usbd_hid.h"
#include "math.h"
#include "string.h"
#define PI 3.1415926535898f

//private variable
static const uint16_t maxEffN = 21, maxParaBlkBytes = 20;
static const uint8_t EffTypeArray[] = { //supported effect types
    PID_ET_Constant_Force,
    PID_ET_Ramp,
    PID_ET_Triangle,
    PID_ET_Spring,
    PID_ET_Sine,
    //previous 5 type
    //new types
    PID_ET_Square,
    PID_ET_Sawtooth_Up,
    PID_ET_Sawtooth_Down,
    PID_ET_Damper,
    PID_ET_Inertia,
    //PID_ET_Friction,
};

static uint8_t effVis[maxEffN]; // 0:free 1:occupied
static uint8_t effStat[maxEffN]; // 0:off >=1:looping count
static uint32_t effRunTime[maxEffN]; // Effect Run Time
static uint8_t effRunning = 0, effGlbGain = 0xff;
static uint8_t actStat = 1; //actuator On
static enum PID_Block_Load_Status_Enum pidLdStat = PID_Block_Load_Error; //Blk_Load_Status

static PID_Set_Effect_Report effRpt[maxEffN]; //effect report pool, index == 0 wasted
static uint8_t paraBlkPool[maxEffN * 2][maxParaBlkBytes]; //parameter block pool
static uint8_t paraType[maxEffN * 2]; //parameter block type -1==not visited

static uint8_t mallocIdx = 0;

//exported function
void FFBMngrDataOutServ(uint8_t *data, uint16_t size);
void FFBMngrFeatureServ(uint8_t rptID, uint8_t dir, uint8_t *data);
void FFBMngrDataInServ(uint8_t rptID);
void FFBMngrEffRun(uint16_t deltaT, int32_t *Tx, int32_t *Ty);
void FFBMngrInit(void);

//private function
//Input
void FFBMngrStat(uint16_t *len, uint8_t *inputReport);
//Output
void FFBMngrEffAcpt(uint8_t *data);
void FFBMngrParaAcpt(uint8_t *data, uint16_t size);
void FFBMngrOpre(uint8_t *data);
void FFBMngrDelete(uint8_t effBlkIdx);
void FFBMngrCtrl(uint8_t *data);
void FFBMngrGain(uint8_t *data);
//Feature
uint8_t FFBMngrMalloc(uint8_t *data);
void FFBMngrBlkLd(uint8_t idx, uint16_t *len, uint8_t *inputReport);
void FFBMngrPoolReport(uint16_t *len, uint8_t *inputReport);
//parameter calculation
uint8_t FFBFindOffset(uint8_t idx, uint8_t type);
void FFBMngrEnvlp(uint8_t effBlkIdx, float *PID_mltipler);
void FFBMngrPrid(uint8_t effBlkIdx, int32_t *level, float tfunc(uint32_t t, uint16_t p));
void FFBMngrCond(uint8_t effBlkIdx, uint8_t offset, int32_t pos, float *PID_mltipler);
//wave generation for Period para
float FFBTriangle(uint32_t t, uint16_t period);
float FFBSine(uint32_t t, uint16_t period);
float FFBSquare(uint32_t t, uint16_t period);
float FFBSawtoothUp(uint32_t t, uint16_t period);
float FFBSawtoothDown(uint32_t t, uint16_t period);
//effect calculation
void FFBMngrConstFoc(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty);
void FFBMngrRampFoc(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty);
void FFBMngrTrngl(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty);
void FFBMngrSine(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty);
void FFBMngrSprng(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty); //previous 5 type
void FFBMngrSquare(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty);
void FFBMngrSawtoothUp(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty);
void FFBMngrSawtoothDown(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty);
void FFBMngrDampr(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty);
void FFBMngrInertia(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty);
//void FFBMngrFriction(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty);

void FFBMngrInit(void)
{
    effRunning = 0;
    actStat = 1;
    effGlbGain = 0xff;
    mallocIdx = 0;
    pidLdStat = PID_Block_Load_Error;
    memset(effVis, 0, sizeof(effVis));
    memset(effStat, 0, sizeof(effStat));
    memset(effRunTime, 0, sizeof(effRunTime));
    memset(paraType, -1, sizeof(paraType));
}
uint8_t FFBMngrMalloc(uint8_t *data)
{
    //return a effect block index,
    //assume type==data[1]
    uint8_t effType = ((PID_Create_New_Effect_Report *) &data[1])\
                      ->PID_Effect_Type.pid_effect_type_enum_0; //more general way
    //uint8_t effType = data[1];
    uint8_t i, legal = 0;
    for(i = 0; i < sizeof(EffTypeArray); i++)
        if(effType == EffTypeArray[i]) {
            legal = 1;
            break;
        }
    if(!legal) { //type checking
        pidLdStat = PID_Block_Load_Error;
        return 0;
    }
    for(i = 1; i < maxEffN; i++)
        if(!effVis[i]) {
            pidLdStat = PID_Block_Load_Success;
            effVis[i] = 1; //set use flag and initialize
            return i;
        }
    pidLdStat = PID_Block_Load_Full;
    return 0;
}
void FFBMngrDelete(uint8_t effBlkIdx)
{
    effVis[effBlkIdx] = 0;
    effRunTime[effBlkIdx] = 0;
    effStat[effBlkIdx] = 0;
    paraType[effBlkIdx * 2] = 0xff;
    paraType[effBlkIdx * 2 + 1] = 0xff;
}
void FFBMngrEffAcpt(uint8_t *data)
{
    //data[0]==packetID
    uint8_t idx = ((const PID_Set_Effect_Report *) &data[1])->pid_effect_block_index;
    memcpy(&effRpt[idx], &data[1], sizeof(PID_Set_Effect_Report));
    effVis[idx] = 1; //set use flag and initialize
    effStat[idx] = 0; //loop 0 times
    effRunTime[idx] = 0;
}
void FFBMngrParaAcpt(uint8_t *data, uint16_t size)
{
    //data[0]==packetID, assume effIdx==data[1]
    uint8_t effBlkIdx = data[1];
    uint8_t type = data[0]; //packet ID
    //uint8_t offset = 0;
    uint8_t idx = effBlkIdx * 2 + 0; //assume offset == 0
    if(paraType[idx] != 0xFF && paraType[idx] != type) idx++; //offset == 1
    if(type == ID_PID_Set_Condition_Report) idx = effBlkIdx * 2 + data[2]; //data[2]=PID_Parameter_Block_Offset

    paraType[idx] = type;
    uint16_t minSize = maxParaBlkBytes; //size=min(RxSize, maxBlkSize)
    minSize = minSize > size ? size : minSize;
    memcpy(&paraBlkPool[idx], &data[1], minSize);
}
void FFBMngrOpre(uint8_t *data)
{
    //data[0]==packetID
    PID_Effect_Operation_Report *effOp = (PID_Effect_Operation_Report *) &data[1];
    uint8_t effBlkIdx = effOp->pid_effect_block_index;
    uint8_t operation = effOp->PID_Effect_Operation.pid_effect_operation_enum_0;
    uint8_t loop = effOp->pid_loop_count;
    if(operation == PID_Op_Effect_Stop) {
        effStat[effBlkIdx] = 0;
        return;
    }
    if(operation == PID_Op_Effect_Start_Solo)
        for(uint8_t i = 1; i < maxEffN; i++)
            effStat[i] = 0;
    if(operation == PID_Op_Effect_Start_Solo || operation == PID_Op_Effect_Start) {
        effStat[effBlkIdx] = loop;
        effRunning = 1;
    }
}
void FFBMngrCtrl(uint8_t *data)
{
    //assume ctrl==data[1] arrayN=1
    enum PID_PID_Device_Control_Enum ctrl = \
                                            ((PID_PID_Device_Control *) &data[1])->pid_pid_device_control_enum_0;
    switch(ctrl) {
    case PID_DC_Enable_Actuators:
        actStat = 1;
        break;
    case PID_DC_Disable_Actuators:
        actStat = 0;
        break;
    case PID_DC_Stop_All_Effects:
        for(uint8_t i = 1; i < maxEffN; i++)
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
void FFBMngrGain(uint8_t *data)
{
    //assume gain==data[1]
    uint8_t gain = ((PID_Device_Gain_Report *) &data[1])->pid_device_gain;
    effGlbGain = gain; //max 0xff
}
void FFBMngrBlkLd(uint8_t idx, uint16_t *len, uint8_t *inputReport)
{
    *len = sizeof(PID_PID_Block_Load_Report) + 1;
    inputReport[0] = ID_PID_PID_Block_Load_Report; //report id
    PID_PID_Block_Load_Report *rpt = (PID_PID_Block_Load_Report *) (inputReport + 1);
    rpt->pid_effect_block_index = idx;
    rpt->PID_Block_Load_Status.pid_block_load_status_enum_0 = pidLdStat;
    uint8_t cnt = 0;
    for(uint8_t i = 1; i < maxEffN; i++)
        if(!effVis[i]) cnt += 1;
    rpt->pid_ram_pool_available = cnt * sizeof(PID_Set_Effect_Report)\
                                  + cnt * 2 * maxParaBlkBytes;
}
void FFBMngrStat(uint16_t *len, uint8_t *inputReport)
{
    *len = sizeof(PID_PID_State_Report) + 1;
    inputReport[0] = ID_PID_PID_State_Report; //report id
    PID_PID_State_Report *rpt = (PID_PID_State_Report *) (inputReport + 1);
    rpt->pid_effect_block_index = mallocIdx; //????? effblkIdx
    rpt->vars_0 = 0;
    rpt->vars_0 |= effRunning ? 0 : Mask_PID_Device_Paused;
    rpt->vars_0 |= actStat ? Mask_PID_Actuators_Enabled : 0;
    rpt->vars_0 |= Mask_PID_Safety_Switch; //always on safety switch
    rpt->vars_0 |= Mask_PID_Actuator_Override_Switch; //always on override switch
    rpt->vars_0 |= Mask_PID_Actuator_Power;
    rpt->vars_0 = effStat[mallocIdx] ? Mask_PID_Effect_Playing : 0; //???which index should be reported?
}
void FFBMngrPoolReport(uint16_t *len, uint8_t *inputReport)
{
    *len = sizeof(PID_PID_Pool_Report) + 1;
    inputReport[0] = ID_PID_PID_Pool_Report;
    PID_PID_Pool_Report *rpt = (PID_PID_Pool_Report *) (inputReport + 1);
    rpt->pid_ram_pool_size = 0xffff;
    rpt->pid_simultaneous_effects_max = maxEffN - 1;
    rpt->vars_0 = Mask_PID_Device_Managed_Pool;
}
void FFBMngrDataOutServ(uint8_t *data, uint16_t size)
{
    //Output Pipe data FFB Service routine
    //assume reportID==data[0]
    uint8_t packetID = (uint8_t) data[0];
    //static uint8_t effType = 0;
    if(DEVICE_BRIDGE_ON) {
        BridgeOutServ(data, size);
    }
    switch(packetID) {
    case ID_PID_Set_Effect_Report:
        FFBMngrEffAcpt(data);
        break;
    case ID_PID_Set_Envelope_Report:
    case ID_PID_Set_Condition_Report:
    case ID_PID_Set_Periodic_Report:
    case ID_PID_Set_Constant_Force_Report:
    case ID_PID_Set_Ramp_Force_Report:
        FFBMngrParaAcpt(data, size);
        break;
    case ID_PID_Effect_Operation_Report:
        FFBMngrOpre(data);
        break;
    case ID_PID_PID_Device_Control: //???Microsoft SideWinder descriptor
        FFBMngrCtrl(data);
        break;
    case ID_PID_PID_Block_Free_Report:
        FFBMngrDelete(data[1]);
        break;
    case ID_PID_Device_Gain_Report:
        FFBMngrGain(data);
        break;
    default:
        USBD_ErrLog("Invalid Out packet type");
        break;
    };
}
void FFBMngrFeatureServ(uint8_t rptID, uint8_t dir, uint8_t *data)
{
    uint8_t packetID = (uint8_t) rptID;
    uint16_t len;
    if(dir != 0) {
        //Device-to-host dir==1,Get Report
        switch(packetID) {
        case ID_PID_PID_Block_Load_Report:
            FFBMngrBlkLd(mallocIdx, &len, HID_In_Report);
            USBD_PID_Send_EP0(HID_In_Report, len);
            break;
        case ID_PID_PID_Pool_Report:
            FFBMngrPoolReport(&len, HID_In_Report);
            USBD_PID_Send_EP0(HID_In_Report, len);
            break;
        default:
            break;
        }
    } else {
        //Host-to-Device dir==0,Set Report
        switch(packetID) {
        case ID_PID_Create_New_Effect_Report:
            mallocIdx = FFBMngrMalloc(data);
            break;
        default:
            break;
        }
    }
}
void FFBMngrDataInServ(uint8_t rptID)
{
    uint8_t packetID = (uint8_t) rptID;
    uint16_t len;
    switch(packetID) {
    case ID_PID_PID_State_Report:
        FFBMngrStat(&len, HID_In_Report);
        USBD_PID_Send(HID_In_Report, len);
        break;
    default:
        break;
    }
}
uint8_t FFBFindOffset(uint8_t idx, uint8_t type)
{
    if(paraType[idx * 2] == type)
        return 0;
    else if(paraType[idx * 2 + 1] == type)
        return 1;
    return 0xFF; //error return 0xFF
}
void FFBMngrEnvlp(uint8_t effBlkIdx, float *PID_mltipler)
{
    uint8_t offset = FFBFindOffset(effBlkIdx, ID_PID_Set_Envelope_Report); //find parameter offset

    PID_Set_Envelope_Report *pRpt = (PID_Set_Envelope_Report *) &paraBlkPool[effBlkIdx * 2 + offset];
    const PID_Set_Effect_Report *eRpt = &effRpt[effBlkIdx];
    uint32_t runtime, pid_duration;
    float lambda;
    const int16_t nrmlLevel = 10000;

    pid_duration = eRpt->pid_duration;
    runtime = effRunTime[effBlkIdx];
    lambda = 1.0; //envelope lambda
    *PID_mltipler = 1.0; //initialize PID_mltipler

    if(runtime > pid_duration - pRpt->pid_fade_time) {
        lambda = (pid_duration - runtime) / ((float) pRpt->pid_fade_time);
        *PID_mltipler = pRpt->pid_attack_level / ((float) nrmlLevel);
    }
    if(runtime < pRpt->pid_attack_time) {
        lambda = runtime / ((float) pRpt->pid_attack_time); //hope won't overflow
        *PID_mltipler = pRpt->pid_attack_level / ((float) nrmlLevel);
    }
    *PID_mltipler += (1 - *PID_mltipler) * lambda;
}
void FFBMngrPrid(uint8_t effBlkIdx, int32_t *level, float tfunc(uint32_t t, uint16_t p))
{
    uint8_t offset = FFBFindOffset(effBlkIdx, ID_PID_Set_Periodic_Report); //find parameter offset

    PID_Set_Periodic_Report *pRpt = (PID_Set_Periodic_Report *) &paraBlkPool[effBlkIdx * 2 + offset];
    pRpt->pid_period = pRpt->pid_period == 0 ? 1 : pRpt->pid_period; //avoid divided by 0
    uint32_t time = (uint32_t) (pRpt->pid_phase / 180.0) * pRpt->pid_period; //normalized value, max=180
    time = (time + effRunTime[effBlkIdx]) % pRpt->pid_period; //assume time unit: ms, remap t to [0,period]

    *level = (int16_t) pRpt->pid_offset;
    *level += (int32_t) (((int16_t) pRpt->pid_magnitude) * tfunc(time, pRpt->pid_period));
}
void FFBMngrCond(uint8_t effBlkIdx, uint8_t offset, int32_t pos, float *PID_mltipler)
{
    //pos: normalized
    PID_Set_Condition_Report *pRpt = (PID_Set_Condition_Report *) &paraBlkPool[effBlkIdx * 2 + offset];
    int16_t CP = pRpt->pid_cp_offset;
    int16_t dead = 0; //pRpt->pid_dead_band; current PID fedit has some bugs
    const float Normalized_SprngGain = 10000.0 / 1.5; //pos nml max=10000

    *PID_mltipler = 0;
    if(pos < CP - dead) { //???beantowel
        *PID_mltipler = pRpt->pid_negative_coefficient * (pos - (CP - dead))\
                        / ((float) pRpt->pid_negative_saturation) / Normalized_SprngGain;
    } else if(pos > CP + dead) {
        *PID_mltipler = pRpt->pid_positive_coefficient * (pos - (CP + dead))\
                        / ((float) pRpt->pid_positive_saturation) / Normalized_SprngGain;
    }
}
void FFBMngrConstFoc(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty)
{
    int32_t magnitude;
    float PID_mltipler;
    FFBMngrEnvlp(idx, &PID_mltipler);

    uint8_t offset = FFBFindOffset(idx, ID_PID_Set_Constant_Force_Report);
    PID_Set_Constant_Force_Report *con = (PID_Set_Constant_Force_Report *) paraBlkPool[idx * 2 + offset];
    magnitude = offset == 0xFF ? 10000 : con->pid_magnitude;
    *PID_Tx = *PID_Ty = (int32_t) (magnitude * PID_mltipler);
}
void FFBMngrRampFoc(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty)
{
    float PID_mltipler;
    FFBMngrEnvlp(idx, &PID_mltipler);

    uint8_t offset = FFBFindOffset(idx, ID_PID_Set_Ramp_Force_Report);
    PID_Set_Ramp_Force_Report *ramp = (PID_Set_Ramp_Force_Report *) paraBlkPool[idx * 2 + offset];
    // normalized start/end -10000--10000
    float lambda = effRunTime[idx] / (float) effRpt[idx].pid_duration;
    *PID_Tx = ramp->pid_ramp_start;
    *PID_Tx += (ramp->pid_ramp_end - ramp->pid_ramp_start) * lambda;
    *PID_Tx = *PID_Tx * PID_mltipler;
    *PID_Ty = *PID_Tx;
}
float FFBTriangle(uint32_t t, uint16_t period)
{
    float phase = t / (float) period;
    if(phase < 0.5) {
        phase = phase * 2;
    } else {
        phase = (1 - phase) * 2;
    }
    phase = phase * 2 - 1;
    return phase;
}
void FFBMngrTrngl(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty)
{
    float PID_mltipler = 1.0;
    int32_t level;
    FFBMngrEnvlp(idx, &PID_mltipler);
    FFBMngrPrid(idx, &level, FFBTriangle);
    *PID_Tx = *PID_Ty = (int32_t) (level * PID_mltipler);
}
float FFBSine(uint32_t t, uint16_t period)
{
    float phase = t / (float)period * 2 * PI;
    return sin(phase);
}
void FFBMngrSine(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty)
{
    float PID_mltipler = 1.0;
    int32_t level;
    FFBMngrEnvlp(idx, &PID_mltipler);
    FFBMngrPrid(idx, &level, FFBSine);
    *PID_Tx = *PID_Ty = (int32_t) (level * PID_mltipler);
}
void FFBMngrSprng(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty)
{
    int32_t x, y, maxp;
    float PID_mltipler;
    x = stick_Get_Position(0);
    y = stick_Get_Position(1);
    maxp = stick_Get_Position_Max();
    x = (int32_t) ( x * 10000 / maxp); //normalized max 10000
    y = (int32_t) ( y * 10000 / maxp);

    *PID_Tx = *PID_Ty = 0;
    //-x: some direction-related problems
    FFBMngrCond(idx, 0, -x, &PID_mltipler); //should check code to make sure x<=10000
    *PID_Tx = (int32_t) (10000 * PID_mltipler); //normalized max 10000

    FFBMngrCond(idx, 1, y, &PID_mltipler);
    *PID_Ty = (int32_t) (10000 * PID_mltipler);
}
//previous 5 types
float FFBSquare(uint32_t t, uint16_t period)
{
    float phase = t / (float) period;
    if(phase < (float) period / 2)
        return 1.0;
    else
        return 0;
}
void FFBMngrSquare(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty)
{
    float PID_mltipler = 1.0;
    int32_t level;
    FFBMngrEnvlp(idx, &PID_mltipler);
    FFBMngrPrid(idx, &level, FFBSquare);
    *PID_Tx = *PID_Ty = (int32_t) (level * PID_mltipler);
}
float FFBSawtoothUp(uint32_t t, uint16_t period)
{
    float phase = t / (float) period;
    return phase / (float) period;
}
void FFBMngrSawtoothUp(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty)
{
    float PID_mltipler = 1.0;
    int32_t level;
    FFBMngrEnvlp(idx, &PID_mltipler);
    FFBMngrPrid(idx, &level, FFBSawtoothUp);
    *PID_Tx = *PID_Ty = (int32_t) (level * PID_mltipler);
}
float FFBSawtoothDown(uint32_t t, uint16_t period)
{
    float phase = t / (float) period;
    return 1.0 - phase / (float) period;
}
void FFBMngrSawtoothDown(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty)
{
    float PID_mltipler = 1.0;
    int32_t level;
    FFBMngrEnvlp(idx, &PID_mltipler);
    FFBMngrPrid(idx, &level, FFBSawtoothDown);
    *PID_Tx = *PID_Ty = (int32_t) (level * PID_mltipler);
}
void FFBMngrDamper(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty)
{
    int32_t x, y, maxp;
    float PID_mltipler;
    x = stick_Get_Velocity(0);
    y = stick_Get_Velocity(1);
    maxp = stick_Get_Velocity_Max();
    x = (int32_t) ( x * 10000 / maxp); //normalized max 10000
    y = (int32_t) ( y * 10000 / maxp);

    *PID_Tx = *PID_Ty = 0;
    //-x: some direction-related problems
    FFBMngrCond(idx, 0, -x, &PID_mltipler); //should check code to make sure x<=10000
    *PID_Tx = (int32_t) (10000 * PID_mltipler); //normalized max 10000

    FFBMngrCond(idx, 1, y, &PID_mltipler);
    *PID_Ty = (int32_t) (10000 * PID_mltipler);
}
void FFBMngrInertia(uint8_t idx, int32_t *PID_Tx, int32_t *PID_Ty)
{
    int32_t x, y, maxp;
    float PID_mltipler;
    x = stick_Get_Acceleration(0);
    y = stick_Get_Acceleration(1);
    maxp = stick_Get_Acceleration_Max();
    x = (int32_t) ( x * 10000 / maxp); //normalized max 10000
    y = (int32_t) ( y * 10000 / maxp);

    *PID_Tx = *PID_Ty = 0;
    //-x: some direction-related problems
    FFBMngrCond(idx, 0, -x, &PID_mltipler); //should check code to make sure x<=10000
    *PID_Tx = (int32_t) (10000 * PID_mltipler); //normalized max 10000

    FFBMngrCond(idx, 1, y, &PID_mltipler);
    *PID_Ty = (int32_t) (10000 * PID_mltipler);
}
void FFBMngrEffRun(uint16_t deltaT, int32_t *Tx, int32_t *Ty)
{
    //given deltaTime(ms),return Torque on x&y axes
    *Tx = 0;
    *Ty = 0;
    if(DEVICE_BRIDGE_ON) {
        BridgeGetXY(Tx, Ty);
    }
    for(uint8_t i = 1; i < maxEffN; i++) {
        const PID_Set_Effect_Report *eRpt = &effRpt[i];
        if(effVis[i] && effStat[i] > 0 && effRunning) {
            effRunTime[i] += deltaT; //update runtime
            uint16_t pid_duration = eRpt->pid_duration;
            if (effRunTime[i] > pid_duration && pid_duration != 0xFFFF) {
                effStat[i]--;
                effRunTime[i] = 0;
                continue;
            }
            //Trigger Button Judge/Repeat Interval(to be done)

            float dirX = 1.0, dirY = 1.0; //normalized value max 0x7f?
            uint8_t gain = eRpt->pid_gain;
            const uint8_t maxGain = 0xFF;
            uint8_t direction = eRpt->PID_Direction.instance_1;
            dirX = cos((float) (-direction * 2 * PI / 180.0 + PI / 2));
            dirY = sin((float) (-direction * 2 * PI / 180.0 + PI / 2));
            if(eRpt->PID_Effect_Type.pid_effect_type_enum_0 == PID_ET_Spring) {
                dirX = 1.0;
                dirY = 1.0;
            }
            int32_t PID_Tx = 0, PID_Ty = 0;
            switch(eRpt->PID_Effect_Type.pid_effect_type_enum_0) {
            case PID_ET_Constant_Force:
                FFBMngrConstFoc(i, &PID_Tx, &PID_Ty);
                break;
            case PID_ET_Ramp:
                FFBMngrRampFoc(i, &PID_Tx, &PID_Ty);
                break;
            case PID_ET_Triangle:
                FFBMngrTrngl(i, &PID_Tx, &PID_Ty);
                break;
            case PID_ET_Spring:
                FFBMngrSprng(i, &PID_Tx, &PID_Ty);
                break;
            case PID_ET_Sine:
                FFBMngrSine(i, &PID_Tx, &PID_Ty);
                break;
            default:
                break;
            }
            *Tx += (int32_t) (PID_Tx * dirX * gain * effGlbGain / maxGain / maxGain);
            *Ty += (int32_t) (PID_Ty * dirY * gain * effGlbGain / maxGain / maxGain);
            //max 10000
        }
    }
}
