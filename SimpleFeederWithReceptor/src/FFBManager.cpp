#include "stdafx.h"
#include "stdint.h"
#include "public.h"
#include "vjoyinterface.h"

//PID Structures Define from <Device Class Definition for PID Devices> page44 Figure19:
#ifndef _PUBLIC_H
// HID Descriptor definitions - FFB Effects
#define HID_USAGE_CONST 0x26    //    Usage ET Constant Force
#define HID_USAGE_RAMP  0x27    //    Usage ET Ramp
#define HID_USAGE_SQUR  0x30    //    Usage ET Square
#define HID_USAGE_SINE  0x31    //    Usage ET Sine
#define HID_USAGE_TRNG  0x32    //    Usage ET Triangle
#define HID_USAGE_STUP  0x33    //    Usage ET Sawtooth Up
#define HID_USAGE_STDN  0x34    //    Usage ET Sawtooth Down
#define HID_USAGE_SPRNG 0x40    //    Usage ET Spring
#define HID_USAGE_DMPR  0x41    //    Usage ET Damper
#define HID_USAGE_INRT  0x42    //    Usage ET Inertia
#define HID_USAGE_FRIC  0x43    //    Usage ET Friction


// HID Descriptor definitions - FFB Report IDs
#define HID_ID_STATE	0x02	// Usage PID State report
#define HID_ID_EFFREP	0x01	// Usage Set Effect Report
#define HID_ID_ENVREP	0x02	// Usage Set Envelope Report
#define HID_ID_CONDREP	0x03	// Usage Set Condition Report
#define HID_ID_PRIDREP	0x04	// Usage Set Periodic Report
#define HID_ID_CONSTREP	0x05	// Usage Set Constant Force Report
#define HID_ID_RAMPREP	0x06	// Usage Set Ramp Force Report
#define HID_ID_CSTMREP	0x07	// Usage Custom Force Data Report
#define HID_ID_SMPLREP	0x08	// Usage Download Force Sample
#define HID_ID_EFOPREP	0x0A	// Usage Effect Operation Report
#define HID_ID_BLKFRREP	0x0B	// Usage PID Block Free Report
#define HID_ID_CTRLREP	0x0C	// Usage PID Device Control
#define HID_ID_GAINREP	0x0D	// Usage Device Gain Report
#define HID_ID_SETCREP	0x0E	// Usage Set Custom Force Report
#define HID_ID_NEWEFREP	0x01	// Usage Create New Effect Report
#define HID_ID_BLKLDREP	0x02	// Usage Block Load Report
#define HID_ID_POOLREP	0x03	// Usage PID Pool Report
#endif // !_PUBLIC_H

#ifndef VJOYINTERFACE_API
//FFB Defines From VjoyInterface
//Some of the defines are not from the <Device Class Definition for PID Devices>
enum FFBEType {
	// FFB Effect Type,Order Defined in Report Descriptor
	// Refer to: Effect Type Item in Create New Effect Report
    ET_NONE  = 0,	  //    No Force
    ET_CONST = 1,    //    Constant Force
    ET_RAMP  = 2,    //    Ramp
    ET_SQR   = 3,    //    Square
    ET_SINE  = 4,    //    Sine
    ET_TRNGL = 5,    //    Triangle
    ET_STUP  = 6,    //    Sawtooth Up
    ET_STDN  = 7,    //    Sawtooth Down
    ET_SPRNG = 8,    //    Spring
    ET_DMPR  = 9,    //    Damper
    ET_INRT  = 10,   //    Inertia
    ET_FRCTN = 11,   //    Friction
    ET_CSTM  = 12,   //    Custom Force Data
};

