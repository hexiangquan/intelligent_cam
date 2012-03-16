/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : img_enc_thr.c
  Version       : Initial Draft
  Author        : Sun
  Created       : 2012/3/13
  Last Modified :
  Description   : send image data thread
  Function List :
              img_enc_params_update
              img_enc_thr
              img_enc_thr_init
              img_enc_thr_run
              msg_process
  History       :
  1.Date        : 2012/3/13
    Author      : Sun
    Modification: Created file

******************************************************************************/
#include "img_enc_thr.h"
#include "jpg_enc.h"
#include "add_osd.h"
#include "log.h"
#include "crc16.h"

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define IMG_MAX_WIDTH		3144
#define IMG_MAX_HEIGHT		2560
#define ENC_POOL_BUF_NUM	3
#define BUF_ALLOC_TIMEOUT	100		//ms

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/




/* thread environment */
typedef struct {
	ParamsMngHandle hParamsMng;
	JpgEncHandle	hJpgEnc;
	MsgHandle		hMsg;
	OsdHandle		hOsd;
	CamOsdParams	osdCfg;
	BufPoolHandle	hPoolEnc;
	BufHandle		hBufSave;		//buffer for save to sd
	FrameDispHandle	hDispatch;
	Bool			exit;
}ImgEncThrEnv;

/*****************************************************************************
 Prototype    : img_enc_thr_init
 Description  : Init thread
 Input        : ImgEncThrArg *arg   
                ImgEncThrEnv *envp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 img_enc_thr_init(ImgEncThrArg *arg, ImgEncThrEnv *envp)
{
	Int32 err;
	
	assert(arg && arg->hParamsMng && arg->hDispatch);

	/* clear */
	memset(envp, 0, sizeof(ImgEncThrEnv));
	envp->hParamsMng = arg->hParamsMng;
	envp->hDispatch = arg->hDispatch;

	/* no longer need input arg */
	free(arg);

	/* init jpeg encode params */
	JpgEncInitParams	encInitParams;
	JpgEncDynParams		encDynParams;

	encInitParams.maxHeight = IMG_MAX_HEIGHT;
	encInitParams.maxWidth = IMG_MAX_WIDTH;
	encInitParams.size = sizeof(encInitParams);

	/* get dyn params, we may need wait capture ready */
	err = params_mng_control(envp->hParamsMng, PMCMD_G_JPGENCDYN, 
			&encDynParams, sizeof(encDynParams));
	if(err) {
		ERR("get jpg enc dyn params failed.");
		return err;
	}

	/* create jpg enc handle */
	envp->hJpgEnc = jpg_enc_create(&encInitParams, &encDynParams);
	if(!envp->hJpgEnc) {
		ERR("create jpg enc handle failed");
		return E_INVAL;
	}

	/* create osd handle */
	OsdInitParams	osdInitParams;
	OsdDynParams	osdDynParams;

	osdInitParams.size = sizeof(osdInitParams);
	osdInitParams.asc16Tab = NULL;
	osdInitParams.hzk16Tab = NULL;

	err = params_mng_control(envp->hParamsMng, PMCMD_G_IMGOSDDYN, 
			&osdDynParams, sizeof(osdDynParams));
	if(err) {
		ERR("get jpg enc dyn params failed.");
		return err;
	}	
	
	envp->hOsd = osd_create(&osdInitParams, &osdDynParams);
	if(!envp->hOsd) {
		ERR("create osd handle failed");
		return E_INVAL;
	}

	/* get current osd config */
	err = params_mng_control(envp->hParamsMng, PMCMD_G_OSDPARAMS, 
				&envp->osdCfg, sizeof(CamOsdParams));
	assert(err == E_NO);

	/* create msg handle  */
	envp->hMsg = msg_create(MSG_IMG_ENC, MSG_IMG_TX, 0);
	if(!envp->hMsg) {
		ERR("create msg handle failed");
		return E_NOMEM;
	}

	/* create buffer pool for encoded image */
	Uint32 bufSize = encDynParams.width * encDynParams.height * 8 / 10;
	bufSize = ROUND_UP(bufSize, 256);
	envp->hPoolEnc = buf_pool_create(bufSize, ENC_POOL_BUF_NUM, NULL);
	if(!envp->hPoolEnc) {
		ERR("create enc pool failed");
		return E_NOMEM;
	}

	/* alloc one buffer for save to local */
	envp->hBufSave = buffer_alloc(bufSize, NULL);
	if(!envp->hBufSave) {
		ERR("alloc buf for save failed");
		return E_NOMEM;
	}

	envp->exit = FALSE;

	DBG("img enc thread init ok.");
	return E_NO;
}

