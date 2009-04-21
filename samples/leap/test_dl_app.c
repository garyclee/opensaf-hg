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
#include <opensaf/ncssysf_def.h>
#include "leaptest.h"

/* Global status variable */
uns32 gl_dl_app_status = 1;

uns32 lt_dl_app_routine(int argument)
{
    if(argument == 2)
    {
        /* Change the status to 2 */
        gl_dl_app_status = 2;
        printf("lt_dl_app_routine( ): Argument passed(%d) is invalid...\n", 
            argument);
        return NCSCC_RC_SUCCESS;
    }

    /* Confirm the status to 1 */
    gl_dl_app_status = 1;
    printf("lt_dl_app_routine( ): Argument passed(%d) is valid...\n", 
        argument);

    return NCSCC_RC_SUCCESS;
}