enum FFBPType {
    // FFB Packet Type,Order Defined in Report Descriptor
    // Refer to: Each Report ID(x) Item
    PT_EFFREP   = HID_ID_EFFREP,	// Usage Set Effect Report
    PT_ENVREP   = HID_ID_ENVREP,	// Usage Set Envelope Report
    PT_CONDREP  = HID_ID_CONDREP,	// Usage Set Condition Report
    PT_PRIDREP  = HID_ID_PRIDREP,	// Usage Set Periodic Report
    PT_CONSTREP = HID_ID_CONSTREP,	// Usage Set Constant Force Report
    PT_RAMPREP  = HID_ID_RAMPREP,	// Usage Set Ramp Force Report
    PT_CSTMREP  = HID_ID_CSTMREP,	// Usage Custom Force Data Report
    PT_SMPLREP  = HID_ID_SMPLREP,	// Usage Download Force Sample
    PT_EFOPREP  = HID_ID_EFOPREP,	// Usage Effect Operation Report
    PT_BLKFRREP = HID_ID_BLKFRREP,	// Usage PID Block Free Report
    PT_CTRLREP  = HID_ID_CTRLREP,	// Usage PID Device Control
    PT_GAINREP  = HID_ID_GAINREP,	// Usage Device Gain Report
    PT_SETCREP  = HID_ID_SETCREP,	// Usage Set Custom Force Report

    // Feature
    PT_NEWEFREP = HID_ID_NEWEFREP + 0x10,	// Usage Create New Effect Report
    PT_BLKLDREP = HID_ID_BLKLDREP + 0x10,	// Usage Block Load Report
    PT_POOLREP  = HID_ID_POOLREP + 0x10,	// Usage PID Pool Report
};

enum FFBOP {
    EFF_START = 1, // EFFECT START
    EFF_SOLO  = 2, // EFFECT SOLO START
    EFF_STOP  = 3, // EFFECT STOP
};

enum FFB_CTRL {
    CTRL_ENACT    = 1,	// Enable all device actuators.
    CTRL_DISACT   = 2,	// Disable all the device actuators.
    CTRL_STOPALL  = 3,	// Stop All Effects?Issues a stop on every running effect.
    CTRL_DEVRST   = 4,	// Device Reset?Clears any device paused condition, enables all actuators and clears all effects from memory.
    CTRL_DEVPAUSE = 5,	// Device Pause?The all effects on the device are paused at the current time step.
    CTRL_DEVCONT  = 6,	// Device Continue?The all effects that running when the device was paused are restarted from their last time step.
};

enum FFB_EFFECTS {
    Constant    = 0x0001,
    Ramp        = 0x0002,
    Square      = 0x0004,
    Sine        = 0x0008,
    Triangle    = 0x0010,
    Sawtooth_Up = 0x0020,
    Sawtooth_Dn = 0x0040,
    Spring      = 0x0080,
    Damper      = 0x0100,
    Inertia     = 0x0200,
    Friction    = 0x0400,
    Custom      = 0x0800,
};

typedef struct _FFB_DATA {
    ULONG	size;
    ULONG	cmd; //what do these two items uesd for?and where do they come from?;BeanTowel comments
    UCHAR	*data;
} FFB_DATA, *PFFB_DATA;
//(data structure)
typedef struct _FFB_EFF_CONSTANT {
    BYTE EffectBlockIndex;
    LONG Magnitude; 			  // Constant force magnitude: 	-10000 - 10000
} FFB_EFF_CONSTANT, *PFFB_EFF_CONSTANT;

typedef struct _FFB_EFF_RAMP {
    BYTE		EffectBlockIndex;
    LONG 		Start;             // The Normalized magnitude at the start of the effect (-10000 - 10000)
    LONG 		End;               // The Normalized magnitude at the end of the effect	(-10000 - 10000)
} FFB_EFF_RAMP, *PFFB_EFF_RAMP;