#if 0
/*****************************************************************************
 Prototype    : img_conv_update
 Description  : update img conv alg
 Input        : CapThrEnv *envp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/7
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 img_conv_update(CapThrEnv *envp)
{
	Int32 				err;
	ImgConvDynParams 	*pConvDynParams;
	
	pConvDynParams = &envp->convDynParams;
	err = params_mng_control(envp->hParamsMng, PMCMD_G_IMGCONVDYN, 
				pConvDynParams, sizeof(ImgConvDynParams));
	if(err)
		return err;

	/* config alg */
	err = img_conv_control( envp->hImgConv, 
							ALG_CMD_SET_DYN_PARAMS, 
							pConvDynParams );
	if(err) {
		ERR(" config for img enc err.");
		return err;
	}

	return E_NO;
}

#endif
/*****************************************************************************
 Prototype    : img_enc_thr_run
 Description  : add osd and do jpeg encode
 Input        : ImgEncThrEnv *envp  
                ImgMsg *msg         
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 img_enc_thr_run(ImgEncThrEnv *envp, ImgMsg *msg)
{
	Int32 	err;
	
	/* Add osd first & ignore error */
	err = add_osd(envp->hOsd, msg, &envp->osdCfg.imgOsd);

	/* Init buffer and args for jpeg encode   */
	AlgBuf			inBuf, outBuf;
	BufHandle		hBufIn, hBufOut;
	JpgEncInArgs	inArgs;
	JpgEncOutArgs	outArgs;
	Int32			dispFlags = 0;
	
	hBufIn = msg->hBuf;
	inBuf.buf = buffer_get_user_addr(msg->hBuf);
	inBuf.bufSize = buffer_get_size(msg->hBuf);
	assert(inBuf.buf && inBuf.bufSize);

#if 0
	/* do image convert */
		inBuf.buf = capFrame.dataBuf;
		inBuf.bufSize = capFrame.bufSize;
		outBuf.buf = buffer_get_user_addr(hBuf);
		outBuf.bufSize = buffer_get_size(hBuf);
		
		err = img_conv_process(envp->hImgConv, &inBuf, NULL, &outBuf, NULL);
		if(err) {
			ERR("imgConv err.");
			goto free_buf;
		}
		
#ifdef CRC_EN
		imgMsg->header.param[0] = crc16(outBuf.buf, imgMsg->dimension.size);
#endif
#endif

#ifdef CRC_EN
	Int32 crc = crc16(inBuf.buf, msg->dimension.size);
	if(crc != msg->header.param[0])
		ERR("crc check error");
#endif

	/* Alloc buffer for encoded data */
	hBufOut = buf_pool_alloc_wait(envp->hPoolEnc, BUF_ALLOC_TIMEOUT);
	if(!hBufOut) {
		/* alloc buffer failed, use buf for save */
		DBG("save file to local file system");
		dispFlags |= FD_FLAG_SAVE_ONLY | FD_FLAG_NOT_FREE_BUF;
		hBufOut = envp->hBufSave;
	}
	
	outBuf.buf = buffer_get_user_addr(hBufOut);
	outBuf.bufSize = buffer_get_size(hBufOut);
	if(!outBuf.buf || !outBuf.bufSize) {
		ERR("got invalid encode buf handle...");
		err = E_NOMEM;
		goto err_quit;
	}

	inArgs.size = sizeof(inArgs);
	inArgs.appendData = NULL;
	inArgs.appendSize = 0;
	inArgs.endMark = 0;

	outArgs.size = sizeof(outArgs);
	outArgs.bytesGenerated = 0;

	/* do jpeg encode */
	err = jpg_enc_process(envp->hJpgEnc, &inBuf, &inArgs, &outBuf, &outArgs);
	if(err) {
		ERR("jpg enc err: %s", str_err(err));
		goto err_quit;
	}

	/* free input buffer */
	static int cnt = 0;
	DBG("<%d>jpg enc free raw buf", ++cnt);
	buf_pool_free(hBufIn);
	hBufIn = NULL;

	/* modify frame info */
	msg->dimension.colorSpace = FMT_JPG;
	msg->dimension.size = outArgs.bytesGenerated;
	msg->hBuf = hBufOut;
	buffer_set_bytes_used(hBufOut, outArgs.bytesGenerated);
#ifdef CRC_EN
	msg->header.param[0] = crc16(outBuf.buf, outArgs.bytesGenerated);
