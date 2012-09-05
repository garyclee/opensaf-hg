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

  DESCRIPTION:

  This module contains ?

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
uint32_t decode_32bitOS_inc(uint8_t **stream)
{

	uint32_t val = 0;		/* Accumulator */
	val = m_NCS_OS_NTOHL_P(*stream);
	*stream += sizeof(uint32_t);
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

uint32_t encode_32bitOS_inc(uint8_t **stream, uint32_t val)
{
	m_NCS_OS_HTONL_P(*stream, val);
	*stream += sizeof(uint32_t);
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

uint32_t encode_16bitOS_inc(uint8_t **stream, uint32_t val)
{
	m_NCS_OS_HTONS_P(*stream, val);
	*stream += sizeof(uint16_t);
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
uint16_t decode_16bitOS_inc(uint8_t **stream)
{

	uint32_t val = 0;		/* Accumulator */
	val = m_NCS_OS_NTOHS_P(*stream);
	*stream += sizeof(uint16_t);
	return (uint16_t)(val & 0x0000FFFF);

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

static uint32_t leap_env_init_count = 0;

uint32_t leap_env_init(void)
{
	if (leap_env_init_count++ != 0) {
		return NCSCC_RC_SUCCESS;
	}

	TRACE("INITIALIZING LEAP ENVIRONMENT");

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
	TRACE("DONE INITIALIZING LEAP ENVIRONMENT");

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

uint32_t leap_env_destroy()
{

	if (--leap_env_init_count != 0) {
		return NCSCC_RC_SUCCESS;
	}

	TRACE("DESTROYING LEAP ENVIRONMENT");

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

	TRACE("DONE DESTROYING LEAP ENVIRONMENT");

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

/**
 * syslog and abort to generate a core dump (if enabled)
 * @param __file
 * @param __line
 * @param __func
 * @param __assertion
 */
void __osafassert_fail(const char *__file, int __line, const char* __func, const char *__assertion)
{
	syslog(LOG_ERR, "%s:%d: %s: Assertion '%s' failed.", __file, __line, __func, __assertion);
	abort();
}

