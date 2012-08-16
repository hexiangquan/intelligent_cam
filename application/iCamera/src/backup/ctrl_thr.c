#include "ctrl_thr.h"
#include "log.h"

#define CTRL_MSG_BUF_LEN		(1 * 1024 * 1024)

typedef struct {
	MsgHeader 	hdr;
	Int8		buf[CTRL_MSG_BUF_LEN];
}CtrlMsg;

/* thread environment */
typedef struct {
	ParamsMngHandle hParamsMng;
	MsgHandle		hMsg;
	FrameDispHandle	hDispatch;
	CtrlMsg			*msgBuf;
	Bool			exit;
}CtrlThrEnv;

static Int32 ctrl_thr_init(CtrlThrArg *arg, CtrlThrEnv *envp)
{
	Int32 err;
	
	assert(arg && arg->hParamsMng && arg->hDispatch);

	/* clear */
	memset(envp, 0, sizeof(ImgEncThrEnv));
	envp->hParamsMng = arg->hParamsMng;
	envp->hDispatch = arg->hDispatch;

	/* no longer need input arg */
	free(arg);

	/* create msg handle  */
	envp->hMsg = msg_create(MSG_CTRL, MSG_MAIN, 0);
	if(!envp->hMsg) {
		ERR("create msg handle failed");
		return E_NOMEM;
	}

	/* alloc msg buffer */
	envp->msgBuf = calloc(1, sizeof(CtrlMsg));
	assert(envp->msgBuf);

	envp->exit = FALSE;

	DBG("ctrl thread init ok.");
	return E_NO;
}

static Int32 msg_process(CtrlThrEnv *envp)
{
	Int32 ret;

	
	return ret;
}

void *ctrl_thr(void *arg)
{
	CtrlThrEnv 		env;
	Int32			ret;

	ret = ctrl_thr_init((CtrlThrArg *)arg, (CtrlThrEnv *)&env);
	assert(ret == E_NO);
	if(ret)
		goto exit;

	/* start main loop */
	while(!env.exit) {		
		/* recv msg */
		ret = msg_recv(env.hMsg, env.msgBuf, sizeof(CtrlMsg));
		if(ret < 0) {
			ERR("ctrl thr recv msg err: %s", str_err(ret));
			continue;
		}
	
		/* process msg */
		MsgHeader *msgHdr = &env.msgBuf->hdr;
		switch(msgHdr->cmd) {
		case APPCMD_SET_IMG_ENC_PARAMS:
			ret = img_enc_params_update(envp);
			break;
		case APPCMD_EXIT:
			env.exit = TRUE;
			break;
		default:
			ERR("unkown cmd: 0x%X", (unsigned int)msgHdr->cmd);
			ret = E_UNSUPT;
			break;
		}
		
	}

exit:
	if(env.msgBuf)
		free(env.msgBuf);

	if(env.hMsg)
		msg_delete(env.hMsg);

	INFO("ctrl thread exit...");
	pthread_exit(0);
	
}

