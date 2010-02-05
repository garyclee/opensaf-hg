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
 */


#include <opensaf/ncsgl_defs.h>
#include <opensaf/ncs_osprm.h>
#include <opensaf/ncssysfpool.h>
#include <opensaf/ncssysf_mem.h>
#include "leaptest.h"

/****************************************************************************************
*
* TEST: lt_memdiag
*
* Parameters:
*       none
*
****************************************************************************************/

int
lt_memdiag(int argc, char **argv)
{
   void *m1,*m2;

   ncs_mem_create ();

   m1 = m_NCS_MEM_ALLOC(100, 0, 0, 0);

   ncs_mem_whatsout_dump( ); /* expect one entry */

   m_NCS_MEM_FREE (m1, 0, 0, 0);

   ncs_mem_whatsout_dump( ); /* expect no entry */

   m1 = m_NCS_MEM_ALLOC(100, 0, 0, 0);

   ncs_mem_whatsout_dump( ); /* expect one entry */

   ncs_mem_ignore(TRUE);

   ncs_mem_whatsout_dump( ); /* expect no entry */

   m2 = m_NCS_MEM_ALLOC(200, 0, 0, 0);

   ncs_mem_whatsout_dump( ); /* expect one entry */

   ncs_mem_ignore(FALSE);

   ncs_mem_whatsout_dump( ); /* expect two entries */

   ncs_mem_ignore(TRUE);

   ncs_mem_whatsout_dump( ); /* expect no entry */

   m_NCS_MEM_FREE (m1, 0, 0, 0);

   ncs_mem_whatsout_dump( ); /* expect no entry */

   ncs_mem_ignore(FALSE);

   ncs_mem_whatsout_dump( ); /* expect one entry - the larger */

   ncs_mem_destroy ();
   return 0;
}

