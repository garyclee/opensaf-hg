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

#ifndef CPND_MIB_H
#define CPND_MIB_H

uns32 sackptcheckpointsectionentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                                       NCSMIB_VAR_INFO* var_info,
                                       NCS_BOOL test_flag);

uns32 sackptcheckpointsectionentry_extract(NCSMIB_PARAM_VAL *param,
                                           NCSMIB_VAR_INFO *var_info, 
                                           NCSCONTEXT data, NCSCONTEXT buffer);

uns32 sackptcheckpointsectionentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, 
                                        NCSCONTEXT *data);

uns32 sackptcheckpointsectionentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                                   NCSMIB_SETROW_PARAM_VAL* params,
                                   struct ncsmib_obj_info* obj_info,
                                   NCS_BOOL testrow_flag);


uns32 sackptcheckpointsectionentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg,
                                               NCSCONTEXT* data);


#endif
