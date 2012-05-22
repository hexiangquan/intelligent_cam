#include "common.h"
#include "log.h"

const char *str_err(int err)
{
	switch(err){
	case E_NO:
		return "No error";
	case E_INVAL:
		return "Invalid arg";
	case E_BUSY:
		return "Device or Resource is busy";
	case E_TIMEOUT:
		return  "Operation timeout";
	case E_IO:
		return  "IO operation error";
	case E_NOMEM:
		return  "Alloc memory failed";
	case E_NOSPC:
		return  "No space left on the device";
	case E_CHECKSUM:
		return "Checksum failed";
	case E_CONNECT:
		return "Connect failed";
	case E_TRANS:
		return "Data transfer error";
	case E_REFUSED:
		return "Operation refused";
	case E_INVPATH:
		return "Invalid path";
	case E_INVNAME:
		return "Invalid name";
	case E_MODE:
		return "Wrong mode";
	case E_NOTEXIST:
		return "Not exist";
	case E_INVUSER:
		return "Invalid user";
	case E_INVPASSWD:
		return "Invalid password";
	case E_AGAIN:
		return "Try again";
	case E_UNSUPT:
		return "Operation not supported";
	default:
		return  "Unkown error";
	}
}

