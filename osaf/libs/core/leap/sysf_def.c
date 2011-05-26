/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 * Author(s): Emerson Network Power
 *
 */

/*****************************************************************************
..............................................................................

..............................................................................

  DESCRIPTION:

  This module contains routines related to time stamps.
..............................................................................

  FUNCTIONS INCLUDED in this module:

  ncs_time_stamp....get the time stamp

 ******************************************************************************
 */

#include <configmake.h>

#include <ncsgl_defs.h>
#include "ncs_osprm.h"

#include "ncs_svd.h"
#include "ncssysf_def.h"
#include "ncssysf_tmr.h"
#include "sysf_def.h"
#include "ncssysf_lck.h"
#include "ncssysfpool.h"
#include "sysf_exc_scr.h"
#include "usrbuf.h"

time_t ncs_time_stamp()
{
	time_t timestamp;

	m_GET_TIME_STAMP(timestamp);

	return timestamp;
}

/*****************************************************************************

  PROCEDURE NAME:    decode_32bitOS_inc

  DESCRIPTION:

  Decode a 4-octet *NON-0/1-EXT-ENCODED* field.
  Using standard operating system defines.
  ARGUMENTS:

 stream:  Address of a pointer to streamed data.

  RETURNS:

  NOTES:

  "stream" points to the start of 4-octet unsigned short in the network order.
  Network-order is the same as "big-endian" order (MSB first).
  This function has a built-in network-to-host order effect.It also automatically
  increments the pointer to the datastore

*****************************************************************************/
uns32 decode_32bitOS_inc(uns8 **stream)
{

	uns32 val = 0;		/* Accumulator */
	val = m_NCS_OS_NTOHL_P(*stream);
	*stream += sizeof(uns32);
	return val;

}

/*****************************************************************************

  PROCEDURE NAME:    encode_32bitOS_inc

  DESCRIPTION:

  Encode a 4-octet *NON-0/1-EXT-ENCODED* field.
  Using standard operating system defines.
  ARGUMENTS:

 stream:  Address of a pointer to streamed data.

  RETURNS:

  NOTES:

  "stream" points to the start of 4-octet unsigned long  in the network order.
  Network-order is the same as "big-endian" order (MSB first).
  This function has a built-in network-to-host order effect. It also automatically
  increments the pointer to the datastore
*****************************************************************************/

uns32 encode_32bitOS_inc(uns8 **stream, uns32 val)
{
	m_NCS_OS_HTONL_P(*stream, val);
	*stream += sizeof(uns32);
	return 4;
}

/*****************************************************************************

  PROCEDURE NAME:    encode_16bitOS_inc

  DESCRIPTION:

  Encode a 2-octet *NON-0/1-EXT-ENCODED* field.
  Using standard operating system defines.
  ARGUMENTS:

 stream:  Address of a pointer to streamed data.

  RETURNS:

  NOTES:

  "stream" points to the start of 2-octet unsigned short in the network order.
  Network-order is the same as "big-endian" order (MSB first).
  This function has a built-in network-to-host order effect.It also automatically
  increments the pointer to the datastore

*****************************************************************************/

uns32 encode_16bitOS_inc(uns8 **stream, uns32 val)
{
	m_NCS_OS_HTONS_P(*stream, val);
	*stream += sizeof(uns16);
	return 2;
}

/*****************************************************************************

  PROCEDURE NAME:    decode_16bitOS_inc

  DESCRIPTION:

  Decode a 2-octet *NON-0/1-EXT-ENCODED* field.
  Using standard operating system defines.
  ARGUMENTS:

 stream:  Address of a pointer to streamed data.

  RETURNS:

  NOTES:

  "stream" points to the start of 2-octet unsigned short in the network order.
  Network-order is the same as "big-endian" order (MSB first).
  This function has a built-in network-to-host order effect.It also automatically
  increments the pointer to the datastore

*****************************************************************************/
uns16 decode_16bitOS_inc(uns8 **stream)
{

	uns32 val = 0;		/* Accumulator */
	val = m_NCS_OS_NTOHS_P(*stream);
	*stream += sizeof(uns16);
	return (uns16)(val & 0x0000FFFF);

}

/*****************************************************************************

  PROCEDURE NAME:    leap_failure

  DESCRIPTION:

   LEAP code has detected a condition that it deems wrong. No doubt there is
   a bug to be found and cleaned up.
   
   Policies can be adjusted for each flavor of error, but so far, all such
   errors are worthy of a crash according to NetPlane.
      
  ARGUMENTS:

  uns32   l             line # in file
  char*   f             file name where macro invoked
  uns32   err           Error code value..
  uns32   retval        return this to invoker, if we servive

  RETURNS:

  code

  NOTES:

*****************************************************************************/

/* the array of error strings */

char *gl_fail_str[] = {
	"illegal string",	/* You shouldn't see this one */
	"Double Delete of Memory",
	"Freeing Null Pointer",
	"Memory Is Screwed Up!?",
	"Failed Bottom Marker Test",
	"Freeing to a NULL pool",
	"Memory DBG Record corrupted",
	"Memory ownership rules violated"
};

