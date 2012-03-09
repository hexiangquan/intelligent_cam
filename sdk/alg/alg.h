#ifndef __ALG_H__
#define __ALG_H__

#include "common.h"

/**
 *  @brief      Algorithm instance object definition.
 *
 *  @remarks    All algorithm instance objects must have this
 *              structure as their first element.  However, they do not
 *              need to initialize it; initialization of this sub-structure
 *              is done by the "framework".
 */
typedef struct _AlgObj {
    const struct _AlgFxns 	*fxns;     	/* Pointer to alg function table. */
	Ptr						handle;		/* Hanlde for alg process */
	Ptr						cfg;		/* Configure info, reserved */
} AlgObj, *AlgHandle;

/**
 *  @brief      buffer info for process
 */
typedef struct _AlgBuf {
	Uint8	*buf;				/* Pointer of buffer */
	Int32	bufSize;			/* Size of this buffer */
}AlgBuf;

/**
 *  @brief      Functions for common algorithms
 */
typedef struct _AlgFxns {

/**
 *  @brief    Init an alg and create specific handle
 *
 *  @param[in]  initParams          Params for init
 *  @param[in]  dynParams          Default dynamic params
 *
 *  @return  NULL if failed, otherwise, return handle for this alg
 */
    Ptr    (*algInit)(const Ptr initParams, const Ptr dynParams);

/**
 *  @brief      Apps call this to run the algorithm
 *
 *  @param[in]  handle             Handle returns by algInit.
 *  @param[in]  inBuf         	     Buffer for input, can also use an structure for input
 *  @param[in]  inArgs              Input args, can be set as input buffer size
 *  @param[out] outBuf            Buffer for output, can also use an structure for output
 *  @param[out] outArgs           Output args, can also use an structure for output
 *
 *  @return	common error code defined in common.h
 *
 *  @remarks    algProcess runs the algorithm, every alg must specify the structure and usage
 *   			  of input and output params; 
 *			  If the alg does not need some params, i.e. outArgs, apps can set this 
 *			  param to NULL
 *
 *  @pre		must call algInit first
 */
    Int32     (*algProcess)(Ptr handle, AlgBuf *inBuf, Ptr inArgs, AlgBuf *outBuf, Ptr outArgs);

/**
 *  @brief      Apps call this to communicate with alg, i.e. set or get dynamic params
 *
 *  @param[in]  handle            Handle returns by algInit.
 *  @param[in]  cmd         	     Cmd for control, i.e. set dynamic params
 *  @param[in]  args                Ptr of args for input or output
 *
 *  @return	common error code defined in common.h
 *
 *  @remarks    Every alg must specify the command andcorresponding data structure
 *
 *  @pre		must call algInit first
 */
    Int32     (*algControl)(Ptr handle, Int32 cmd, Ptr args);

/**
 *  @brief      Apps call this to free resources used by the alg
 *			
 *
 *  @param[in]  handle         Handle returns by algInit.
 *
 *  @return	common error code defined in common.h
 *
 *  @remarks   Once this function is called, the alg can no longer run
 *
 *  @pre		must call algInit first
 */
    Int32     (*algExit)(Ptr handle);
} AlgFxns;

/**
 *  @brief      Commands define for algControl
 */
typedef enum _AlgCmds {
	ALG_CMD_SET_DYN_PARAMS = 0,	/* Set dynamic params, must be supported by alg */
	ALG_CMD_GET_DYN_PARAMS = 1,	/* Get dynamic params, optional */
	ALG_CMD_USER_BASE = 128,		/* Base value for alg defined cmd */
}AlgCmds;

/**
 *  @brief     Init common resources  used
 *			
 *  @remarks   This function Must be called before any other API in this module
 *
 */
Int32 alg_init();

/**
 *  @brief     Free common resources  used
 *			
 *  @remarks   This function Must be called after all alg deleted
 *
 */
void alg_exit();


/**
 *  @brief     Create a top level alg handle
 *			
 *
 *  @param[in]  fxns        	 Functions table for this alg
 *  @param[in]  initParams    Params for create and init
 *
 *  @return	 NULL if failed, otherwise, return top level alg handle 
 *
 *  @remarks   Once this function is called, the alg can no longer run
 *
 */
AlgHandle alg_create(const AlgFxns *fxns, Ptr initParams, Ptr dynParams);

/**
 *  @brief      Delete a top level alg handle
 *
 *  @param[in]  alg         Handle returns by alg_create.
 *
 *  @return	common error code defined in common.h
 *
 *  @pre		must call alg_create first
 */
static inline Int32 alg_delete(AlgHandle alg)
{
	if(!alg || !alg->handle)
		return E_INVAL;
	
	/* Free resources */
	if(alg->fxns->algExit)
		alg->fxns->algExit(alg->handle);

	free(alg);

	return E_NO;
}

/**
 *  @brief     Top level API to run an algorithm
 *
 *  @param[in]  alg             Handle returns by alg_create.
 *  @param[in]  inBuf         	Buffer for input, a structure  can also be used for input
 *  @param[in]  inArgs         Input args, can be set as input buffer size
 *  @param[out] outBuf        Buffer for output, can also use an structure for output
 *  @param[out] outArgs      Output args, can also use an structure for output
 *
 *  @return	common error code defined in common.h
 *
 *  @remarks    alg_process runs the algorithm, every alg must specify the structure and usage
 *   			  of input and output params; 
 *			  If the alg does not need some params, i.e. outArgs, apps can set this 
 *			  param to NULL
 *
 *  @pre		must call alg_create first
 */
static inline Int32 alg_process(AlgHandle alg, AlgBuf *inBuf, Ptr inArgs, AlgBuf *outBuf, Ptr outArgs)
{
	if(!alg|| !alg->handle)
		return E_INVAL;

	/* Call low level function */
	Int32 err = E_UNSUPT;
	if(alg->fxns->algProcess)
		err = alg->fxns->algProcess(alg->handle, inBuf, inArgs, outBuf, outArgs);

	return err;
}

/**
 *  @brief     Top level API to control an algorithm, i.e. set or get dynamic params
 *
 *  @param[in]      alg            Handle returns by alg_create.
 *  @param[in]      cmd           Cmd for control, i.e. set dynamic params
 *  @param[inout]  args          Ptr of args for input or output
 *
 *  @return	common error code defined in common.h
 *
 *  @see also   algControl
 *
 *  @pre		must call alg_create first
 */
 static inline Int32 alg_control(AlgHandle alg, Int32 cmd, Ptr args)
{
	if(!alg|| !alg->handle)
		return E_INVAL;

	/* Call low level function */
	Int32 err = E_UNSUPT;
	if(alg->fxns->algControl)
		err = alg->fxns->algControl(alg->handle, cmd, args);

	return err;
}

/**
 *  @brief     API for set dynamic params
 *
 *  @param[in]      alg           	Handle returns by alg_create.
 *  @param[inout]  dynParams     Ptr of dynamic params
 *
 *  @return	common error code defined in common.h
 *
 *  @see also   algControl
 *
 *  @pre		must call alg_create first
 */
static inline Int32 alg_set_dynamic_params(AlgHandle alg, Ptr dynParams)
{
	return alg_control(alg, ALG_CMD_SET_DYN_PARAMS, dynParams);
}

#endif

