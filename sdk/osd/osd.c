#include "common.h"
#include "osd.h"
#include "log.h"
#include "_osd.h"
#include <iconv.h> 

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define HZK16_SIZE 		267616
#define ASC16_SIZE 		1536
#define OSD_POS_ALIGN	2
#define OSD_MIN_VAL		32
#define OSD_ASCII_MAX	127
#define OSD_HZ_MIN		160
#define OSD_BUF_SIZE	256

//#define OSD_CODE_CONVERT	

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/
typedef struct _OsdObj *OsdAlgHandle;

typedef struct _OsdObj {
	const Uint8 	*pHzk16Lib;
	const Uint8 	*pAsc16Lib; 
	OsdDynParams	dynParams;
	Int32            (*fnAddOsd)(OsdAlgHandle ,Uint8 *,const char*,Uint16,Uint16);
	Uint8			tagY;
	Uint8			tagU;
	Uint8			tagV;
	Int32			offset[2];
	Bool			skipOddLine;
	Bool			zoom2X;
	Int32			yShift;
	Int32			xShift;
	Bool			scanDown;		//Scan from up to down, or from down to up
#ifdef OSD_CODE_CONVERT
	iconv_t 		codeConvert;	//For utf8 to gb2312 convert
	char			bufConvert[OSD_BUF_SIZE];
#endif
}OsdObj;

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/
const OsdInitParams OSD_INIT_DEFAULT = {
	.size = sizeof(OsdInitParams),
	.hzk16Tab = NULL,
	.asc16Tab = NULL,
};

const OsdDynParams OSD_DYN_DEFAULT = {
	.size = sizeof(OsdDynParams),
	.imgFormat = FMT_YUV_420SP,
	.width = 1920,
	.height = 1080,
	.mode = OSD_MODE_32x16,
	.color = OSD_COLOR_YELLOW,
};

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/*****************************************************************************
 Prototype    : osd_replace_pixel
 Description  : replace one pixel to some color for osd
 Input        : OsdAlgHandle hOsd  
                Uint16 x        
                Uint16 y        
                Uint8 *bufY     
                Uint8 *bufU     
                Uint8 *bufV     
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/20
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static inline void osd_replace_pixel(OsdAlgHandle hOsd, Uint16 x, Uint16 y, Uint8 *bufY, Uint8 *bufU, Uint8 *bufV)
{
	Uint16 width = hOsd->dynParams.width;
	Int32  offset = y * width + x;
	
	/* set y,u,v (2*2 points <==> 1 dots in char) */
	*(bufY + offset) = hOsd->tagY;
	*(bufY + offset + 1) = hOsd->tagY;
	if(!hOsd->skipOddLine || (y % 2) == 0) {
		offset = (y * width >> hOsd->yShift) + (x >> hOsd->xShift);
		*(bufU + offset) = hOsd->tagU;
		*(bufV + offset) = hOsd->tagV;
	}
}

