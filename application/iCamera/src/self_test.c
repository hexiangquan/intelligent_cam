#include "self_test.h"
#include "detector.h"
#include "log.h"
#include "detector_uart_generic.h"
#include "alg.h"
#include "buffer.h"
#include "path_name.h"
#include "ftp_upload.h"

/* test vehicle detectors */
extern Int32 detector_test();

/* test path name */
extern Int32 path_name_test();

/* test ftp upload */
extern Int32 ftp_upload_test(const char *username, const char *passwd, const char *srvip, Uint16 port);

/* test local upload */
extern Int32 local_upload_test(const char *pathName);

/* test rtp upload */
extern Int32 rtp_upload_test();

/* test sys commu data transfer */
extern Int32 sys_commu_test();

/* test day night switch */
extern Int32 day_night_test();

/*****************************************************************************
 Prototype    : self_test
 Description  : self test module
 Input        : Int32 flags  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2012/5/21
    Author       : Sun
    Modification : Created function

*****************************************************************************/
Int32 self_test(Int32 flags)
{
	Int32 err;

	err = alg_init();
	err |= buffer_init();

	assert(err == E_NO);
	
	/* do auto self test of app modules */
	if(flags & SELF_TEST_DETECTOR)
		err = detector_test();

	if(flags & SELF_TEST_PATHNAME)
		err = path_name_test();

	if(flags & SELF_TEST_FTPUPLOAD)
		err = ftp_upload_test("test", "123456", "192.168.0.15", 21);

	if(flags & SELF_TEST_LOCALSND)
		err = local_upload_test("/media/mmcblk0p1");

	if(flags & SELF_TEST_RTPUPLOAD)
		err = rtp_upload_test();

	if(flags & SELF_TEST_SYSCOMMU)
		err = sys_commu_test();

	if(flags & SELF_TEST_DAYNIGHT)
		err = day_night_test();
	
	INFO("self test pass...\n");

	alg_exit();	
	buffer_exit();
	
	return err;
}

