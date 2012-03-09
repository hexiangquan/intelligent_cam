#include <xdc/std.h>
#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/ce/CERuntime.h>
#include <cmem.h>
#include <ti/sdo/fc/rman/rman.h>
#include <ti/sdo/fc/ires/iresman.h>
#include <ti/sdo/fc/ires/memtcm/ires_memtcm.h>
#include <ti/sdo/fc/ires/memtcm/iresman_memtcm.h>


#include "common.h"
#include "alg.h"
#include "log.h"
#include "codecs.h"

Engine_Handle g_hEngine = NULL;

/**
 *  @brief     Init common resources  used
 *			
 *  @remarks   This function Must be called before any other API in this module
 *
 */
Int32 alg_init()
{
	int err;
	
	/* Initialize the codec engine run time */
    CERuntime_init();

	/* Open the codec engine */
    g_hEngine = Engine_open(ENGINE_NAME, NULL, &err);

	if(!g_hEngine) {
		ERR("open engine failed: %d!", err);
		return E_IO;
	}

	/* Initialize Davinci Multimedia Application Interface */
   	err = CMEM_init();
	if(err < 0) {
		ERR("cmem init failed: %d!", err);
		return E_IO;
	}

	IRES_Status status = RMAN_init();
	if(status != IRES_OK) {
		ERR("rman init failed: %d!", err);
		return E_IO;
	}

	/* Register IRES Componets */
	IRESMAN_MemTcmParams memTcmConfigParams;
	memTcmConfigParams.baseConfig.allocFxn = RMAN_PARAMS.allocFxn;
	memTcmConfigParams.baseConfig.freeFxn = RMAN_PARAMS.freeFxn;
	memTcmConfigParams.baseConfig.size = sizeof(IRESMAN_MemTcmParams);
	if(RMAN_register(&IRESMAN_MEMTCM,(IRESMAN_Params *)&memTcmConfigParams) != IRES_OK) {
		ERR("register memtcm failed!");
		return E_IO;
	}

	extern Bool VISA_checked;
	VISA_checked = FALSE;
	return E_NO;
}

/**
 *  @brief     Free common resources  used
 *			
 *  @remarks   This function Must be called after all alg deleted
 *
 */
void alg_exit()
{
	if(!g_hEngine)
		Engine_close(g_hEngine);
}

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
AlgHandle alg_create(const AlgFxns *fxns, Ptr initParams, Ptr dynParams)
{
	if(!fxns || !fxns->algInit)
		return NULL;

	/* Alloc memory for alg obj */
	AlgHandle alg = malloc(sizeof(AlgObj));
	if(alg) {

		/* Create and init specific alg handle */
		alg->handle = fxns->algInit(initParams, dynParams);
		if(!alg->handle) {
			free(alg);
			return NULL;
		}
		
		/* Record functions */
		alg->fxns = fxns;
		alg->cfg = (Ptr)g_hEngine;
	}

	return alg;
}


