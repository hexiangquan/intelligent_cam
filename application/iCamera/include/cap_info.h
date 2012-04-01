#ifndef __CAP_INFO__
#define __CAP_INFO__

#include "cam_params.h"

/* Frame Num */
enum FrameID
{
	FRAME_CONTINUE,				//continous capture
	FRAME_MANUALTRIG,			//munual trigger
	FRAME_EPOLICE_CP = 0,		//epolice & check post, green light
	FRAME_EPOLICE_1ST,			//epolice, fist frame
	FRAME_EPOLICE_2ND,			//epolice, second frame
	FRAME_EPOLICE_3RD,			//epolice, third frame
	FRAME_EPOLICE_4TH,			//epolice, 4th frame
	FRAME_CHECKPOST_1ST,		//checkpost, first frame
	FRAME_CHECKPOST_2ND,		//checkpost, second frame
	FRAME_RETROGRADE_1ST,		//retrograde 1st frame
	FRAME_RETROGRADE_2ND,		//retrograde 2nd frame
	FRAME_RETROGRADE_3RD,		//retrograde 3rd frame
	FRAME_LIGHTDETECT = 128,	//auto brightness detect
	FRAME_VIDEOTRIGGER,			//video trigger
	FRAME_MAX					//unused
};

/* captrue and trigger info generated at run time */
#define TRIG_INFO_OVERSPEED					(1 << 0)
#define TRIG_INFO_SEND_UTIL_NEXT_CAP		(1 << 1)
#define TRIG_INFO_RED_LIGHT		            (1 << 2)
#define TRIG_INFO_RETROGRADE				(1 << 3)
#define TRIG_INFO_DELAY_CAP					(1 << 4)

/* single way trigger info */
typedef struct {
	Uint8			frameId; 			//Frame ID for Epolice 1-3, Checkpost 1-2, Retrograde, etc.
	Uint8			wayNum;				//Way number for trigger, start from 1
	Uint16			redlightTime; 		//Redlight time, unit: 10Ms
	Uint16			groupId; 			//Group Id
	Uint16			flags;				//Flag for extra info, i.e. overspeed
	Uint8			speed;				//Vehicle speed
	Int8			reserved[3];		//Reversed
}TriggerInfo;

/* Flags for capture info */
#define CAPINFO_FLAG_STROBE_FORCE		(1 << 0)
#define CAPINFO_FLAG_STROBE1_EN			(1 << 1)
#define CAPINFO_FLAG_STROBE2_EN			(1 << 2)
#define CAPINFO_FLAG_DELAY_CAP			(1 << 3)
#define CAPINFO_FLAG_OVERSPEED			(1 << 4)

/* capture info for one frame */
typedef struct {
	Uint32			capCnt;							//num of ways captured
	TriggerInfo		triggerInfo[APP_MAX_CAP_CNT];	//trigger info
	Uint16			flags;							//other Capture info, i.e. strobe 
	Uint8			limitSpeed;						//Limit speed
	Uint8			reserved[5];					//Reserved
}CaptureInfo;

#endif

