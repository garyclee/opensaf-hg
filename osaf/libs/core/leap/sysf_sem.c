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

  This module contains routines related H&J Counting Semaphores.
..............................................................................

  FUNCTIONS INCLUDED in this module:

  ncs_sem_create....create/initialize a semaphore
  ncs_sem_release...release a semaphore
  ncs_sem_give......increment the semaphore by 1
  ncs_sem_take......wait for semaphore to be greater than 0

 ******************************************************************************
 */

#include <ncsgl_defs.h>
#include "ncs_osprm.h"
#include "ncssysf_sem.h"

uint32_t ncs_sem_create(void **s_handle)
{
	NCS_OS_SEM sem;
	uint32_t rc;

	rc = m_NCS_OS_SEM(&sem, NCS_OS_SEM_CREATE);

	if (NCSCC_RC_SUCCESS == rc)
		*s_handle = sem.info.create.o_handle;

	return rc;
}

uint32_t ncs_sem_release(void *s_handle)
{
	NCS_OS_SEM sem;

	sem.info.release.i_handle = s_handle;

	return m_NCS_OS_SEM(&sem, NCS_OS_SEM_RELEASE);
}

uint32_t ncs_sem_give(void *s_handle)
{
	NCS_OS_SEM sem;

	sem.info.give.i_handle = s_handle;

	return m_NCS_OS_SEM(&sem, NCS_OS_SEM_GIVE);
}

uint32_t ncs_sem_take(void *s_handle)
{
	NCS_OS_SEM sem;

	sem.info.take.i_handle = s_handle;

	return m_NCS_OS_SEM(&sem, NCS_OS_SEM_TAKE);
}