#endif

	/* dispatch jpeg data */
	err = frame_disp_run(envp->hDispatch, envp->hMsg, msg, NULL, dispFlags);
	if(err) {
		ERR("dispatch jpeg data failed");
		goto err_quit;
	}

	//DBG("<%d> jpg enc run ok", msg->index);

	return E_NO;

err_quit:
	if(hBufIn)
		buf_pool_free(hBufIn);
	
	if(hBufOut && !dispFlags)
		buf_pool_free(hBufOut);

	return err;
}

/*****************************************************************************
 Prototype    : img_enc_params_update
 Description  : update encode thread params
 Input        : ImgEncThrEnv *envp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 img_enc_params_update(ImgEncThrEnv *envp)
{
	OsdDynParams	osdDynParams;
	JpgEncDynParams	encDynParams;
	Int32 			err;

	/* update osd params */
	err = params_mng_control(envp->hParamsMng, PMCMD_G_IMGOSDDYN, 
			&osdDynParams, sizeof(osdDynParams));
	err = osd_control(envp->hOsd, OSD_CMD_SET_DYN_PARAMS, &osdDynParams);
	if(err)
		ERR("set osd params err.");

	err = params_mng_control(envp->hParamsMng, PMCMD_G_OSDPARAMS, 
				&envp->osdCfg, sizeof(CamOsdParams));

	/* update jpeg enc params */
	err = params_mng_control(envp->hParamsMng, PMCMD_G_JPGENCDYN, 
			&encDynParams, sizeof(encDynParams));
	err = jpg_enc_control(envp->hJpgEnc, JPGENC_CMD_SET_DYN_PARAMS, &encDynParams);
	if(err)
		ERR("set jpg enc params err.");

	return err;
}

/*****************************************************************************
 Prototype    : msg_process
 Description  : process msg
 Input        : ImgEncThrEnv *envp  
                CommonMsg *msgBuf   
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 msg_process(ImgEncThrEnv *envp, CommonMsg *msgBuf)
{
	Int32 ret;

	/* recv msg */
	ret = msg_recv(envp->hMsg, msgBuf, sizeof(CommonMsg));
	if(ret < 0) {
		ERR("img enc thr recv msg err: %s", str_err(ret));
		return ret;
	}

	/* process msg */
	MsgHeader *msgHdr = &msgBuf->header;
	switch(msgHdr->cmd) {
	case APPCMD_NEW_DATA:
		ret = img_enc_thr_run(envp, (ImgMsg *)msgBuf);
		break;
	case APPCMD_SET_ENC_PARAMS:
		ret = img_enc_params_update(envp);
		break;
	case APPCMD_EXIT:
		envp->exit = TRUE;
		break;
	default:
		ERR("unkown cmd: 0x%X", (unsigned int)msgHdr->cmd);
		ret = E_UNSUPT;
		break;
	}

	return ret;
}

/*****************************************************************************
 Prototype    : img_enc_thr
 Description  : image encode thread
 Input        : void *arg  
 Output       : None
 Return Value : void
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
void *img_enc_thr(void *arg)
{
	ImgEncThrEnv 	env;
	CommonMsg		msgBuf;
	Int32			ret;
	Int32			fdMsg, fdMax;
	fd_set			rdSet;

	ret = img_enc_thr_init((ImgEncThrArg *)arg, (ImgEncThrEnv *)&env);
	assert(ret == E_NO);
	if(ret)
		goto exit;

	fdMsg = msg_get_fd(env.hMsg);
	fdMax = fdMsg + 1;

	/* start main loop */
	while(!env.exit) {
		/* wait data ready */
		FD_ZERO(&rdSet);
		FD_SET(fdMsg, &rdSet);
		
		ret = select(fdMax, &rdSet, NULL, NULL, NULL);
		if(ret < 0 && errno != EINTR) {
			ERRSTR("select err");
			break;
		}

		/* no data ready */
		if(!ret)
			continue;

		if(FD_ISSET(fdMsg, &rdSet)) {
			/* process msg */
			msg_process(&env, &msgBuf);
		}
	}

exit:
	if(env.hJpgEnc)
		jpg_enc_delete(env.hJpgEnc);

	if(env.hOsd)
		osd_delete(env.hOsd);

	if(env.hPoolEnc)
		buf_pool_delete(env.hPoolEnc);

	if(env.hMsg)
		msg_delete(env.hMsg);

	INFO("img encode thread exit...");
	pthread_exit(0);
	
}

