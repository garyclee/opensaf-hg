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

  This module contains target system specific declarations related to
  LOCK Management.

..............................................................................
*/

/** Get compile time options...
 **/

#include "ncs_opt.h"
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "sysf_lck.h"
#include "ncssysf_def.h"


#if (NCSSYSM_LOCK_DBG_ENABLE != 0 )

static NCS_LOCK lock_mngr;
static int     lock_mngr_active = FALSE;
uns32 init_lock_count[NCS_SERVICE_ID_MAX];
uns32 destroy_lock_count[NCS_SERVICE_ID_MAX];

#endif

/****************************************************************************
 *
 * Function Name: ncs_lock_create_mngr
 *
 * Purpose:       Put the lock manager into start state
 *                NOP if NCSSYSM_LOCK_DBG_ENABLE is not set
 *
 *
 ****************************************************************************/

unsigned int  ncs_lock_create_mngr(void) {


#if (NCSSYSM_LOCK_DBG_ENABLE != 0 )
   unsigned int retval;

   lock_mngr_active = FALSE;
   retval = m_NCS_LOCK_INIT_V2(&lock_mngr, NCS_SERVICE_ID_OS_SVCS, 0);
   if(retval == NCSCC_RC_FAILURE) {
      return retval;
   }
   lock_mngr_active = TRUE;
#endif
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 *
 * Function Name: ncs_lock_destroy_mngr
 *
 * Purpose:       Put the lock manager into start stop
 *                Free any resources allocated by ncs_lock_create_mngr
 *
 *
 ****************************************************************************/
unsigned int ncs_lock_destroy_mngr(void) {

#if (NCSSYSM_LOCK_DBG_ENABLE != 0 )
   m_NCS_LOCK_DESTROY_V2(&lock_mngr, NCS_SERVICE_ID_OS_SVCS, 0);
   lock_mngr_active = FALSE;
#endif

   return NCSCC_RC_SUCCESS;
}



#if (NCSSYSM_LOCK_DBG_ENABLE != 0 )

/****************************************************************************
 *
 * Function Name: ncs_lock_init
 *
 * Purpose:       Initialize lock via OS Prims m_NCS_OS_LOCK
 *                Record statistics about number of locks initialized
 *
 * Note:          If both service_id and local_id are 0, the macro m_NCS_LOCK_INIT
 *                was used.  Otherwise, the macro m_NCS_LOCK_INIT_V2 was used.
 *                This is possible because m_NCS_LOCK_INIT_V2 does not use
 *                service_id 0.
 *
 ****************************************************************************/
unsigned int ncs_lock_init(NCS_LOCK * lock,
                          NCS_SERVICE_ID service_id,
                          unsigned int  local_id,
                          unsigned int  line,
                          char*         file)
{

   unsigned int retval;
   retval = m_NCS_OS_LOCK(&lock->lock,NCS_OS_LOCK_CREATE,0);
   if (retval == NCSCC_RC_SUCCESS)
     lock->exists = NCS_LOCK_EXISTS;
   else
     return m_LEAP_GOTO_DBG_SINK(line,file,NCSCC_RC_FAILURE);

   /* The lock_mngr is a special lock, so avoid processing it
   */
   if(lock != &lock_mngr && lock_mngr_active ) {
      if(retval == NCSCC_RC_SUCCESS) {
         if(service_id < NCS_SERVICE_ID_MAX) {
            m_NCS_LOCK_V2(&lock_mngr, NCS_LOCK_WRITE, NCS_SERVICE_ID_OS_SVCS, 0);
            ++init_lock_count[service_id];
            /* Add lock address to linked list
            */
            m_NCS_UNLOCK_V2(&lock_mngr, NCS_LOCK_WRITE, NCS_SERVICE_ID_OS_SVCS, 0);
         }
      }
   }
   return retval;
}

/****************************************************************************
 *
 * Function Name: ncs_lock_destroy
 *
 * Purpose:       Destroy a lock via OS Prims m_NCS_OS_LOCK
 *                Record statistics about number of locks destroyed
 *
 * Note:          If both service_id and local_id are 0, the macro m_NCS_LOCK_DESTROY
 *                was used.  Otherwise, the macro m_NCS_LOCK_DESTROY_V2 was used.
 *                This is possible because m_NCS_LOCK_DESTROY_V2 does not use
 *                service_id 0.
 *
 ****************************************************************************/
unsigned int ncs_lock_destroy(NCS_LOCK * lock,
                             NCS_SERVICE_ID service_id,
                             unsigned int  local_id,
                             unsigned int  line,
                             char*         file)
{

   unsigned int retval;

   if (lock->exists != NCS_LOCK_EXISTS)
    {
    m_NCS_OS_ASSERT(0);
    m_LEAP_DBG_SINK(NCSCC_RC_FAILURE); /* so we know its this LOCK failure */
    return m_LEAP_GOTO_DBG_SINK(line,file,NCSCC_RC_FAILURE);
    }

   retval =  m_NCS_OS_LOCK(&lock->lock,NCS_OS_LOCK_RELEASE,0);
   if (retval != NCSCC_RC_SUCCESS)
     {
     m_LEAP_DBG_SINK(NCSCC_RC_FAILURE); /* so we know its this LOCK failure */
     m_LEAP_GOTO_DBG_SINK(line,file,NCSCC_RC_FAILURE);
     }

   lock->exists = 0; /* No matter what, get rid of EXISTS pattern */

   /* The lock_mngr is a special lock, so avoid processing it
   */
   if(lock != &lock_mngr && lock_mngr_active ) {
      if(retval == NCSCC_RC_SUCCESS) {
         if(service_id < NCS_SERVICE_ID_MAX) {
            m_NCS_LOCK_V2(&lock_mngr, NCS_LOCK_WRITE, NCS_SERVICE_ID_OS_SVCS, 0);
            ++destroy_lock_count[service_id];
            /* remove lock address to linked list
            */
            m_NCS_UNLOCK_V2(&lock_mngr, NCS_LOCK_WRITE, NCS_SERVICE_ID_OS_SVCS, 0);
         }
      }
   }
   return retval;
}

/****************************************************************************
 *
 * Function Name: ncs_lock
 *
 * Purpose:       Lock a lock via OS PRIMS. Confirm that the lock exists. 
 *                Check return code and complain if it returns FAILURE.
 *
 * Note:          If both service_id and local_id are 0, the macro m_NCS_LOCK_DESTROY
 *                was used.  Otherwise, the macro m_NCS_LOCK_DESTROY_V2 was used.
 *                This is possible because m_NCS_LOCK_DESTROY_V2 does not use
 *                service_id 0.
 *
 ****************************************************************************/

unsigned int  ncs_lock  (NCS_LOCK*      lock,
                        unsigned int  flag,
                        NCS_SERVICE_ID service_id,
                        unsigned int  local_id,
                        unsigned int  line,
                        char*         file)
  {
  USE(service_id);
  USE(local_id);

  if (lock->exists != NCS_LOCK_EXISTS)
    {
    m_NCS_OS_ASSERT(0);
    m_LEAP_DBG_SINK(NCSCC_RC_FAILURE); /* so we know its this LOCK failure */
    return m_LEAP_GOTO_DBG_SINK(line,file,NCSCC_RC_FAILURE);
    }

  if (m_NCS_OS_LOCK(&lock->lock,NCS_OS_LOCK_LOCK,flag) != NCSCC_RC_SUCCESS)
    {
    m_LEAP_DBG_SINK(NCSCC_RC_FAILURE); /* so we know its this LOCK failure */
    return m_LEAP_GOTO_DBG_SINK(line,file,NCSCC_RC_FAILURE);
    }

  return NCSCC_RC_SUCCESS;
  }

/****************************************************************************
 *
 * Function Name: ncs_unlock
 *
 * Purpose:       Unlock a lock via OS PRIMS. Confirm that the lock exists. 
 *                Check return code and complain if it returns FAILURE.
 *
 * Note:          If both service_id and local_id are 0, the macro m_NCS_LOCK_DESTROY
 *                was used.  Otherwise, the macro m_NCS_LOCK_DESTROY_V2 was used.
 *                This is possible because m_NCS_LOCK_DESTROY_V2 does not use
 *                service_id 0.
 *
 ****************************************************************************/

EXTERN_C unsigned int  ncs_unlock      (NCS_LOCK*      lock,
                                       unsigned int  flag,
                                       NCS_SERVICE_ID service_id,
                                       unsigned int  local_id,
                                       unsigned int  line,
                                       char*         file)
  {
  USE(service_id);
  USE(local_id);

  if (lock->exists != NCS_LOCK_EXISTS)
    {
    m_NCS_OS_ASSERT(0);
    m_LEAP_DBG_SINK(NCSCC_RC_FAILURE); /* so we know its this LOCK failure */
    return m_LEAP_GOTO_DBG_SINK(line,file,NCSCC_RC_FAILURE);
    }

  if (m_NCS_OS_LOCK(&lock->lock,NCS_OS_LOCK_UNLOCK,flag) != NCSCC_RC_SUCCESS)
    {
    m_LEAP_DBG_SINK(NCSCC_RC_FAILURE); /* so we know its this LOCK failure */
    return m_LEAP_GOTO_DBG_SINK(line,file,NCSCC_RC_FAILURE);
    }

  return NCSCC_RC_SUCCESS;
  }

#endif

/****************************************************************************
 *
 * Function Name: ncs_lock_get_init_count
 *
 * Purpose:       Tells how many locks in the service id have been initialized
 *
 *
 *
 ****************************************************************************/
uns32 ncs_lock_get_init_count(NCS_SERVICE_ID service_id) {

   uns32 retval = 0;
   USE(service_id);
#if (NCSSYSM_LOCK_DBG_ENABLE != 0 )
   if(service_id < NCS_SERVICE_ID_MAX) {
      retval = init_lock_count[service_id];
   }
#endif
   return retval;
}

/****************************************************************************
 *
 * Function Name: ncs_lock_get_destroy_count
 *
 * Purpose:       Tells how many locks in the service id have been destroyed
 *
 *
 *
 ****************************************************************************/
uns32 ncs_lock_get_destroy_count(NCS_SERVICE_ID service_id) {

   uns32 retval = 0;
   USE(service_id);
#if (NCSSYSM_LOCK_DBG_ENABLE != 0 )
   if(service_id < NCS_SERVICE_ID_MAX) {
      retval = destroy_lock_count[service_id];
   }
#endif
   return retval;
}


/****************************************************************************
 *
 * Function Name: ncs_lock_stats
 *
 * Purpose:       Tells how many locks in the service id have been destroyed
 *
 * Parameters:
 *                filename - Name of file to dump stats to.  If null or "",
 *                           stats are dumped to the console
 *
 ****************************************************************************/
void ncs_lock_stats(char * filename) {

  char   buffer[80];
  FILE  *fh = NULL;
  char   asc_tod[32];
  time_t tod;


#if (NCSSYSM_LOCK_DBG_ENABLE != 0 )
  int   i;
  uns32 total_init_count;
  uns32 total_destroy_count;
  uns32 total_diff_count;
  uns32 diff_count;
#endif

  if(filename != NULL) {
    if(m_NCS_STRLEN(filename) > 0) {
      fh = sysf_fopen(filename, "at");
      if(fh == NULL) {
        m_NCS_CONS_PRINTF("Cannot open %s\n", filename);
        return;
      }
    }
  }



  asc_tod[0] = '\0';
  m_GET_ASCII_TIME_STAMP(tod, asc_tod);
  sysf_sprintf(buffer, "%s\n", asc_tod);
  fh==NULL ? m_NCS_CONS_PRINTF("%s", buffer) : sysf_fprintf (fh, buffer);

  sysf_sprintf(buffer, "|---------+-------------+-------------+-------------|\n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", buffer) : sysf_fprintf (fh, buffer);
  sysf_sprintf(buffer, "|                  Lock Statistics                  |\n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", buffer) : sysf_fprintf (fh, buffer);
  sysf_sprintf(buffer, "|---------+-------------+-------------+-------------|\n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", buffer) : sysf_fprintf (fh, buffer);
  sysf_sprintf(buffer, "| Service | Initialized |  Destroyed  |  Difference |\n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", buffer) : sysf_fprintf (fh, buffer);
  sysf_sprintf(buffer, "|   ID    |    Locks    |    Locks    |             |\n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", buffer) : sysf_fprintf (fh, buffer);
  sysf_sprintf(buffer, "|---------+-------------+-------------+-------------|\n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", buffer) : sysf_fprintf (fh, buffer);

#if (NCSSYSM_LOCK_DBG_ENABLE != 0 )

  total_init_count    = 0;
  total_destroy_count = 0;
  total_diff_count    = 0;
  m_NCS_LOCK_V2(&lock_mngr, NCS_LOCK_WRITE, NCS_SERVICE_ID_OS_SVCS, 0);
  for(i=0; i<NCS_SERVICE_ID_MAX; ++i) {
    total_init_count     += init_lock_count[i];
    total_destroy_count  += destroy_lock_count[i];
    diff_count = init_lock_count[i] - destroy_lock_count[i];
    total_diff_count += diff_count;
    if(init_lock_count[i] > 0 || destroy_lock_count[i] > 0) {
      sysf_sprintf (buffer, "   %3u    %9u     %9u     %9u\n",
                    i, init_lock_count[i], destroy_lock_count[i] , diff_count);
      fh==NULL ? m_NCS_CONS_PRINTF("%s", buffer) : sysf_fprintf (fh, buffer);
    }
  }
  m_NCS_UNLOCK_V2(&lock_mngr, NCS_LOCK_WRITE, NCS_SERVICE_ID_OS_SVCS, 0);
  sysf_sprintf(buffer, "|---------+-------------+-------------+-------------|\n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", buffer) : sysf_fprintf (fh, buffer);
  sysf_sprintf (buffer, " Total    %9u     %9u     %9u\n\n",
                total_init_count, total_destroy_count , total_diff_count);
  fh==NULL ? m_NCS_CONS_PRINTF("%s", buffer) : sysf_fprintf (fh, buffer);

#else

  sysf_sprintf (buffer, "Compile time flag NCSSYSM_LOCK_DBG_ENABLE must be set to 1\n") ;
  fh==NULL ? m_NCS_CONS_PRINTF("%s", buffer) : sysf_fprintf (fh, buffer);


#endif

  if(fh != NULL) sysf_fclose(fh);
}