/* The function that says it all */

uns32 leap_failure(uns32 l, char *f, uns32 err, uns32 retval)
{
	switch (err) {
	case NCSFAIL_DOUBLE_DELETE:
	case NCSFAIL_FREEING_NULL_PTR:
	case NCSFAIL_MEM_SCREWED_UP:
	case NCSFAIL_FAILED_BM_TEST:
	case NCSFAIL_MEM_NULL_POOL:
	case NCSFAIL_MEM_REC_CORRUPTED:
	case NCSFAIL_OWNER_CONFLICT:

#if (NCS_MMGR_ERROR_BEHAVIOR == BANNER_ERR) || (NCS_MMGR_ERROR_BEHAVIOR == CRASH_ERR)

		printf("MEMORY FAILURE: %s line %d, file %s\n", gl_fail_str[err], l, f);

#if (NCS_MMGR_ERROR_BEHAVIOR == CRASH_ERR)

		assert(0);	/* Policy is to crash! */
#endif   /* CRASH */
#endif   /* BANNER or CRASH */

		break;
	}
	return retval;
}

/*****************************************************************************

  PROCEDURE NAME:    leap_env_init

  DESCRIPTION:

   Initialize LEAP services
      
  ARGUMENTS:

  RETURNS:

  code

  NOTES:

*****************************************************************************/

static uns32 leap_env_init_count = 0;

uns32 leap_env_init()
{

	if (leap_env_init_count++ != 0) {
		return NCSCC_RC_SUCCESS;
	}

	/* 
	 ** Change buffering type for the stdout stream to line buffered.
	 ** Otherwise printf output will be block buffered and not immidiately
	 ** printed to file.
	 ** TODO: to be removed in OpenSAF 4.0
	 */
	if (setvbuf(stdout, (char *)NULL, _IOLBF, 0) != 0) {
		fprintf(stderr, "%s:%d - setvbuf failed\n", __FILE__, __LINE__);
		return NCSCC_RC_FAILURE;
	}

	m_NCS_DBG_PRINTF("\n\n\nINITIALIZING LEAP ENVIRONMENT\n");

	/* initialize OS target */
	m_NCS_OS_TARGET_INIT;
	ncs_os_atomic_init();

#if (NCSL_ENV_INIT_TMR == 1)
	/* initialize LEAP Timer Service */
	if (sysfTmrCreate() != NCSCC_RC_SUCCESS) {
		printf("\nleap_env_init: FAILED to initialize Timer Service\n");
		return NCSCC_RC_FAILURE;
	}
#endif   /* #if (NCSL_ENV_INIT_TMR == 1) */

#if (NCSL_ENV_INIT_HM == 1)
	/* initialize Handle Manager */
	if (ncshm_init() != NCSCC_RC_SUCCESS) {
		printf("\nleap_env_init: FAILED to initialize Handle Manager\n");

#if (NCSL_ENV_INIT_TMR == 1)
		(void)sysfTmrDestroy();
#endif
		return NCSCC_RC_FAILURE;
	}
#endif   /* #if (NCSL_ENV_INIT_HM == 1) */

	/* Initialize script execution control block */
	if (NCSCC_RC_SUCCESS != init_exec_mod_cb()) {
		/* Log error */
		printf("\nleap_env_init: FAILED to initialize Execute Module CB \n");

		return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
	}
	m_NCS_DBG_PRINTF("\nDONE INITIALIZING LEAP ENVIRONMENT\n");

	return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    leap_env_destroy

  DESCRIPTION:

   Destroy LEAP services
      
  ARGUMENTS:

  RETURNS:

  code

  NOTES:

*****************************************************************************/

uns32 leap_env_destroy()
{

	if (--leap_env_init_count != 0) {
		return NCSCC_RC_SUCCESS;
	}

	m_NCS_DBG_PRINTF("\n\n\nDESTROYING LEAP ENVIRONMENT\n");

	/* Destroying  execution control block */
	exec_mod_cb_destroy();

#if (NCSL_ENV_INIT_HM == 1)
	/* destroying Handle Manager */
	(void)ncshm_delete();
#endif

#if (NCSL_ENV_INIT_TMR == 1)
	/* destroying LEAP Timer Service */
	(void)sysfTmrDestroy();
#endif

	ncs_os_atomic_destroy();

	m_NCS_DBG_PRINTF("\nDONE DESTROYING LEAP ENVIRONMENT\n");

	return NCSCC_RC_SUCCESS;
}

/**
 * 
 * @param reason
 */
void opensaf_reboot(unsigned int node_id, char *ee_name, const char *reason) 
{

	char str[256];
	memset(str,0,256);

	snprintf(str,255,PKGLIBDIR"/opensaf_reboot %d %s\n",node_id,((ee_name == NULL)?"":ee_name));
	syslog(LOG_CRIT,"Rebooting OpenSAF NodeId = %d EE Name = %s, Reason: %s\n",node_id,((ee_name == NULL)? "No EE Mapped":ee_name),reason);
	if(system(str) == -1){
        	syslog(LOG_CRIT, "node reboot failure!");
	}
}