/*****************************************************************************
 Prototype    : YUVAddOSD32x16
 Description  : osd 32*16 function
 Input        : OSD_HANDLE_ST *hOsd  
                Uint8 yuvpic                 
                Uint32 width                 
                Uint32 height                
                const Int8* osd              
                Uint32 rawx                  
                Uint32 rawy                  
                Uint32 (Uint32)(hOsd->eColor)                 
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2011/2/28
    Author       : byon
    Modification : Created function

*****************************************************************************/
static Int32 osd_add(OsdAlgHandle hOsd, Uint8 *yuvpic, const char* osd, Uint16 rawx, Uint16 rawy)
{
	Uint8 *offsetY, *offsetU, *offsetV;
	unsigned long offset;
	Int32 i, j, k, x, y;
	Int32 len;
	Uint16 width, height;
	
	width = hOsd->dynParams.width;
	height = hOsd->dynParams.height;

	/* init position */
	len = strlen(osd);
	rawx = ROUND_DOWN(rawx, OSD_POS_ALIGN);
	rawy = ROUND_DOWN(rawy, OSD_POS_ALIGN);
	
	if(rawx + (len << 4) >= width || rawy + 32 >= height || len == 0) {
		ERR("osd position out of range.");
		return E_INVAL;
	}
	
	offsetY = yuvpic;
	offsetU = yuvpic + hOsd->offset[0];
	offsetV = yuvpic + hOsd->offset[1];

	for(i = 0; i < len; i++) {
		/* the i th char */
		Uint8 osdchar = *(Uint8 *)(osd + i);
		if(osdchar < OSD_MIN_VAL || ((osdchar > OSD_ASCII_MAX) && (osdchar < OSD_HZ_MIN)))
			continue;

		if(osdchar <= OSD_ASCII_MAX) {
			/* ascii char */
			offset = (osdchar - 32) << 4;
			for(j = 0, y = rawy; j < 16 && y < height; j++, y++) {
				Uint8 p8 = *(hOsd->pAsc16Lib + offset + j);
				Uint8 mask8 = 0x80;

				for(k = 0, x = rawx + (i << 4); k < 8 && x < width - 1; k++, x += 2) {
					if(p8 & mask8) {
						/* set y,u,v (2*2 points <==> 1 dots in char) */
						osd_replace_pixel(hOsd, x, y, offsetY, offsetU, offsetV);
						if(hOsd->zoom2X)
							osd_replace_pixel(hOsd, x, y + 1, offsetY, offsetU, offsetV);
						
					}
					mask8 = mask8 >> 1;
				}

				if(hOsd->zoom2X)
					y++;
			}
		} else {
			/* chinese char */
			Uint8 osdchar2 = *(Uint8 *)(osd + i + 1);
			offset = (94 * (osdchar - 0xa1) + (osdchar2 - 0xa1)) << 5;
			for(j = 0, y = rawy; j < 16 && y < height - 1; j++, y++) {
				Uint16 p16 = *(unsigned short*)(hOsd->pHzk16Lib + offset + (j << 1));
				Uint16 mask16 = 0x0080;

				for(k = 0, x = rawx + (i << 4); k < 16 && x < width-1; k++, x+=2) {
					if(p16 & mask16) {
						/* set y,u,v (2*2 points <==> 1 dots in char) */
						osd_replace_pixel(hOsd, x, y, offsetY, offsetU, offsetV);
						if(hOsd->zoom2X)
							osd_replace_pixel(hOsd, x, y + 1, offsetY, offsetU, offsetV);
					}
					mask16 = mask16 >> 1;
					if(mask16 == 0)
						mask16 = 0x8000;
				}

				if(hOsd->zoom2X)
					y++;
			}
			
			i++;
		}
	}

	return E_NO;
}

#if 1



