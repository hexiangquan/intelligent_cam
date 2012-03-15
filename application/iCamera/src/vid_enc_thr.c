#include "vid_enc_thr.h"
#include "h264_enc.h"
#include "add_osd.h"
#include "log.h"

/* thread environment */
typedef struct {
	ParamsMngHandle hParamsMng;
	H264EncHandle	hH264Enc;
	MsgHandle		hMsg;
	OsdHandle		hOsd;
	CamOsdParams	osdCfg;
	BufHandle		hBufEnc;		//buffer for Encode
	FrameDispHandle	hDispatch;
	Bool			exit;
}VidEncThrEnv;

/*****************************************************************************
 Prototype    : vid_enc_thr_init
 Description  : Init thread
 Input        : VidEncThrArg *arg   
                VidEncThrEnv *envp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/8
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 vid_enc_thr_init(VidEncThrArg *arg, VidEncThrEnv *envp)
{
	Int32 err;
	
	assert(arg && arg->hParamsMng && arg->hDispatch);

	/* clear */
	memset(envp, 0, sizeof(VidEncThrEnv));
	envp->hParamsMng = arg->hParamsMng;
	envp->hDispatch = arg->hDispatch;

	/* no longer need input arg */
	free(arg);

	/* init jpeg encode params */
	H264EncInitParams	encInitParams;
	H264EncDynParams	encDynParams;

	encInitParams = H264ENC_INIT_DEFAULT;

	/* get dyn params, we may need wait capture ready */
	err = params_mng_control(envp->hParamsMng, PMCMD_G_H264ENCDYN, 
			&encDynParams, sizeof(encDynParams));
	if(err) {
		ERR("get h264 enc dyn params failed.");
		return err;
	}

	/* create h.264 enc handle */
	envp->hH264Enc = h264_enc_create(&encInitParams, &encDynParams);
	if(!envp->hH264Enc) {
		ERR("create h264 enc handle failed");
		return E_INVAL;
	}

	/* create osd handle */
	OsdInitParams	osdInitParams;
	OsdDynParams	osdDynParams;

	osdInitParams.size = sizeof(osdInitParams);
	osdInitParams.asc16Tab = NULL;
	osdInitParams.hzk16Tab = NULL;

	err = params_mng_control(envp->hParamsMng, PMCMD_G_VIDOSDDYN, 
			&osdDynParams, sizeof(osdDynParams));
	if(err) {
		ERR("get osd enc params failed.");
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
	envp->hMsg = msg_create(MSG_VID_ENC, MSG_IMG_TX, 0);
	if(!envp->hMsg) {
		ERR("create msg handle failed");
		return E_NOMEM;
	}

	/* alloc one buffer for encode */
	Uint32 bufSize = encInitParams.maxWidth * encInitParams.maxHeight * 8 / 10;
	bufSize = ROUND_UP(bufSize, 256);
	
	envp->hBufEnc = buffer_alloc(bufSize, NULL);
	if(!envp->hBufEnc) {
		ERR("alloc buf for enc failed");
		return E_NOMEM;
	}

	envp->exit = FALSE;

	DBG("vid enc thread init ok.");
	return E_NO;
}