//typedef struct _FFB_EFF_CONST {
typedef struct _FFB_EFF_REPORT {
    BYTE		EffectBlockIndex;
    FFBEType	EffectType;
    WORD		Duration;// Value in milliseconds. 0xFFFF means infinite
    WORD		TrigerRpt;
    WORD		SamplePrd;
    BYTE		Gain;
    BYTE		TrigerBtn;
    BOOL		Polar; // How to interpret force direction Polar (0-360? or Cartesian (X,Y)
    union {
        BYTE	Direction; // Polar direction: (0x00-0xFF correspond to 0-360?
        BYTE	DirX; // X direction: Positive values are To the right of the center (X); Negative are Two's complement
    };
    BYTE		DirY; // Y direction: Positive values are below the center (Y); Negative are Two's complement
} FFB_EFF_REPORT, *PFFB_EFF_REPORT;
//} FFB_EFF_CONST, *PFFB_EFF_CONST;

typedef struct _FFB_EFF_OP { //Effect Operation Report
    BYTE		EffectBlockIndex;
    FFBOP		EffectOp;
    BYTE		LoopCount;
} FFB_EFF_OP, *PFFB_EFF_OP;

typedef struct _FFB_EFF_PERIOD {
    BYTE		EffectBlockIndex;
    DWORD		Magnitude;			// Range: 0 - 10000
    LONG 		Offset;				// Range: ?0000 - 10000
    DWORD 		Phase;				// Range: 0 - 35999
    DWORD 		Period;				// Range: 0 - 32767
} FFB_EFF_PERIOD, *PFFB_EFF_PERIOD;

typedef struct _FFB_EFF_COND {
    BYTE		EffectBlockIndex;
    BOOL		isY;
    LONG 		CenterPointOffset; // CP Offset:  Range -?0000 ? 10000
    LONG 		PosCoeff; // Positive Coefficient: Range -?0000 ? 10000
    LONG 		NegCoeff; // Negative Coefficient: Range -?0000 ? 10000
    DWORD 		PosSatur; // Positive Saturation: Range 0 ?10000
    DWORD 		NegSatur; // Negative Saturation: Range 0 ?10000
    LONG 		DeadBand; // Dead Band: : Range 0 ?1000
} FFB_EFF_COND, *PFFB_EFF_COND;

typedef struct _FFB_EFF_ENVLP {
    BYTE		EffectBlockIndex;
    DWORD 		AttackLevel;   // The Normalized magnitude of the stating point: 0 - 10000
    DWORD 		FadeLevel;	   // The Normalized magnitude of the stopping point: 0 - 10000
    DWORD 		AttackTime;	   // Time of the attack: 0 - 4294967295
    DWORD 		FadeTime;	   // Time of the fading: 0 - 4294967295
} FFB_EFF_ENVLP, *PFFB_EFF_ENVLP;

#endif // !VJOYINTERFACE_API

const uint16_t maxEffN = 10, maxTypeN = 4, maxParaBlkBytes=10; 
const uint8_t RptIdArrayN = 13, EffTypeArrayN = 5;
// const uint8_t ReportIDArray[RptIdArrayN] = {
//     //HID_ID_STATE+0x10,
//     HID_ID_EFFREP + 0x10,
//     HID_ID_ENVREP + 0x10,
//     HID_ID_CONDREP + 0x10,
//     HID_ID_PRIDREP + 0x10,
//     HID_ID_CONSTREP + 0x10,
//     HID_ID_RAMPREP + 0x10,
//     //HID_ID_CSTMREP+0x10,
//     //HID_ID_SMPLREP+0x10,
//     HID_ID_EFOPREP + 0x10,
//     HID_ID_BLKFRREP + 0x10,
//     HID_ID_CTRLREP + 0x10,
//     HID_ID_GAINREP + 0x10,
//     //HID_ID_SETCREP+0x10,
//     HID_ID_NEWEFREP + 0x10,
//     HID_ID_BLKLDREP + 0x10,
//     HID_ID_POOLREP + 0x10,
// }; //i don't know why but 0x10 have to be added
const uint8_t EffTypeArray[EffTypeArrayN] = {
	ET_NONE,
	ET_CONST,
	ET_RAMP,
	//ET_SQR,
	//ET_SINE,
	ET_TRNGL,
	//ET_STUP,
	//ET_STDN,
	ET_SPRNG,
	//ET_DMPR,
	//ET_INRT,
	//ET_FRCTN,
	//ET_CSTM,
}
uint8_t effVis[maxEffN]; // 0:free 1:occupied
uint8_t effStat[maxEffN]; // 0:off 1:looping count
uint8_t effRunning,actStat; //effect running flag, actuator enable flag
FFB_EFF_REPORT effRpt[maxEffN]; //effect report pool
uint8_t paraBlkPool[maxEffN*2][maxParaBlkBytes]; //parameter block pool

