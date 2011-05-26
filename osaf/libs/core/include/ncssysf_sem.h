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

  H&J Counting Semaphore Facility.

******************************************************************************
*/

/** Module Inclusion Control...
 **/
#ifndef NCSSYSF_SEM_H
#define NCSSYSF_SEM_H

#include "ncsgl_defs.h"

#ifdef  __cplusplus
extern "C" {
#endif

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

          H&J Counting Semaphore Definitions

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/****************************************************************************
 * m_NCS_SEM_CREATE
 *
 * This macro is invoked in order to allocate and/or initialize a counting
 * semaphore.  Upon return from this macro, "p_s_handle" should be dereferenced
 * and the handle of the allocated semaphore should be stored in *p_s_handle.
 *
 * ARGUMENTS:
 *
 * "p_s_handle" is a pointer to a semaphore handle (void **).
 *
 * RETURNS:
 *
 * NCSCC_RC_SUCCESS  if counting semaphore created and initialized successfully.
 * <error return>   otherwise (such as NCSCC_RC_FAILURE).
 */
#define m_NCS_SEM_CREATE(p_s_handle)    ncs_sem_create(p_s_handle)

/****************************************************************************
 * m_NCS_SEM_RELEASE
 *
 * This macro is invoked in order to release a counting semaphore.
 *
 * ARGUMENTS:
 *
 * "s_handle" is a semaphore handle (void *).
 *
 * RETURNS:
 *
 * NCSCC_RC_SUCCESS  if counting semaphore released successfully.
 * <error return>   otherwise (such as NCSCC_RC_FAILURE).
 */
#define m_NCS_SEM_RELEASE(s_handle)   ncs_sem_release(s_handle)

/****************************************************************************
 * m_NCS_SEM_GIVE
 *
 * This macro is invoked in order to "give" (i.e., increment) a counting
 * semaphore.
 *
 * ARGUMENTS:
 *
 * "s_handle" is a semaphore handle (void *).
 *
 * RETURNS:
 *
 * NCSCC_RC_SUCCESS  if counting semaphore "given" successfully.
 * <error return>   otherwise (such as NCSCC_RC_FAILURE).
 */
#define m_NCS_SEM_GIVE(s_handle)      ncs_sem_give(s_handle)

/****************************************************************************
 * m_NCS_SEM_TAKE
 *
 * This macro is invoked in order to "take" a counting semaphore.  This call
 * should block and not return until the semaphore has a non-zero value.  If
 * the semaphore has a non-zero value, it should be decremented by 1 before
 * returning.
 *
 * ARGUMENTS:
 *
 * "s_handle" is a semaphore handle (void *).
 *
 * RETURNS:
 *
 * NCSCC_RC_SUCCESS  if counting semaphore "taken" successfully.
 * <error return>   otherwise (such as NCSCC_RC_FAILURE).
 */
#define m_NCS_SEM_TAKE(s_handle)      ncs_sem_take(s_handle)

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 
               FUNCTION PROTOTYPES
 
 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

	uint32_t ncs_sem_create(void **);
	uint32_t ncs_sem_release(void *);
	uint32_t ncs_sem_give(void *);
	uint32_t ncs_sem_take(void *);

#ifdef  __cplusplus
}
#endif

#endif   /* NCSSYSF_SEM_H */