/*****************************************************************************
 Prototype    : vid_enc_thr_run
 Description  : add osd and do jpeg encode
 Input        : VidEncThrEnv *envp  
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
static Int32 vid_enc_thr_run(VidEncThrEnv *envp, ImgMsg *msg)
{
	Int32 	err;
	
	/* Add osd first & ignore error */
	err = add_osd(envp->hOsd, msg, &envp->osdCfg.vidOsd);

	/* Init buffer and args for encode   */
	AlgBuf			inBuf, outBuf;
	BufHandle		hBufIn, hBufOut;
	H264EncInArgs	inArgs;
	H264EncOutArgs	outArgs;
	Int32			dispFlags = 0;
	
	hBufIn = msg->hBuf;
	inBuf.buf = buffer_get_user_addr(msg->hBuf);
	inBuf.bufSize = buffer_get_size(msg->hBuf);
	assert(inBuf.buf && inBuf.bufSize);

	dispFlags = FD_FLAG_NOT_FREE_BUF;
	hBufOut = envp->hBufEnc;
	
	outBuf.buf = buffer_get_user_addr(hBufOut);
	outBuf.bufSize = buffer_get_size(hBufOut);
	
	inArgs.size = sizeof(inArgs);
	inArgs.inputID = msg->index;
	inArgs.timeStamp = 0;
	
	outArgs.size = sizeof(outArgs);
	outArgs.bytesGenerated = 0;

	/* do h264 encode */
	err = h264_enc_process(envp->hH264Enc, &inBuf, &inArgs, &outBuf, &outArgs);
	if(err) {
		ERR("h264 enc err: %s", str_err(err));
		goto err_quit;
	}

	/* free input buffer */
	static int cnt = 0;
	DBG("<%d>h264 free raw buf",++cnt);
	buf_pool_free(hBufIn);
	hBufIn = NULL;

	/* modify frame info */
	msg->dimension.colorSpace = FMT_H264;
	msg->dimension.size = outArgs.bytesGenerated;
	msg->frameType = outArgs.frameType;
	msg->hBuf = hBufOut;
	buffer_set_bytes_used(hBufOut, outArgs.bytesGenerated);

	//DBG("<%d> h264 enc run ok", msg->index);

	/* dispatch h.264 data */
	err = frame_disp_run(envp->hDispatch, envp->hMsg, msg, NULL, dispFlags);
	if(err) {
		ERR("dispatch h264 data failed");
		goto err_quit;
	}

	return E_NO;

err_quit:
	if(hBufIn)
		buf_pool_free(hBufIn);
	
	return err;
}

/*****************************************************************************
 Prototype    : vid_enc_params_update
 Description  : update encode thread params
 Input        : VidEncThrEnv *envp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/9
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 vid_enc_params_update(VidEncThrEnv *envp)
{
	OsdDynParams	osdDynParams;
	H264EncDynParams	encDynParams;
	Int32 			err;

	/* update osd params */
	err = params_mng_control(envp->hParamsMng, PMCMD_G_VIDOSDDYN, 
			&osdDynParams, sizeof(osdDynParams));
	err = osd_control(envp->hOsd, OSD_CMD_SET_DYN_PARAMS, &osdDynParams);
	if(err)
		ERR("set osd params err.");

	err = params_mng_control(envp->hParamsMng, PMCMD_G_OSDPARAMS, 
				&envp->osdCfg, sizeof(CamOsdParams));

	/* update jpeg enc params */
	err = params_mng_control(envp->hParamsMng, PMCMD_G_H264ENCDYN, 
			&encDynParams, sizeof(encDynParams));
	err = h264_enc_control(envp->hH264Enc, H264ENC_CMD_SET_DYN_PARAMS, &encDynParams);
	if(err)
		ERR("set h264 enc params err.");

	return err;
}

/*****************************************************************************
 Prototype    : msg_process
 Description  : process msg
 Input        : VidEncThrEnv *envp  
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
static Int32 msg_process(VidEncThrEnv *envp, CommonMsg *msgBuf)
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
		ret = vid_enc_thr_run(envp, (ImgMsg *)msgBuf);
		break;
	case APPCMD_SET_ENC_PARAMS:
		ret = vid_enc_params_update(envp);
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
 Prototype    : vid_enc_thr
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
void *vid_enc_thr(void *arg)
{
	VidEncThrEnv 	env;
	CommonMsg		msgBuf;
	Int32			ret;
	Int32			fdMsg, fdMax;
	fd_set			rdSet;

	ret = vid_enc_thr_init((VidEncThrArg *)arg, (VidEncThrEnv *)&env);
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
	if(env.hH264Enc)
		h264_enc_delete(env.hH264Enc);

	if(env.hOsd)
		osd_delete(env.hOsd);

	if(env.hBufEnc)
		buffer_free(env.hBufEnc);

	if(env.hMsg)
		msg_delete(env.hMsg);

	INFO("vid encode thread exit...");
	pthread_exit(0);
	
}


