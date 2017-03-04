#pragma once
//Defines from Device Definition for Human Interface Devices (HID)

#define u_Byte_Size(x)		1
#define Byte_Size(x)		1
//6.2.2.4 Main Items
#define Input(x)			(0x80+u_Byte_Size(x)),(x)
#define Output(x)			(0x90+u_Byte_Size(x)),(x)
#define Feature(x)			(0xB0+u_Byte_Size(x)),(x)
#define Collection(x)		(0xA0+u_Byte_Size(x)),(x)
#define End_Collection		0xC0

//6.2.2.5 Input Output Feature Items
#define IOF_Defalut_Items	0x0
/*Data,Array,Absolute,No Wrap,
Linear,Preferred State,No Null Position
Non Volatile,Bit Field*/
#define IOF_Constant		0x1
#define IOF_Variable		0x2
#define IOF_Array			IOF_Constant

//6.2.2.6 Collection Items
#define Clc_Physical		0x00
#define Clc_Application		0x01
#define Clc_Logical			0x02
#define Clc_Report			0x03
#define Clc_Named_Array		0x04
#define Clc_Usage_Switch	0x05

//6.2.2.7 Global Items 
//example: (uint8_t) Usage_Page(Generic_Desktop) ==> 0x05,0x01
#define Usage_Page(x)		(0x04+u_Byte_Size(x)),(x)
#define Logical_Minimum(x)	(0x14+Byte_Size(x)),(x)
#define Logical_Maximum(x)	(0x24+Byte_Size(x)),(x)
#define Physical_Minimum(x)	(0x34+Byte_Size(x)),(x)
#define Physical_Maximum(x)	(0x44+Byte_Size(x)),(x)
#define	Unit_Exponent(x)	(0x54+u_Byte_Size(x)),(x)
#define	Unit(x)				(0x64+u_Byte_Size(x)),(x)
#define Report_Size(x)		(0x74+u_Byte_Size(x)),(x)
#define Report_ID(x)		(0x84+u_Byte_Size(x)),(x)
#define Report_Count(x)		(0x94+u_Byte_Size(x)),(x)

#define Unit_None			0x00

#define Eng_Lin_Time		0x1003
#define Eng_Rot_Angular_Pos	0x14
//6.2.2.8 Local Items
#define Usage(x)			(0x08+u_Byte_Size(x)),(x)
#define Usage_Minimum(x)	(0x18+u_Byte_Size(x)),(x)
#define Usage_Maximum(x)	(0x28+u_Byte_Size(x)),(x)

//Usage Pages ID
#define Generic_Desktop_ID	0x01
#define Simulation_ID		0x02
#define Button_ID			0x09
#define Physical_Interface	0x0F

//Generic Desktop Page
#define Pointer_ID			0x01
#define Joystick_ID			0x04
#define Game_Pad_ID			0x05
#define X_ID				0x30
#define Y_ID				0x31
#define Z_ID				0x32
#define Rx_ID				0x33
#define Ry_ID				0x34
#define Rz_ID				0x35
#define Byte_Count			0x3B

//Simulation Control Page
#define Throttle_ID			0xBB

//Button Usage Page
#define No_botton_ID		0x00
#define Button1_ID			0x01
#define Button4_ID			0x04