/*****************************************************************************
 Prototype    : YUVAddOSD32x32ForLeftRotate
 Description  : osd 32*32 plus left rotate
 Input        : OsdAlgHandle hOsd  
                Uint8 yuvpic                  
                Uint32 width                  
                Uint32 height                 
                const Int8* osd               
                Uint32 rawx                   
                Uint32 rawy                   
                Uint32 color                  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2011/2/28
    Author       : byon
    Modification : Created function

*****************************************************************************/
static Int32 osd_add_left_rotate(OsdAlgHandle hOsd, Uint8 *yuvpic, const char* osd, Uint16 rawx, Uint16 rawy)
{
	Uint8 *offsetY, *offsetU, *offsetV;
	unsigned long offset;
	Int32 i, j, k, x, y;
	Int32 len;
	Uint16 width, height;
	
	/* Switch height & width for rotation */
	height = hOsd->dynParams.height;
	width = hOsd->dynParams.width;

	/* init position */
	len = strlen(osd);
	rawx = ROUND_DOWN(rawx, OSD_POS_ALIGN);
	rawy = ROUND_DOWN(rawy, OSD_POS_ALIGN);
	
	if(rawx + (len << 4) >= width || rawy + 32 >= height || len == 0) {
		ERR("osd position out of range.");
		return E_INVAL;
	}
	
	offsetY = yuvpic;
	offsetU = yuvpic + hOsd->offset[0];
	offsetV = yuvpic + hOsd->offset[1];
	
	for(i = 0; i < len; i++) {
		/* the i th char */
		Uint8 osdchar = *(Uint8 *)(osd + i);
		if(osdchar < OSD_MIN_VAL || ((osdchar > OSD_ASCII_MAX) && (osdchar < OSD_HZ_MIN)))
			continue;

		if(osdchar <= OSD_ASCII_MAX) {
			// ascii char
			offset = (osdchar - 32) << 4;
			for(j = 0, x = rawy; j < 16 && x < width-1; j++, x+=2) {
				Uint8 p8 = *(hOsd->pAsc16Lib + offset + j);
				Uint8 mask8 = 0x80;

				for(k = 0, y = height - (rawx + (i << 4)); k < 8 && y > 0; k++, y-=2){
					if(p8 & mask8) {
						/* set y,u,v (2*2 points <==> 1 dots in char) */
						osd_replace_pixel(hOsd, x, y - 1, offsetY, offsetU, offsetV);
						osd_replace_pixel(hOsd, x, y, offsetY, offsetU, offsetV);
					}
					mask8 = mask8 >> 1;
				}
			}
		} else {
			// chinese char
			Uint8 osdchar2 = *(Uint8 *)(osd + i + 1);
			offset = (94 * (osdchar - 0xa1) + (osdchar2 - 0xa1)) << 5;
			for(j = 0, x = rawy; j < 16 && x < width-1; j++, x+=2) {
				Uint16 p16 = *(Uint16 *)(hOsd->pHzk16Lib + offset + j*2);
				Uint16 mask16 = 0x0080;

				for(k = 0, y = height - (rawx + (i << 4)); k < 16 && y > 0; k++, y-=2) {
					if(p16 & mask16) {
						/* set y,u,v (2*2 points <==> 1 dots in char) */
						osd_replace_pixel(hOsd, x, y - 1, offsetY, offsetU, offsetV);
						osd_replace_pixel(hOsd, x, y, offsetY, offsetU, offsetV);
					}
					mask16 = mask16 >> 1;
					if(mask16 == 0)
						mask16 = 0x8000;
				}
			}
			i++;
		}
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : YUVAddOSD32x32ForRightRotate
 Description  : osd 32*32 plus right rotate
 Input        : OsdAlgHandle hOsd  
                Uint8 yuvpic                  
                Uint32 width                  
                Uint32 height                 
                const Int8* osd               
                Uint32 rawx                   
                Uint32 rawy                   
                Uint32 color                  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2011/2/28
    Author       : byon
    Modification : Created function

*****************************************************************************/
static Int32 osd_add_right_rotate(OsdAlgHandle hOsd, Uint8 *yuvpic, const char* osd, Uint16 rawx, Uint16 rawy)
{
	Uint8 *offsetY, *offsetU, *offsetV;
	unsigned long offset;
	Int32 i, j, k, x, y;
	Int32 len;
	Uint16 width, height;
	
	/* Switch height & width for rotation */
	height = hOsd->dynParams.height;
	width = hOsd->dynParams.width;

	/* init position */
	len = strlen(osd);
	rawx = ROUND_DOWN(rawx, OSD_POS_ALIGN);
	rawy = ROUND_DOWN(rawy, OSD_POS_ALIGN);
	
	if(rawx + (len << 4) >= width || rawy + 32 >= height || len == 0) {
		ERR("osd position out of range.");
		return E_INVAL;
	}
	
	offsetY = yuvpic;
	offsetU = yuvpic + hOsd->offset[0];
	offsetV = yuvpic + hOsd->offset[1];

	
	for(i = 0; i < len; i++) {
		Uint8 osdchar = *(Uint8 *)(osd + i);
		if(osdchar < OSD_MIN_VAL || ((osdchar > OSD_ASCII_MAX) && (osdchar < OSD_HZ_MIN)))
			continue;

		if(osdchar <= OSD_ASCII_MAX) {
			/* ascii char */
			offset = (osdchar - 32) << 4;
			for(j = 0, x = width - rawy - 2; j < 16 && x > 0; j++, x -= 2) {
				Uint8 p8 = *(hOsd->pAsc16Lib + offset + j);
				Uint8 mask8 = 0x80;

				for(k = 0, y = rawx + (i << 4); k < 8 && y < height-1; k++, y += 2) {
					if(p8 & mask8) {
						/* set y,u,v (2*2 points <==> 1 dots in char) */
						osd_replace_pixel(hOsd, x, y, offsetY, offsetU, offsetV);
						osd_replace_pixel(hOsd, x, y + 1, offsetY, offsetU, offsetV);
					}
					mask8 = mask8 >> 1;
				}
			}
		} else {
			/* chinese char */
			Uint8 osdchar2 = *(Uint8 *)(osd + i + 1);
			offset = (94*(osdchar - 0xa1) + (osdchar2 - 0xa1)) << 5;
			for(j = 0, x = width - rawy - 2; j < 16 && x > 0; j++, x -= 2) {
				Uint16 p16 = *(Uint16 *)(hOsd->pHzk16Lib + offset + j*2);
				Uint16 mask16 = 0x0080;

				for(k = 0, y = rawx + (i << 4); k < 16 && y < height - 1; k++, y += 2) {
					if(p16 & mask16) {
						/* set y,u,v (2*2 points <==> 1 dots in char) */
						osd_replace_pixel(hOsd, x, y, offsetY, offsetU, offsetV);
						osd_replace_pixel(hOsd, x, y + 1, offsetY, offsetU, offsetV);
					}
					mask16 = mask16 >> 1;
					if(mask16 == 0)
						mask16 = 0x8000;
				}
			}
			i++;
		}
	}

	return E_NO;
}
#endif


/*****************************************************************************
 Prototype    : osd_set_format
 Description  : Set format
 Input        : OsdAlgHandle hOsd      
                eOsdFormat eFormat  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2011/9/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 osd_set_format(OsdAlgHandle hOsd, OsdMode mode)
{
	if(!hOsd)
		return E_INVAL;
	
	switch(mode)
	{
		case OSD_MODE_32x16 :
		{
			hOsd->fnAddOsd = osd_add;
			hOsd->dynParams.mode = mode;
			hOsd->zoom2X = FALSE;
			hOsd->scanDown = TRUE;
			break;
		}
		case OSD_MODE_32x32 :
		{
			hOsd->fnAddOsd = osd_add;
			hOsd->dynParams.mode = mode;
			hOsd->zoom2X = TRUE;
			hOsd->scanDown = TRUE;
			break;
		}
		case OSD_MODE_32x32_ROTATE_L:
		{
			hOsd->fnAddOsd = osd_add_left_rotate;
			hOsd->dynParams.mode = mode;
			hOsd->zoom2X = TRUE;
			hOsd->scanDown = FALSE;
			break;
		}
		case OSD_MODE_32x32_ROTATE_R :
		{
			hOsd->fnAddOsd = osd_add_right_rotate;
			hOsd->dynParams.mode = mode;
			hOsd->zoom2X = TRUE;
			hOsd->scanDown = TRUE;
			break;
		}
		default:
		{
			hOsd->fnAddOsd = osd_add;
			hOsd->dynParams.mode = OSD_MODE_32x16;
			hOsd->zoom2X = FALSE;
			break;
		}
	}

	return E_NO;
}

/*****************************************************************************
 Prototype    : osdParamCfg
 Description  : osd parameter configure
 Input        : OsdAlgHandle hOsd  
                OSD_FORMAT_EN enOsdFormat     
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2011/2/28
    Author       : byon
    Modification : Created function

*****************************************************************************/
static Int32 osd_set_color(OsdAlgHandle hOsd, OsdColor color)
{
	if(!hOsd || color < OSD_COLOR_YELLOW || color >= OSD_COLOR_MAX)
		return E_INVAL;

	// set the yuv value of font
	switch(color)
	{
		case OSD_COLOR_YELLOW: 		// Yellow
			hOsd->tagY = 226;
			hOsd->tagU = 0;
			hOsd->tagV = 149; 			
			break;
		case OSD_COLOR_RED: 		// Red
			hOsd->tagY = 76;
			hOsd->tagU = 85;
			hOsd->tagV = 255; 			
			break;
		case OSD_COLOR_GREEN: 		// Green
			hOsd->tagY = 150;
			hOsd->tagU = 44;
			hOsd->tagV = 21;				
			break;
		case OSD_COLOR_BLUE: 		// Blue
			hOsd->tagY = 29;
			hOsd->tagU = 255;
			hOsd->tagV = 107; 			
			break;
		default:		// White
			hOsd->tagY = 128;
			hOsd->tagU = 128;
			hOsd->tagV = 128; 	
			break;
	}
	
	hOsd->dynParams.color = color;
	
	return E_NO;
}

static Int32 osd_set_input_chroma_format(OsdAlgHandle hOsd, ChromaFormat format)
{
	if(!hOsd)
		return E_INVAL;

	switch(format) {
	case FMT_YUV_422P:
		hOsd->offset[0] = hOsd->dynParams.width * hOsd->dynParams.height;
		hOsd->offset[1] = hOsd->offset[0] + (hOsd->dynParams.width * hOsd->dynParams.height >> 1);
		hOsd->skipOddLine = FALSE;
		hOsd->yShift = 1;
		hOsd->xShift = 1;
		break;
	case FMT_YUV_420P:
		hOsd->offset[0] = hOsd->dynParams.width * hOsd->dynParams.height;
		hOsd->offset[1] = hOsd->offset[0] + (hOsd->dynParams.width * hOsd->dynParams.height >> 2);
		hOsd->skipOddLine = TRUE;
		hOsd->yShift = 2;
		hOsd->xShift = 1;
		break;
	case FMT_YUV_420SP:
		hOsd->offset[0] = hOsd->dynParams.width * hOsd->dynParams.height;
		hOsd->offset[1] = hOsd->offset[0] + 1;
		hOsd->skipOddLine = TRUE;
		hOsd->yShift = 1;
		hOsd->xShift = 0;
		break;
	default:
		ERR("input chroma format unsupported.");
		return E_UNSUPT;
	}

	hOsd->dynParams.imgFormat = format;
	return E_NO;
}


static Int32 osd_set_dyn_params(OsdAlgHandle hOsd, OsdDynParams *dynParams)
{
	if(!hOsd || !dynParams || dynParams->size != sizeof(OsdDynParams)) {
		ERR("invalid size");
		return E_INVAL;
	}

	if(dynParams->color >= OSD_COLOR_MAX) {
		ERR("invalid color.");
		return E_INVAL;
	}

	if( dynParams->imgFormat != FMT_YUV_422P &&
		dynParams->imgFormat != FMT_YUV_420P &&
		dynParams->imgFormat != FMT_YUV_420SP ) {
		ERR("input chroma format unsupported.");
		return E_UNSUPT;
	}

	if(dynParams->mode >= OSD_MODE_MAX) {
		ERR("invalid osd format.");
		return E_INVAL;
	}

	hOsd->dynParams = *dynParams;
	osd_set_format(hOsd, dynParams->mode);
	osd_set_color(hOsd, dynParams->color);
	osd_set_input_chroma_format(hOsd, dynParams->imgFormat);
	
	return E_NO;
	
}


/*****************************************************************************
 Prototype    : osdCreate
 Description  : osd handle create
 Input        : OsdAlgHandle hOsd  
                OSD_FORMAT_EN enOsdFormat     
                Uint8 *pOsdHzkData            
                Uint32 unHzkTableLen          
                Uint8 *pOsdAscData            
                Uint32 unAscTableLen          
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2011/2/28
    Author       : byon
    Modification : Created function

*****************************************************************************/
static Ptr _osd_init(const Ptr init, const Ptr dyn)
{
	OsdAlgHandle 		hOsd;
	const OsdInitParams	*initParams = (OsdInitParams *)init;
	OsdDynParams	*dynParams = (OsdDynParams *)dyn;

	hOsd = (OsdAlgHandle)calloc(1, sizeof(OsdObj));

	if(!hOsd) {
		ERR("Alloc mem failed.");
		return NULL;
	}

	if(!init)
		initParams = &OSD_INIT_DEFAULT;

	if(!dynParams)
		dynParams = (OsdDynParams *)&OSD_DYN_DEFAULT;
	
#ifdef INCLUDE_OSD_FRONT
	extern const Uint8 c_ucAsc16Table[];
	extern const Uint8 c_ucHzk16Table[];

	hOsd->pHzk16Lib = c_ucHzk16Table;
	hOsd->pAsc16Lib = c_ucAsc16Table;
#else
    hOsd->pHzk16Lib = initParams->hzk16Tab;
    hOsd->pAsc16Lib = initParams->asc16Tab;	
#endif
	assert(hOsd->pHzk16Lib && hOsd->pAsc16Lib);

	if(osd_set_dyn_params(hOsd, dynParams) != E_NO) {
		free(hOsd);
		return NULL;
	}

#ifdef OSD_CODE_CONVERT
	hOsd->codeConvert = iconv_open("gb2312", "utf-8"); 
	if(hOsd->codeConvert == (iconv_t)(-1)) {
		ERRSTR("open code convert failed...");
		free(hOsd);
		return NULL;
	}
#endif
	return hOsd;
}

/*****************************************************************************
 Prototype    : osd_delete
 Description  : Delete OSD handle
 Input        : OsdAlgHandle hOsd  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2011/9/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 _osd_exit(Ptr handle)
{
	OsdAlgHandle hOsd = (OsdAlgHandle)handle;
	
	if(!hOsd)
		return E_INVAL;
	
#ifdef OSD_CODE_CONVERT	
	if(hOsd->codeConvert)
		iconv_close(hOsd->codeConvert); 
#endif

	free(hOsd);

	return E_NO;
}


/*****************************************************************************
 Prototype    : osd_add
 Description  : Add OSD string to YUV image
 Input        : OsdAlgHandle hOsd     
                Uint8 *pYuvImg     
                const Int8 *szOsd  
                Uint16 usStartX    
                Uint16 usStartY    
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2011/9/14
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 _osd_process(Ptr algHandle, AlgBuf *inBuf, Ptr inArgsPtr, AlgBuf *outBuf, Ptr outArgsPtr)
{
	OsdAlgHandle hOsd = (OsdAlgHandle)algHandle;
	OsdInArgs *inArgs = (OsdInArgs *)inArgsPtr;
	
	if(!hOsd || !inBuf || !inArgs || inArgs->size != sizeof(OsdInArgs))
		return E_INVAL;

	const char *osdStr = inArgs->strOsd;

#ifdef OSD_CODE_CONVERT
	/* Convert UTF-8 code to ASCII & GB2312 */
	memset(hOsd->bufConvert, 0, OSD_BUF_SIZE);
	size_t inlen = strlen(inArgs->strOsd);
	size_t outlen = OSD_BUF_SIZE;
	
	char **pin = (char **)(inArgs->strOsd);
	char **pout = (char **)(hOsd->bufConvert);

	DBG("convert code");
 	if(iconv(hOsd->codeConvert, pin, &inlen, pout, &outlen) < 0) {
		ERRSTR("convert code failed...");
		return E_IO; 
	}
	hOsd->bufConvert[OSD_BUF_SIZE - 1] = 0;
	DBG("convert finish, %s, left: %d", hOsd->bufConvert, inlen);
#endif

	return hOsd->fnAddOsd(hOsd, inBuf->buf, osdStr, inArgs->startX, inArgs->startY);
}

/*****************************************************************************
 Prototype    : osd_control
 Description  : IO control fo alg
 Input        : Ptr algHandle  
                Int32 cmd      
                Ptr args       
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/2/20
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 _osd_control(Ptr algHandle, Int32 cmd, Ptr args)
{
	OsdAlgHandle hOsd = (OsdAlgHandle)algHandle;
	
	if(!algHandle)
		return E_INVAL;

	Int32 err;
	
	switch(cmd) {
	case ALG_CMD_SET_DYN_PARAMS:
		err = osd_set_dyn_params(hOsd, (OsdDynParams *)args);
		break;
	default:
		err = E_UNSUPT;
		break;
	}

	return err;
}

/* structure for alg functions */
const AlgFxns OSD_ALG_FXNS = {
	.algInit = _osd_init,
	.algProcess = _osd_process,
	.algControl = _osd_control,
	.algExit = _osd_exit,
};

