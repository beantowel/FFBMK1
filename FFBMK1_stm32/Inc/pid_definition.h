#ifndef __USB_PID_Def
#define __USB_PID_Def

#ifdef __cplusplus
 extern "C" {
#endif

#include "stdint.h"

enum PID_Effect_Type_Enum {
    PID_ET_Constant_Force = 1,
    PID_ET_Sine = 2,
    PID_ET_Triangle = 3,
    PID_ET_Spring = 4,
    PID_ET_Ramp = 5,
};

enum PID_Block_Load_Status_Enum {
    PID_Block_Load_Success = 1,
    PID_Block_Load_Full = 2,
    PID_Block_Load_Error = 3,
};

enum PID_Effect_Operation_Enum {
    PID_Op_Effect_Start = 1,
    PID_Op_Effect_Start_Solo = 2,
    PID_Op_Effect_Stop = 3,
};

enum PID_PID_Device_Control_Enum {
    PID_DC_Enable_Actuators = 1,
    PID_DC_Disable_Actuators = 2,
    PID_DC_Stop_All_Effects = 3,
    PID_DC_Device_Reset = 4,
    PID_DC_Device_Pause = 5,
    PID_DC_Device_Continue = 6,
};

enum Report_ID_Enum {
    ID_PID_Create_New_Effect_Report = 1,
    ID_PID_PID_Block_Load_Report = 2,
    ID_PID_Set_Effect_Report = 3,
    ID_PID_Set_Envelope_Report = 4,
    ID_PID_Set_Condition_Report = 5,
    ID_PID_Set_Periodic_Report = 6,
    ID_PID_Set_Constant_Force_Report = 7,
    ID_PID_Set_Ramp_Force_Report = 8,
    ID_PID_Effect_Operation_Report = 9,
    ID_PID_PID_State_Report = 10,
    ID_PID_PID_Device_Control_Report = 11,
    ID_PID_Device_Gain_Report = 12,
    ID_PID_PID_Block_Free_Report = 13, 
};

const uint8_t Mask_X_ID = 0xf;
const uint8_t Mask_Y_ID = 0xf0;
const uint8_t Mask_PID_Effect_Playing = 0x1;
const uint8_t Mask_PID_Actuators_Enabled = 0x2;
const uint8_t Mask_PID_Safety_Switch = 0x4;
const uint8_t Mask_PID_Actuator_Power = 0x8;

typedef struct _PID_Create_New_Effect_Report {
    //Report_ID:1
    struct {
        enum PID_Effect_Type_Enum pid_effect_type_enum_0;
        //Logical_Maximum:5
        //Logical_Minimum:1
    } PID_Effect_Type;
    uint8_t byte_count;
    //Logical_Maximum:255
} PID_Create_New_Effect_Report;

typedef struct _PID_PID_Block_Load_Report {
    //Report_ID:2
    uint8_t pid_effect_block_index;
    //Logical_Maximum:255
    struct {
        enum PID_Block_Load_Status_Enum pid_block_load_status_enum_0;
        //Logical_Maximum:3
        //Logical_Minimum:1
    } PID_Block_Load_Status;
    uint16_t pid_ram_pool_available;
    //Logical_Maximum:65535
} PID_PID_Block_Load_Report;

typedef struct _PID_Set_Effect_Report {
    //Report_ID:3
    uint8_t pid_effect_block_index;
    //Logical_Maximum:255
    struct {
        enum PID_Effect_Type_Enum pid_effect_type_enum_0;
        //Logical_Maximum:4
        //Logical_Minimum:1
    } PID_Effect_Type;
    uint16_t pid_duration;
    uint16_t pid_trigger_repeat_interval;
    //Logical_Maximum:10000
    //Unit:Eng_Lin_Time
    //Unit_Exponent:-3
    uint16_t pid_sample_period;
    //Logical_Maximum:10000
    //Unit:Eng_Lin_Time
    //Unit_Exponent:-6
    uint8_t pid_gain;
    uint8_t pid_trigger_button;
    //Logical_Maximum:127
    struct {
        struct {
            uint8_t vars_0;
            //x_id,y_id,
            //Check Pads
            //Logical_Maximum:1
        } Pointer_ID;
    } PID_Axes_Enable;
    uint8_t pid_direction_enable;
    //Logical_Maximum:1
    struct {
        struct {
            uint8_t x_id;
            uint8_t y_id;
            //Logical_Maximum:255
            //Unit:Eng_Rot_Angular_Pos
        } Pointer_ID;
    } PID_Direction;
} PID_Set_Effect_Report;

typedef struct _PID_Set_Envelope_Report {
    //Report_ID:4
    uint8_t pid_effect_block_index;
    //Logical_Maximum:255
    uint16_t pid_attack_level;
    uint16_t pid_fade_level;
    //Logical_Maximum:10000
    uint16_t pid_attack_time;
    uint16_t pid_fade_time;
    //Logical_Maximum:10000
    //Unit:Eng_Lin_Time
    //Unit_Exponent:-3
} PID_Set_Envelope_Report;

typedef struct _PID_Set_Condition_Report {
    //Report_ID:5
    uint8_t pid_effect_block_index;
    //Logical_Maximum:255
    uint8_t pid_parameter_block_offset;
    //Logical_Maximum:1
    int16_t pid_cp_offset;
    int16_t pid_positive_coefficient;
    int16_t pid_negative_coefficient;
    int16_t pid_positive_saturation;
    int16_t pid_negative_saturation;
    int16_t pid_dead_band;
    //Logical_Maximum:10000
    //Logical_Minimum:-10000
} PID_Set_Condition_Report;

typedef struct _PID_Set_Periodic_Report {
    //Report_ID:6
    uint8_t pid_effect_block_index;
    //Logical_Maximum:255
    int16_t pid_magnitude;
    int16_t pid_offset;
    int16_t pid_phase;
    //Logical_Maximum:10000
    //Logical_Minimum:-10000
    uint16_t pid_period;
    //Logical_Maximum:10000
    //Unit:Eng_Lin_Time
    //Unit_Exponent:-3
} PID_Set_Periodic_Report;

typedef struct _PID_Set_Constant_Force_Report {
    //Report_ID:7
    uint8_t pid_effect_block_index;
    //Logical_Maximum:255
    uint16_t pid_magnitude;
    //Logical_Maximum:10000
} PID_Set_Constant_Force_Report;

typedef struct _PID_Set_Ramp_Force_Report {
    //Report_ID:8
    uint8_t pid_effect_block_index;
    //Logical_Maximum:255
    int16_t pid_ramp_start;
    int16_t pid_ramp_end;
    //Logical_Maximum:10000
    //Logical_Minimum:-10000
} PID_Set_Ramp_Force_Report;

typedef struct _PID_Effect_Operation_Report {
    //Report_ID:9
    uint8_t pid_effect_block_index;
    //Logical_Maximum:255
    struct {
        enum PID_Effect_Operation_Enum pid_effect_operation_enum_0;
        //Logical_Maximum:3
        //Logical_Minimum:1
    } PID_Effect_Operation;
    uint8_t pid_loop_count;
    //Logical_Maximum:255
} PID_Effect_Operation_Report;

typedef struct _PID_PID_State_Report {
    //Report_ID:10
    uint8_t pid_effect_block_index;
    //Logical_Maximum:255
    uint8_t vars_0;
    //pid_effect_playing,pid_actuators_enabled,pid_safety_switch,pid_actuator_power,
    //Check Pads
    //Logical_Maximum:4
    //Logical_Minimum:1
    //4pads added
    //Logical_Maximum:4
    //Logical_Minimum:1
} PID_PID_State_Report;

typedef struct _PID_PID_Device_Control_Report {
    //Report_ID:11
    struct {
        enum PID_PID_Device_Control_Enum pid_pid_device_control_enum_0;
        //Logical_Maximum:6
        //Logical_Minimum:1
    } PID_PID_Device_Control;
} PID_PID_Device_Control_Report;

typedef struct _PID_Device_Gain_Report {
    //Report_ID:12
    uint8_t pid_device_gain;
    //Logical_Maximum:255
} PID_Device_Gain_Report;

typedef struct _PID_PID_Block_Free_Report {
    //Report_ID:13
    uint8_t pid_effect_block_index;
    //Logical_Maximum:255
} PID_PID_Block_Free_Report;


#ifdef __cplusplus
}
#endif

#endif /* __USB_PID_Def */
