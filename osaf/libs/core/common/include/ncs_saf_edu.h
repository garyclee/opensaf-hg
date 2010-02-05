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

  This file lists EDP(EDU program) definitions for common SAF data structures.
  
******************************************************************************
*/

#ifndef NCS_SAF_EDU_H
#define NCS_SAF_EDU_H

#include "ncs_edu_pub.h"

/*
    SYNOPSIS on mapping between SAF-data-type and NCS-data-type.
        SaUint8T        ->  "unsigned char"
        SaUint16T       ->  "unsigned short"
        SaUint32T       ->  "unsigned long"
        SaUint64T       ->  "uns64"
                (if NCS_UNS64_DEFINED is set, uns64 is 64bit data type.
                 else, uns64 - defaults to uns32.)
        SaInt64T        ->  "int64"
                (if NCS_64BIT_DATA_TYPE_SUPPORT is set, int64 is 64bit
                            data type.
                 else, int64 - SHOULD BE defaulting to uns32.)

        SaAmfHealthcheckInvocationT     ->  "uns32"
        SaAmfErrorImpactAndSeverityT    ->  "uns32"
        SaAmfRecommendedRecoveryT       ->  "uns32"
        SaErrorT                        ->  "uns32"
        SaAmfReadinessStateT            ->  "uns32"
        SaAmfHAStateT                   ->  "uns32"
        SaAmfComponentCapabilityModelT  ->  "uns32"
        SaAmfCSITransitionDescriptorT   ->  "uns32"
        SaAmfExternalComponentActionT   ->  "uns32"
        SaAmfHandleT                    ->  SaUint32T
        SaAmfPendingOperationFlagsT     ->  SaUint32T
        SaAmfCSIFlagsT                  ->  SaUint32T
        SaInvocationT                   ->  SaUint64T
        SaSizeT                         ->  SaUint64T
        SaClmNodeIdT                    ->  SaUint32T
        SaAmfPmErrorsT                  ->  SaUint32T
        SaAmfPmStopQualifierT           ->  "uns32"
        SaClmHandleT                    ->  SaUint64T
        SaClmClusterChangesT            ->  "uns32"
        SaBoolT                         ->  "uns32"

*/

#define m_NCS_EDP_SAUINT8T                          ncs_edp_uns8
#define m_NCS_EDP_SAUINT16T                         ncs_edp_uns16
#define m_NCS_EDP_SAUINT32T                         ncs_edp_uns32
#define m_NCS_EDP_SAINT32T                          ncs_edp_int

/* Note : 
 *      m_NCS_EDP_ULONG is mapped to "ncs_edp_uns32" for 32-bits only(
 *      defined in ncs_edu_pub.h). 
 *      Need modify this function for 64-bit machines later, if needed. 
 */
#if(NCS_UNS64_DEFINED == 1)
#define m_NCS_EDP_SAUINT64T                         ncs_edp_uns64
#define m_NCS_EDP_SAINT64T                          ncs_edp_int64
#else
    /*  
     *  Purposefully left for builds to fail, so that it is tracked 
     *  when we move to 64-bit machines.
     */
#define m_NCS_EDP_SAUINT64T                         ncs_edp_64bit_fail
#define m_NCS_EDP_SAINT64T                          ncs_edp_64bit_fail
#endif

#define m_NCS_EDP_SAAMFHEALTHCHECKINVOCATIONT       ncs_edp_uns32
#define m_NCS_EDP_SAAMFRECOMMENDEDRECOVERYT         ncs_edp_uns32
#define m_NCS_EDP_SAERRORT                          ncs_edp_uns32
#define m_NCS_EDP_SAAMFREADINESSSTATET              ncs_edp_uns32
#define m_NCS_EDP_SAAMFHASTATET                     ncs_edp_uns32
#define m_NCS_EDP_SAAMFCOMPONENTCAPABILITYMODELT    ncs_edp_uns32
#define m_NCS_EDP_SAAMFCSITRANSITIONDESCRIPTORT     ncs_edp_uns32
#define m_NCS_EDP_SAAMFEXTERNALCOMPONENTACTIONT     ncs_edp_uns32
#define m_NCS_EDP_SAAMFHANDLET                      m_NCS_EDP_SAUINT64T
#define m_NCS_EDP_SAAMFPENDINGOPERATIONFLAGST       m_NCS_EDP_SAUINT32T
#define m_NCS_EDP_SAAMFCSIFLAGST                    m_NCS_EDP_SAUINT32T
#define m_NCS_EDP_SAINVOCATIONT                     m_NCS_EDP_SAUINT64T
#define m_NCS_EDP_SASIZET                           m_NCS_EDP_SAUINT64T
#define m_NCS_EDP_SACLMNODEIDT                      m_NCS_EDP_SAUINT32T
#define m_NCS_EDP_SATIMET                           m_NCS_EDP_SAINT64T
#define m_NCS_EDP_NCSOPERSTATE                      ncs_edp_uns32
#define m_NCS_EDP_SAAMFPROTECTIONGROUPCHANGEST      ncs_edp_uns32

#define m_NCS_EDP_SAAMFPMERRORST                    m_NCS_EDP_SAUINT32T
#define m_NCS_EDP_SAAMFPMSTOPQUALIFIERT             ncs_edp_uns32
#define m_NCS_EDP_SACLMHANDLET                      m_NCS_EDP_SAUINT64T
#define m_NCS_EDP_SABOOLT                           ncs_edp_uns32
#define m_NCS_EDP_SACLMCLUSTERCHANGEST              ncs_edp_uns32

 uns32 ncs_edp_sanamet(EDU_HDL *edu_hdl,
				  EDU_TKN *edu_tkn, NCSCONTEXT ptr,
				  uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
 uns32 ncs_edp_sanamet_net(EDU_HDL *edu_hdl,
				      EDU_TKN *edu_tkn, NCSCONTEXT ptr,
				      uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
 uns32 ncs_edp_saversiont(EDU_HDL *edu_hdl,
				     EDU_TKN *edu_tkn, NCSCONTEXT ptr,
				     uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
 uns32 ncs_edp_saamfhealthcheckkeyt(EDU_HDL *edu_hdl,
					       EDU_TKN *edu_tkn, NCSCONTEXT ptr,
					       uns32 *ptr_data_len, EDU_BUF_ENV *buf_env,
					       EDP_OP_TYPE op, EDU_ERR *o_err);
 uns32 ncs_edp_saclmnodeaddresst(EDU_HDL *edu_hdl,
					    EDU_TKN *edu_tkn, NCSCONTEXT ptr,
					    uns32 *ptr_data_len, EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
 uns32 ncs_edp_saamfprotectiongroupmembert(EDU_HDL *hdl, EDU_TKN *edu_tkn,
						      NCSCONTEXT ptr, uns32 *ptr_data_len,
						      EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
 uns32 ncs_edp_saamfprotectiongroupnotificationt(EDU_HDL *hdl, EDU_TKN *edu_tkn,
							    NCSCONTEXT ptr, uns32 *ptr_data_len,
							    EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, EDU_ERR *o_err);
 uns32 avsv_edp_saamfprotectiongroupnotificationbuffert(EDU_HDL *hdl, EDU_TKN *edu_tkn,
								   NCSCONTEXT ptr, uns32 *ptr_data_len,
								   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op,
								   EDU_ERR *o_err);

/* Utility routines to free data structures malloc'ed by EDU */
 void ncs_saf_free_saamfhealthcheckkeyt(SaAmfHealthcheckKeyT *p);

#endif   /* NCS_SAF_EDU_H */
