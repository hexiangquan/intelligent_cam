#include "common.h"
#include "log.h"

const char *str_err(int err)
{

	char *pErrStr = NULL;

	DBG("Enter %s", __func__);

	switch(err){
		case E_NO:
			return "No error";
			break;
		case E_INVAL:
			return "Invalid arg";
			break;
		case E_BUSY:
			return "Device or Resource is busy";
			//break;
		case E_TIMEOUT:
			return  "Operation timeout";
			break;
		case E_IO:
			return  "IO operation error";
			break;
		case E_NOMEM:
			return  "Alloc memory failed";
			break;
		case E_NOSPC:
			return  "No space left on the device";
			break;
		default:
			return  "";
			break;
	}

	DBG("Leave %s", __func__);
	return pErrStr;
}

