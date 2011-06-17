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
 * DESCRIPTION:
 *   This file provides the suggested additions to the C language binding for 
 *   the Service Availability(TM) Forum Information Model Management Service (IMM).
 *   It contains only the prototypes and type definitions that are part of this
 *   proposed addition. 
 *   These additions are currently NON STANDARD. But the intention is to get these
 *   additions approved formally by SAF in the future.
 *
 * FILE:
 *   saImmOm.A.2.11.h
 *
 ** SPECIFICATION VERSION:
 *   SAI-AIS-IMM-A.02.11
 *
 * DATE:
 *   Mon  May   23  2011
 * Author(s): Ericsson AB
 *
 */


#ifndef _SA_IMM_OM_A_2_11_H
#define _SA_IMM_OM_A_2_11_H

#ifdef  __cplusplus
extern "C" {
#endif
	/* 4.9.2 SaImmOmAdminOperationInvokeCallbackT  See http://devel.opensaf.org/ticket/1764 */

	typedef void
	 (*SaImmOmAdminOperationInvokeCallbackT_o2) (
						    SaInvocationT invocation,
						    SaAisErrorT operationReturnValue,
						    SaAisErrorT error,
						    SaImmAdminOperationParamsT_2 **returnParams);

	/* 4.2.18 SaImmCallbacksT  See http://devel.opensaf.org/ticket/1764*/

	typedef struct {
		SaImmOmAdminOperationInvokeCallbackT_o2
		  saImmOmAdminOperationInvokeCallback;
	} SaImmCallbacksT_o2;


	/*
	 *************************
	 *                       *
	 *   Om Function Calls   *
	 *                       *
	 *************************
	 */

	/* 4.3.1 saImmOmInitialize() */

	extern SaAisErrorT
	  saImmOmInitialize_o2(
			       SaImmHandleT *immHandle, 
			       const SaImmCallbacksT_o2 *immCallbacks, 
			       SaVersionT *version);

	/* 4.9.1 saImmOmAdminOperationInvoke(),
	   saImmOmAdminOperationInvokeAsync() 
	   See http://devel.opensaf.org/ticket/1764 */
	extern SaAisErrorT
	 saImmOmAdminOperationInvoke_o2(
					SaImmAdminOwnerHandleT ownerHandle,
					const SaNameT *objectName,
					SaImmContinuationIdT continuationId,
					SaImmAdminOperationIdT operationId,
					const SaImmAdminOperationParamsT_2 **params,
					SaAisErrorT *operationReturnValue,
					SaTimeT timeout,
					SaImmAdminOperationParamsT_2 ***returnParams);

	/* saImmOmAdminOperationInvokeAsync() downcall is unchanged in A.02.11 */ 

	/* 4.9.3 saImmOmAdminOperationContinue(),
	   saImmOmAdminOperationContinueAsync() */

	/* This is not implemented in OpenSAF, but define the API anyway for completenes. 
           See http://devel.opensaf.org/ticket/1764 */

	extern SaAisErrorT
	 saImmOmAdminOperationContinue_o2(
					  SaImmAdminOwnerHandleT ownerHandle,
					  const SaNameT *objectName,
					  SaImmContinuationIdT continuationId,
					  SaAisErrorT *operationReturnValue,
					  SaImmAdminOperationParamsT_2 ***returnParams);


	/* Free memory for return parameters. Used after syncronous reply. 
           See http://devel.opensaf.org/ticket/1764 */

	extern SaAisErrorT
	 saImmOmAdminOperationMemoryFree(
					 SaImmAdminOwnerHandleT ownerHandle,
					 SaImmAdminOperationParamsT_2 **returnParams);


	/* See: http://devel.opensaf.org/ticket/1904 */

	extern SaAisErrorT
	 saImmOmCcbGetErrorStrings(
				   SaImmCcbHandleT ccbHandle,
				   SaStringT **errorStrings);


	/* 4.2.22 IMM Service Object Attributes */
	/* Multivalued non-persistent RTA, not yet supported. */
	/* See http://devel.opensaf.org/ticket/1827 */
#define SA_IMM_ATTR_APPLIER_NAME "saImmAttrApplierName"  


	/* 4.2.5 SaImmAttrFlagsT */
	/*
#define SA_IMM_ATTR_MULTI_VALUE   0x00000001
#define SA_IMM_ATTR_RDN           0x00000002
#define SA_IMM_ATTR_CONFIG        0x00000100
#define SA_IMM_ATTR_WRITABLE      0x00000200
#define SA_IMM_ATTR_INITIALIZED   0x00000400
#define SA_IMM_ATTR_RUNTIME       0x00010000
#define SA_IMM_ATTR_PERSISTENT    0x00020000
#define SA_IMM_ATTR_CACHED        0x00040000
	*/
#define SA_IMM_ATTR_NO_DUPLICATES 0x0000000000000004   /* See: http://devel.opensaf.org/ticket/1545 */


        /* 4.2.12 SaImmSearchOptionsT */
	/*
#define SA_IMM_SEARCH_ONE_ATTR       0x0001
#define SA_IMM_SEARCH_GET_ALL_ATTR   0x0100
#define SA_IMM_SEARCH_GET_NO_ATTR    0x0200
#define SA_IMM_SEARCH_GET_SOME_ATTR  0x0400
	*/
#define SA_IMM_SEARCH_GET_CONFIG_ATTR 0x0000000000000800   /* See: http://devel.opensaf.org/ticket/1897 */


#ifdef  __cplusplus
}
#endif

#endif   /* _SA_IMM_OM_A_2_11_H */
