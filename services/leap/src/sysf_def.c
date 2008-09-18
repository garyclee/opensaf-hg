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


#include "ncs_opt.h"
#include "gl_defs.h"
#include "ncs_osprm.h"
#include "ncs_scktprm.h"

#include "ncs_svd.h"
#include "ncssysf_def.h"
#include "ncssysf_tmr.h"
#include "sysf_def.h"
#include "sysf_lck.h"
#include "ncssysfpool.h"
#include "usrbuf.h"
#include "sysf_exc_scr.h"

#if (NCS_USE_SYSMON == 1)
#if (NCSL_ENV_INIT_LBP == 1)
#include "ncs_sysm.h"
#endif
#endif
#if (NCSL_ENV_INIT_KMS == 1)
#include "ncs_kms.h"
#endif
#if (NCSL_ENV_INIT_MTM == 1)
#include "ncs_mib.h"
#endif

#include "ncs_mib_pub.h"
#include "ncsmiblib.h"

time_t
ncs_time_stamp()
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
uns32
decode_32bitOS_inc( uns8 **stream)
{

  uns32 val = 0;  /* Accumulator */
  val       = m_NCS_OS_NTOHL_P(*stream);
  *stream   += sizeof(uns32);
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

uns32
encode_32bitOS_inc( uns8 **stream, uns32 val)
{
  m_NCS_OS_HTONL_P(*stream,val);
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

uns32
encode_16bitOS_inc( uns8 **stream, uns32 val)
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
uns16
decode_16bitOS_inc( uns8 **stream)
{

  uns32 val = 0;  /* Accumulator */
  val = m_NCS_OS_NTOHS_P(*stream);
  *stream += sizeof(uns16); 
  return (uns16)(val & 0x0000FFFF);

}



/*****************************************************************************

  PROCEDURE NAME:    leap_dbg_sink

  DESCRIPTION:

   If the ENABLE_LEAP_DBG flag is enabled (ncs_opt.h) then LEAP code that
   has been instrumented to seek runtime errors will invoke this macro.
   If the flag is disabled, LEAP debug code is benign and not in the built
   image.

   The customer is invited to redefine the macro or re-populate the function 
   here.  This sample function is invoked by the macro m_LEAP_DBG_SINK.
   
  ARGUMENTS:

  uns32   l             line # in file
  char*   f             file name where macro invoked
  code    code          Error code value.. Usually FAILURE

  RETURNS:

  code

  NOTES:

*****************************************************************************/


uns32 leap_dbg_sink(uns32 l, char* f, long code)
{
  /*IR00061064*/ 
#if (ENABLE_LEAP_DBG == 1)
  switch(code)
   {
   case NCSCC_RC_NO_TO_SVC:
      m_NCS_CONS_PRINTF ("MDS: Destination service is not UP: line %d, file %s\n", (unsigned int) l,f);
      break;
   default:
      m_NCS_CONS_PRINTF ("IN LEAP_DBG_SINK: line %d, file %s\n", (unsigned int) l,f);
      break;
   }
#endif
  return code;
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

char* gl_fail_str[] = 
  {
  "illegal string",              /* You shouldn't see this one */
  "Double Delete of Memory",
  "Freeing Null Pointer",
  "Memory Is Screwed Up!?",
  "Failed Bottom Marker Test",
  "Freeing to a NULL pool",
  "Memory DBG Record corrupted",
  "Memory ownership rules violated"
  };

/* The function that says it all */

uns32 leap_failure(uns32 l, char* f, uns32 err, uns32 retval)
  {
  switch(err)
    {
    case NCSFAIL_DOUBLE_DELETE     :
    case NCSFAIL_FREEING_NULL_PTR  :
    case NCSFAIL_MEM_SCREWED_UP    :
    case NCSFAIL_FAILED_BM_TEST    :
    case NCSFAIL_MEM_NULL_POOL     :
    case NCSFAIL_MEM_REC_CORRUPTED :
    case NCSFAIL_OWNER_CONFLICT    :

#if (NCS_MMGR_ERROR_BEHAVIOR == BANNER_ERR) || (NCS_MMGR_ERROR_BEHAVIOR == CRASH_ERR)        

      m_NCS_CONS_PRINTF ("MEMORY FAILURE: %s line %d, file %s\n", gl_fail_str[err],l,f);

#if (NCS_MMGR_ERROR_BEHAVIOR == CRASH_ERR)

      m_NCS_OS_ASSERT(0);  /* Policy is to crash! */

#endif /* CRASH */
#endif /* BANNER or CRASH */

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

static  uns32 leap_env_init_count = 0;

uns32 leap_env_init()
{

  /* IR00061488 */
  if(leap_env_init_count++ != 0)
  {   
    return NCSCC_RC_SUCCESS;
  }  

  m_NCS_DBG_PRINTF("\n\n\nINITIALIZING LEAP ENVIRONMENT\n");

  /* initialize OS target */
  m_NCS_OS_TARGET_INIT;

#if (NCSL_ENV_INIT_MMGR == 1)
  /* initialize LEAP Memory Manager */
  if(ncs_mem_create() != NCSCC_RC_SUCCESS)
  {
    m_NCS_CONS_PRINTF("\nleap_env_init: FAILED to initialize Memory Manager\n");
    return NCSCC_RC_FAILURE;
  }
#endif

#if (NCS_USE_SYSMON == 1)
#if (NCSL_ENV_INIT_LBP == 1)
  /* initialize LEAP Buffer Pool */
  if(ncs_lbp_create() != NCSCC_RC_SUCCESS)
  {
    m_NCS_CONS_PRINTF("\nleap_env_init: FAILED to initialize Buffer Pool\n");

#if (NCSL_ENV_INIT_MMGR == 1)
    (void)ncs_mem_destroy();
#endif
    return NCSCC_RC_FAILURE;
  }
#endif /* #if (NCSL_ENV_INIT_LBP == 1) */
#endif

#if (NCSL_ENV_INIT_LM == 1)
  /* initialize LEAP Lock Manager */
  if(ncs_lock_create_mngr() != NCSCC_RC_SUCCESS)
  {
    m_NCS_CONS_PRINTF("\nleap_env_init: FAILED to initialize Lock Manager\n");

#if (NCS_USE_SYSMON == 1)
#if (NCSL_ENV_INIT_LBP == 1)
    (void)ncs_lbp_destroy();
#endif
#endif
#if (NCSL_ENV_INIT_MMGR == 1)
    (void)ncs_mem_destroy();
#endif

    return NCSCC_RC_FAILURE;
  }
#endif /* #if (NCSL_ENV_INIT_LM == 1) */

  /*IR00061064*/
  ncs_os_atomic_init();

#if (NCSL_ENV_INIT_TMR == 1)
  /* initialize LEAP Timer Service */
  if(sysfTmrCreate() != NCSCC_RC_SUCCESS)
  {
    m_NCS_CONS_PRINTF("\nleap_env_init: FAILED to initialize Timer Service\n");

#if (NCSL_ENV_INIT_LM == 1)
    (void)ncs_lock_destroy_mngr();
#endif
#if (NCS_USE_SYSMON == 1)
#if (NCSL_ENV_INIT_LBP == 1)
    (void)ncs_lbp_destroy();
#endif
#endif
#if (NCSL_ENV_INIT_MMGR == 1)
    (void)ncs_mem_destroy();
#endif

    return NCSCC_RC_FAILURE;
  }
#endif /* #if (NCSL_ENV_INIT_TMR == 1) */

#if (NCSL_ENV_INIT_KMS == 1)
  /* initialize KMS */
  {
    NCSKMS_LM_ARG kms_arg;
    kms_arg.i_op = NCSKMS_LM_CREATE;
    if(ncskms_lm(&kms_arg) != NCSCC_RC_SUCCESS)
    {
      m_NCS_CONS_PRINTF("\nleap_env_init: FAILED to initialize KMS\n");

#if (NCSL_ENV_INIT_TMR == 1)
     (void)sysfTmrDestroy();
#endif
#if (NCSL_ENV_INIT_LM == 1)
    (void)ncs_lock_destroy_mngr();
#endif
#if (NCS_USE_SYSMON == 1)
#if (NCSL_ENV_INIT_LBP == 1)
    (void)ncs_lbp_destroy();
#endif
#endif
#if (NCSL_ENV_INIT_MMGR == 1)
    (void)ncs_mem_destroy();
#endif
      return NCSCC_RC_FAILURE;
    }
  }
#endif /* #if (NCSL_ENV_INIT_KMS == 1) */

#if (NCSL_ENV_INIT_MTM == 1)
  /* initialize MIB Transaction Manager: 
     needed for NCS_MIB code...
  */
  (void)ncsmib_tm_create();
#endif /* #if (NCSL_ENV_INIT_MTM == 1) */

#if (NCSL_ENV_INIT_HM == 1)
  /* initialize Handle Manager */
  if(ncshm_init() != NCSCC_RC_SUCCESS)
  {
    m_NCS_CONS_PRINTF("\nleap_env_init: FAILED to initialize Handle Manager\n");

#if (NCSL_ENV_INIT_MTM == 1)
    (void)ncsmib_tm_destroy();
#endif
#if (NCSL_ENV_INIT_KMS == 1)
    {
    NCSKMS_LM_ARG kms_arg;
    kms_arg.i_op = NCSKMS_LM_DESTROY;
    (void)ncskms_lm(&kms_arg);
    }
#endif
#if (NCSL_ENV_INIT_TMR == 1)
     (void)sysfTmrDestroy();
#endif
#if (NCSL_ENV_INIT_LM == 1)
    (void)ncs_lock_destroy_mngr();
#endif
#if (NCS_USE_SYSMON == 1)
#if (NCSL_ENV_INIT_LBP == 1)
    (void)ncs_lbp_destroy();
#endif
#endif
#if (NCSL_ENV_INIT_MMGR == 1)
    (void)ncs_mem_destroy();
#endif

    return NCSCC_RC_FAILURE;
  }
#endif /* #if (NCSL_ENV_INIT_HM == 1) */

#if (NCSL_ENV_INIT_SMON == 1) 
  /* initialize SYSMON */
#if (NCS_USE_SYSMON == 1)
  {
    NCSSYSM_LM_ARG sm_arg;
    sm_arg.i_op = NCSSYSM_LM_OP_INIT;
    sm_arg.i_vrtr_id = 1;
    if(ncssysm_lm(&sm_arg) != NCSCC_RC_SUCCESS)
    {    
      m_NCS_CONS_PRINTF("\nleap_env_init: FAILED to initialize SYSMON\n");

#if (NCSL_ENV_INIT_HM == 1)      
    (void)ncshm_delete();
#endif
#if (NCSL_ENV_INIT_MTM == 1)
    (void)ncsmib_tm_destroy();
#endif
#if (NCSL_ENV_INIT_KMS == 1)
    {
    NCSKMS_LM_ARG kms_arg;
    kms_arg.i_op = NCSKMS_LM_DESTROY;
    (void)ncskms_lm(&kms_arg);
    }
#endif
#if (NCSL_ENV_INIT_TMR == 1)
     (void)sysfTmrDestroy();
#endif
#if (NCSL_ENV_INIT_LM == 1)
    (void)ncs_lock_destroy_mngr();
#endif
#if (NCS_USE_SYSMON == 1)
#if (NCSL_ENV_INIT_LBP == 1)
    (void)ncs_lbp_destroy();
#endif
#endif
#if (NCSL_ENV_INIT_MMGR == 1)
    (void)ncs_mem_destroy();
#endif
      return NCSCC_RC_FAILURE;
    }  
  }
#endif /* #if (NCS_USE_SYSMON == 1) */
#endif /* #if (NCSL_ENV_INIT_SMON == 1)  */

  /* NCS_IPC implementation uses INET sockets on Windows. INET sockets
  need WSAStartup to done before NCS-IPC creation. - Phani:10/02/04*/
  if (m_NCSSOCK_CREATE != NCSCC_RC_SUCCESS)
  {
     return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
  }

  /* NCS MIBLIB is being initialized here. */
  {
      NCSMIBLIB_REQ_INFO  miblib_init;
      uns32 status;

      /* Initalize miblib */
      m_NCS_MEMSET(&miblib_init, 0, sizeof(NCSMIBLIB_REQ_INFO));

      /* register with MIBLIB */
      miblib_init.req = NCSMIBLIB_REQ_INIT_OP;
      status = ncsmiblib_process_req(&miblib_init);
      if (status != NCSCC_RC_SUCCESS)
      {
         /* Log error */

         m_NCS_CONS_PRINTF("\nleap_env_init: FAILED to initialize MIBLIB\n");
           
         return m_LEAP_DBG_SINK(status);
      }

  }

  /* Initialize script execution control block */
  if (NCSCC_RC_SUCCESS != init_exec_mod_cb())
  {
     /* Log error */
     m_NCS_CONS_PRINTF("\nleap_env_init: FAILED to initialize Execute Module CB \n");
 
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
  
  /*IR00061488 */ 
  if(--leap_env_init_count != 0)
  {   
     return NCSCC_RC_SUCCESS;
  }
 
  
  m_NCS_DBG_PRINTF("\n\n\nDESTROYING LEAP ENVIRONMENT\n");

  m_NCSSOCK_DESTROY;

  /* IR00060372 */
  /* Destroying  execution control block */
  exec_mod_cb_destroy();
  

#if (NCSL_ENV_INIT_SMON == 1) 
#if (NCS_USE_SYSMON == 1)
  /* destroying SYSMON */
  {
    NCSSYSM_LM_ARG sm_arg;
    sm_arg.i_op = NCSSYSM_LM_OP_INIT;
    sm_arg.i_vrtr_id = 1;


    (void)ncssysm_lm(&sm_arg);
  }
#endif   
#endif  

#if (NCSL_ENV_INIT_HM == 1)  
   /* destroying Handle Manager */ 
    (void)ncshm_delete();
#endif

#if (NCSL_ENV_INIT_MTM == 1)
  /* destroying MIB Transaction Manager */
    (void)ncsmib_tm_destroy();
#endif

#if (NCSL_ENV_INIT_KMS == 1)
  /* destroying KMS */
    {
    NCSKMS_LM_ARG kms_arg;
    kms_arg.i_op = NCSKMS_LM_DESTROY;
    (void)ncskms_lm(&kms_arg);
    }
#endif

#if (NCSL_ENV_INIT_TMR == 1)
  /* destroying LEAP Timer Service */
     (void)sysfTmrDestroy();
#endif

  /*IR00061064*/
  ncs_os_atomic_destroy();

#if (NCSL_ENV_INIT_LM == 1)
  /* destroying Lock Manager */
    (void)ncs_lock_destroy_mngr();
#endif

#if (NCS_USE_SYSMON == 1)
#if (NCSL_ENV_INIT_LBP == 1)
  /* destroying LEAP Buffer Pool */
    (void)ncs_lbp_destroy();
#endif
#endif

#if (NCSL_ENV_INIT_MMGR == 1)
  /* destroying LEAP Memory Manager */
    (void)ncs_mem_destroy();
#endif

  m_NCS_DBG_PRINTF("\nDONE DESTROYING LEAP ENVIRONMENT\n");

  return NCSCC_RC_SUCCESS;
}


/**
 * Print message and reboot the system
 * @param reason
 */
void ncs_reboot(const char *reason)
{
   struct timeval tv;
   char time_str[128];
 
   gettimeofday(&tv, NULL);
   strftime(time_str, sizeof(time_str), "%b %e %k:%M:%S", localtime(&tv.tv_sec));
   fprintf(stderr, "%s node rebooting, reason: %s\n", time_str, reason);
   syslog(LOG_CRIT, "node rebooting, reason: %s", reason);

   system("/etc/opt/opensaf/reboot");
}
 