void FFBMngrInit() {
	effRunning=1;
	actStat=1;
	memset(effVis,0,sizeof(effVis));
	memset(effStat,0,sizeof(effStat));
}
uint8_t FFBMngrMalloc(uint8_t* data){
	//return a effect block index,
	//if not avaliable or can't perform the effect,return -1
	//assume id==data[0]
	uint8_t effType=data[0];
	uint8_t i;
	for(i=0;i<EffTypeArrayN;i++)
		if(effType == EffTypeArray[i]) break;
	if(effType != EffTypeArray[i]) return -1; //type checking
	for(i=0;i<maxEffN;i++)
		if(!effVis[i]) return i;
	return -1;
}
void FFBMngrDelete(uint8_t effBlkIdx){
	effVis[effBlkIdx]=0;
}
void FFBMngrEffAcpt(uint8_t *data){
	uint8_t effBlkIdx=((FFB_EFF_REPORT*) data)->EffectBlockIndex;
	effRpt[effBlkIdx]=*((FFB_EFF_REPORT *) data);
	effVis[effBlkIdx]=1;
}
void FFBMngrParaAcpt(uint8_t *data,uint8_t size){
	//assume id==data[0] offset==data[1]
	uint8_t effBlkIdx=data[0],offset=data[1];
	uint8_t idx=effBlkIdx*2+offset;
	for(uint8_t i=0;i<size;i++)
		paraBlkPool[idx][i]=data[i];	
}
void FFBMngrOpre(uint8_t *data){	
	FFB_EFF_OP *effOp=(FFB_EFF_OP *) data;
	uint8_t effBlkIdx=effOp->EffectBlockIndex;
	uint8_t operation=effOp->EffectOp;
	uint8_t loop=effOp->LoopCount;
	if(operation==EFF_STOP){
		effStat[effBlkIdx]=0;
		return;
	}
	if(operation==EFF_SOLO)
		for(uint8_t i=0;i<maxEffN;i++)
			effStat[i]=0;
	if(operation==EFF_SOLO || operation==EFF_START)
		effStat[effBlkIdx]=loop;
}
void FFBMngrCtrl(uint8_t *data){
	FFB_CTRL ctrl=data[1];
	uint8_t 
	switch()
}
void FFBMngrOutServ(uint8_t *data,uint8_t size){
	//Output Pipe data FFB Service routine
	uint8_t packetID=data[0];
	uint8_t effBlkIdx=-1,effType=0;
	switch(packetID-0x10){
		// 0x10 it's magic,got it from experiment statistics
		case HID_ID_NEWEFREP:
			effBlkIdx=FFBMngrMalloc(data);
			break;
		case HID_ID_EFFREP:
			FFBMngrEffAcpt(data);
			break;
		case HID_ID_ENVREP:
		case HID_ID_CONDREP:
		case HID_ID_PRIDREP:
		case HID_ID_CONSTREP:
		case HID_ID_RAMPREP:
			FFBMngrParaAcpt(data,size);
			break;
		case HID_ID_EFOPREP:
			FFBMngrOpre(data);
			break;
		case HID_ID_CTRLREP:
			FFBMngrCtrl(data);
		case 
	}
}