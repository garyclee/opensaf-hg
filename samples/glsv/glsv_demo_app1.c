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


#include <stdio.h>
#include <opensaf/ncsgl_defs.h>
#include <opensaf/os_defs.h>
#include <opensaf/ncs_osprm.h>
#include <opensaf/ncssysf_tsk.h>

int glsv_demo_appl1(void );

int glsv_demo_appl1(void)
{
    printf(" GLSV DEMO STARTED ...\n");

    /* create the process and exec the application */
    m_NCS_OS_PROCESS_EXECUTE("../../common/obj/glsv/glsv_main_app1.out", NULL);
    m_NCS_OS_PROCESS_EXECUTE("../../common/obj/glsv/glsv_main_app1.out", NULL);

   return 0;
}
