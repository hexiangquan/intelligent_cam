#include "img_snd_thr.h"
#include "upload.h"
#include "params_mng.h"
#include "log.h"
#include "crc16.h"

typedef struct {
	UploadHandle 			hUpload;
	ParamsMngHandle 		hParamsMng;
	FrameDispHandle			hDispatch;
	MsgHandle				hMsg;
	CamImageUploadProtocol	protol;
	Bool					exit;
}ImgSndThrEnv;

#define RECONNECT_TIMEOUT	5 	//second
#define MSG_RECV_TIMEOUT	25	//second
#define HEAT_BEAT_INTERVAL	60	//second

/*****************************************************************************
 Prototype    : img_snd_config
 Description  : config upload params
 Input        : ImgSndThrEnv *envp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 img_snd_config(ImgSndThrEnv *envp)
{
	CamImageUploadProtocol 	protol;
	Int8					buf[1024];
	Int32 					err;

	/* get current protol */
	err = params_mng_control(envp->hParamsMng, PMCMD_G_IMGTRANSPROTO, &protol, sizeof(protol));
	if(err) {
		ERR("get img trans protol failed...");
		return err;
	}

	if(protol != envp->protol && envp->hUpload) {
		/* delete previous handle because protol is changed  */
		err = upload_delete(envp->hUpload);
		assert(err == E_NO);
		envp->hUpload = NULL;
	}

	envp->protol = protol;

	/* get params */
	err = params_mng_control(envp->hParamsMng, PMCMD_G_IMGUPLOADPARAMS, buf, sizeof(buf));
	if(err) {
		ERR("get img upload params failed...");
		return err;
	}

	if(!envp->hUpload) {
		/* create upload module */
		envp->hUpload = upload_create(protol, buf, RECONNECT_TIMEOUT);
		if(!envp->hUpload)
			return E_IO;

		/* set connect status */
		frame_disp_set_tx_status(envp->hDispatch, FT_SRV_UNCONNECTED);
	} else {
		/* just update params */
		err = upload_set_params(envp->hUpload, buf);
	}

	return err;
}

/*****************************************************************************
 Prototype    : img_snd_thr_init
 Description  : init thread env
 Input        : ImgSndThrArg *arg   
                ImgSndThrEnv *envp  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/12
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 img_snd_thr_init(ImgSndThrArg *arg, ImgSndThrEnv *envp)
{
	assert(arg && arg->hDispatch && arg->hParamsMng);

	memset(envp, 0, sizeof(ImgSndThrEnv));
	envp->hDispatch = arg->hDispatch;
	envp->hParamsMng = arg->hParamsMng;

	/* arg is not needed */
	free(arg);

	Int32 err;

	err = img_snd_config(envp);
	if(err)
		return err;

	/* create msg handle  */
	envp->hMsg = msg_create(MSG_IMG_TX, MSG_MAIN, 0);
	if(!envp->hMsg) {
		ERR("create msg handle failed");
		return E_NOMEM;
	}

	envp->exit = FALSE;
	return err;
	
}

/*****************************************************************************
 Prototype    : img_snd_thr_run
 Description  : send image
 Input        : ImgSndThrEnv *envp  
                ImgMsg *imgBuf      
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 img_snd_thr_run(ImgSndThrEnv *envp, ImgMsg *imgBuf)
{
	Int32	err;
	Int32	dispFlags = 0;

#ifdef CRC_EN
	Int32 crc = crc16(buffer_get_user_addr(imgBuf->hBuf), imgBuf->dimension.size);

	if(crc != imgBuf->header.param[0])
		ERR("crc check error");
#endif
	
	err = upload_send_frame(envp->hUpload, imgBuf);
	if(err) {
		/* send err, reconnect the server and try again */
		err = upload_connect(envp->hUpload, RECONNECT_TIMEOUT);
		if(!err)
			err = upload_send_frame(envp->hUpload, imgBuf);
	} 

	if(err) {
		/* send error, we need save to local file system */
		dispFlags |= FD_FLAG_SAVE_ONLY;
		frame_disp_set_tx_status(envp->hDispatch, FT_SRV_UNCONNECTED);
	} else {
		frame_disp_set_tx_status(envp->hDispatch, FT_SRV_CONNECTED);
	}

	DBG("<%d> snd result: %d.", imgBuf->index, err);

	/* don't dispatch, just free */
	err = frame_disp_run(envp->hDispatch, NULL, imgBuf, NULL, dispFlags);
	
	return err;
}

/*****************************************************************************
 Prototype    : msg_process
 Description  : process msg
 Input        : ImgSndThrEnv *envp  
                CommonMsg *msgBuf   
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
static Int32 msg_process(ImgSndThrEnv *envp, CommonMsg *msgBuf)
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
		ret = img_snd_thr_run(envp, (ImgMsg *)msgBuf);
		break;
	case APPCMD_SET_UPLOAD_PARAMS:
		ret = img_snd_config(envp);
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
 Prototype    : img_snd_thr
 Description  : thread function for send frame
 Input        : void *arg  
 Output       : None
 Return Value : void
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/3/13
    Author       : Sun
    Modification : Created function

*****************************************************************************/
void *img_snd_thr(void *arg)
{
	ImgSndThrEnv 	env;
	Int32			ret;
	Int32			fdMsg, fdMax;
	fd_set			rdSet;
	CommonMsg		msgBuf;
	struct timeval 	tmVal;
	time_t			lastTime = time(NULL);

	/* init for thread env */
	ret = img_snd_thr_init((ImgSndThrArg *)arg, &env);
	assert(ret == E_NO);
	if(ret)
		goto exit;

	fdMsg = msg_get_fd(env.hMsg);
	fdMax = fdMsg + 1;

	while(1) {
		/* check if we need connect server */
		if(!upload_get_connect_status(env.hUpload)) {
			ret = upload_connect(env.hUpload, RECONNECT_TIMEOUT);
			if(!ret)
				frame_disp_set_tx_status(env.hDispatch, FT_SRV_CONNECTED);
			
		} else {
			/* keep alive with server */
			if(time(NULL) - lastTime > HEAT_BEAT_INTERVAL) {
				upload_send_heartbeat(env.hUpload);
				lastTime = time(NULL);
			}	
		}
		
		/* wait data ready */
		FD_ZERO(&rdSet);
		FD_SET(fdMsg, &rdSet);
		tmVal.tv_sec = MSG_RECV_TIMEOUT;
		tmVal.tv_usec = 0;
		
		ret = select(fdMax, &rdSet, NULL, NULL, &tmVal);
		if(ret < 0 && errno != EINTR) {
			ERRSTR("select err");
			break;
		}

		if(ret && FD_ISSET(fdMsg, &rdSet)) {
			/* process msg */
			msg_process(&env, &msgBuf);
		}
	}

exit:
	if(env.hUpload)
		upload_delete(env.hUpload);

	if(env.hMsg)
		msg_delete(env.hMsg);

	pthread_exit(0);
}

