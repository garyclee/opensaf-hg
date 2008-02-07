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

#ifndef PDRBD_AMF_H
#define PDRBD_AMF_H



#include "pdrbd.h"



/*
   Script response timeouts in millisecs; refer timeouts in BOM before changing the values.
   The values here ought to be smaller than the BOM timeouts.
*/
#define   PDRBD_SCRIPT_TIMEOUT_ACTIVE        120000      /* 1800000 in BOM */
#define   PDRBD_SCRIPT_TIMEOUT_STANDBY       1680000     /* 1800000 in BOM */
#define   PDRBD_SCRIPT_TIMEOUT_QUIESCED      90000       /* 120000 in BOM */
#define   PDRBD_SCRIPT_TIMEOUT_REMOVE        90000       /* 120000 in BOM */
/*
   In the health check callback the Pseudo (main) component always returns success and invokes the pdrbdsts
   script to check the status of all proxied (sub) components. The health check callback is not registered and
   not applicable for the proxied components. So, the below timeout is only the script response timeout.
*/
#define   PDRBD_SCRIPT_TIMEOUT_HLTH_CHK      60000       /* frequency:75000, duration:80000 in BOM */
#define   PDRBD_SCRIPT_TIMEOUT_TERMINATE     90000       /* 120000 in BOM */
#define   PDRBD_SCRIPT_TIMEOUT_CLEAN_META    60000

#define   PSEUDO_DRBD_DEFAULT_ROLE    0

#define PSEUDO_CTRL_SCRIPT_NAME    "/opt/opensaf/controller/scripts/pdrbdctrl"
#define PSEUDO_STS_SCRIPT_NAME    "/opt/opensaf/controller/scripts/pdrbdsts"

extern uns32 pseudoAmfInitialise(void);
extern uns32 pseudoAmfRegister(void);

extern uns32 pseudoAmfUninitialise(void);
extern uns32 pseudoAmfUnregister(void);

extern uns32 pdrbdProxiedAmfInitialise(void);
extern uns32 pdrbdProxiedAmfRegister(uns32);

extern uns32 pdrbdProxiedAmfUninitialise(void);
extern uns32 pdrbdProxiedAmfUnregister(uns32);

extern void pseudoAmfCsiSetCallback(SaInvocationT,const SaNameT *,SaAmfHAStateT,SaAmfCSIDescriptorT);
extern void pseudoAmfCsiRemoveCallback(SaInvocationT,const SaNameT *);
extern void pseudoAmfHealthcheckCallback (SaInvocationT,const SaNameT *,SaAmfHealthcheckKeyT *);
extern void pseudoAmfTerminateCallback(SaInvocationT,const SaNameT *);

extern void pdrbdProxiedAmfCsiSetCallback(SaInvocationT,const SaNameT *,SaAmfHAStateT,SaAmfCSIDescriptorT);
extern void pdrbdProxiedAmfCsiRemoveCallback(SaInvocationT,const SaNameT *);
extern void pdrbdProxiedAmfHealthcheckCallback (SaInvocationT,const SaNameT *,SaAmfHealthcheckKeyT *);
extern void pdrbdProxiedAmfTerminateCallback(SaInvocationT,const SaNameT *);
extern void pdrbdProxiedAmfInstantiateCallback(SaInvocationT,const SaNameT *);
extern void pdrbdProxiedAmfCleanupCallback(SaInvocationT,const SaNameT *);


#endif /* PDRBD_AMF_H */
