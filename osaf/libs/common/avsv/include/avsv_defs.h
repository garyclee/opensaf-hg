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

  This module is the main include file for the entire Availability Service.
  It contains the data definitions for some common data types.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVSV_DEFS_H
#define AVSV_DEFS_H

/* leap timers are 10ms timers so divide with 10000000 */
#define AVSV_NANOSEC_TO_LEAPTM 10000000

/* Global known values */
#define AVSV_AVD_VCARD_ID  (MDS_VDEST_ID)1

/* units of nano seconds value 2 seconds */
#define AVSV_CLUSTER_INIT_INTVL ((SaTimeT)2000000000)

/* The defines for the script size and other 
 * misc string sizes. same as name size. 
 */
#define AVSV_MISC_STR_MAX_SIZE 256

/* Minimum preferred num of su in 2N, N+M and NWay red model*/
#define AVSV_SG_2N_PREF_INSVC_SU_MIN 2

/* Max value for a handle given by avsv to APP */
#define AVSV_UNS32_HDL_MAX 0xffffffff

typedef enum {
	AVSV_COMP_TYPE_INVALID,
	AVSV_COMP_TYPE_SA_AWARE,
	AVSV_COMP_TYPE_PROXIED_LOCAL_PRE_INSTANTIABLE,
	AVSV_COMP_TYPE_PROXIED_LOCAL_NON_PRE_INSTANTIABLE,
	AVSV_COMP_TYPE_EXTERNAL_PRE_INSTANTIABLE,
	AVSV_COMP_TYPE_EXTERNAL_NON_PRE_INSTANTIABLE,
	AVSV_COMP_TYPE_NON_SAF,
} AVSV_COMP_TYPE_VAL;

/* 
 * SaAmfRecommendedRecoveryT definition does not include escalated recovery
 * actions like su-restart, su-failover etc. The following enum definition
 * captures them..
 */
typedef enum avsv_err_rcvr {
	/* recovery specified in SaAmfRecommendedRecoveryT */
	/* escalated recovery */
	AVSV_ERR_RCVR_SU_RESTART = SA_AMF_CONTAINER_RESTART + 1,
	AVSV_ERR_RCVR_SU_FAILOVER,
	AVSV_ERR_RCVR_MAX
} AVSV_ERR_RCVR;

/* This structure is used by the CSI
 * attribute name value pair.
 */
typedef struct {
	SaNameT name;
	SaNameT value;
} AVSV_ATTR_NAME_VAL;

/* This structure is the list of CSI
 * attribute name value pairs .
 */
typedef struct {
	uns32 number;
	AVSV_ATTR_NAME_VAL *list;
} AVSV_CSI_ATTRS;

/* This enums represent classes defined in AMF Spec B.04.01, sorted from table 21 ch 8.4 */
typedef enum {
	AVSV_SA_AMF_CLASS_INVALID = 0,
	AVSV_SA_AMF_CONFIG_CLASS_BASE = 1,
	AVSV_SA_AMF_APP_BASE_TYPE = 1,
	AVSV_SA_AMF_APP = 2,
	AVSV_SA_AMF_APP_TYPE = 3,
	AVSV_SA_AMF_CLUSTER = 4,
	AVSV_SA_AMF_COMP = 5,
	AVSV_SA_AMF_COMP_BASE_TYPE = 6,
	AVSV_SA_AMF_COMP_CS_TYPE = 7,
	AVSV_SA_AMF_COMP_GLOBAL_ATTR = 8,
	AVSV_SA_AMF_COMP_TYPE = 9,
	AVSV_SA_AMF_CS_BASE_TYPE = 10,
	AVSV_SA_AMF_CSI = 11,
	AVSV_SA_AMF_CSI_ASSIGNMENT = 12,
	AVSV_SA_AMF_CSI_ATTRIBUTE = 13,
	AVSV_SA_AMF_CS_TYPE = 14,
	AVSV_SA_AMF_CT_CS_TYPE = 15,
	AVSV_SA_AMF_HEALTH_CHECK = 16,
	AVSV_SA_AMF_HEALTH_CHECK_TYPE = 17,
	AVSV_SA_AMF_NODE = 18,
	AVSV_SA_AMF_NODE_GROUP = 19,
	AVSV_SA_AMF_NODE_SW_BUNDLE = 20,
	AVSV_SA_AMF_SG = 21,
	AVSV_SA_AMF_SG_BASE_TYPE = 22,
	AVSV_SA_AMF_SG_TYPE = 23,
	AVSV_SA_AMF_SI = 24,
	AVSV_SA_AMF_SI_ASSIGNMENT = 25,
	AVSV_SA_AMF_SI_DEPENDENCY = 26,
	AVSV_SA_AMF_SI_RANKED_SU = 27,
	AVSV_SA_AMF_SU = 28,
	AVSV_SA_AMF_SU_BASE_TYPE = 29,
	AVSV_SA_AMF_SUT_COMP_TYPE = 30,
	AVSV_SA_AMF_SU_TYPE = 31,
	AVSV_SA_AMF_SVC_BASE_TYPE = 32,
	AVSV_SA_AMF_SVC_TYPE = 33,
	AVSV_SA_AMF_SVC_TYPE_CS_TYPES = 34,
	AVSV_SA_AMF_CLASS_MAX
} AVSV_AMF_CLASS_ID;

/* Attribute ID enum for the saAmfNode class */
typedef enum
{
   saAmfNodeName_ID = 1,
   saAmfNodeSuFailoverProb_ID = 2,
   saAmfNodeSuFailoverMax_ID = 3,
   saAmfNodeAdminState_ID = 4,
} AVSV_AMF_NODE_ATTR_ID; 

/* Attribute ID enum for the saAmfSG class */
typedef enum
{
   saAmfSGName_ID = 1,
   saAmfSGRedModel_ID = 2,
   saAmfSGFailbackOption_ID = 3,
   saAmfSGNumPrefActiveSUs_ID = 4,
   saAmfSGNumPrefStandbySUs_ID = 5,
   saAmfSGNumPrefInserviceSUs_ID = 6,
   saAmfSGNumPrefAssignedSUs_ID = 7,
   saAmfSGMaxActiveSIsperSU_ID = 8,
   saAmfSGMaxStandbySIsperSU_ID = 9,
   saAmfSGAdminState_ID = 10,
   saAmfSGCompRestartProb_ID = 11,
   saAmfSGCompRestartMax_ID = 12,
   saAmfSGSuRestartProb_ID = 13,
   saAmfSGSuRestartMax_ID = 14,
   saAmfSGNumCurrAssignedSU_ID = 15,
   saAmfSGNumCurrNonInstantiatedSpareSU_ID = 16,
   saAmfSGNumCurrSpareSU_ID = 17,
} AVSV_AMF_SG_ATTR_ID; 

/* Attribute ID enum for the saAmfSU class */
typedef enum
{
   saAmfSUName_ID = 1,
   saAmfSURank_ID = 2,
   saAmfSUNumComponents_ID = 3,
   saAmfSUNumCurrActiveSIs_ID = 4,
   saAmfSUNumCurrStandbySIs_ID = 5,
   saAmfSUAdminState_ID = 6,
   saAmfSUFailOver_ID = 7,
   saAmfSUReadinessState_ID = 8,
   saAmfSUOperState_ID = 9,
   saAmfSUPresenceState_ID = 10,
   saAmfSUPreInstantiable_ID = 11,
   saAmfSUParentSGName_ID = 12,
   saAmfSUIsExternal_ID = 13,
} AVSV_AMF_SU_ATTR_ID; 

/* Attribute ID enum for the saAmfComp class */
typedef enum
{
   saAmfCompName_ID = 1,
   saAmfCompCapability_ID = 2,
   saAmfCompCategory_ID = 3,
   saAmfCompInstantiateCmd_ID = 4,
   saAmfCompTerminateCmd_ID = 5,
   saAmfCompCleanupCmd_ID = 6,
   saAmfCompAmStartCmd_ID = 7,
   saAmfCompAmStopCmd_ID = 8,
   saAmfCompInstantiationLevel_ID = 9,
   saAmfCompInstantiateTimeout_ID = 10,
   saAmfCompDelayBetweenInstantiateAttempts_ID = 11,
   saAmfCompTerminateTimeout_ID = 12,
   saAmfCompCleanupTimeout_ID = 13,
   saAmfCompAmStartTimeout_ID = 14,
   saAmfCompAmStopTimeout_ID = 15,
   saAmfCompTerminateCallbackTimeOut_ID = 16,
   saAmfCompCSISetCallbackTimeout_ID = 17,
   saAmfCompQuiescingCompleteTimeout_ID = 18,
   saAmfCompCSIRmvCallbackTimeout_ID = 19,
   saAmfCompProxiedCompInstantiateCallbackTimeout_ID = 20,
   saAmfCompProxiedCompCleanupCallbackTimeout_ID = 21,
   saAmfCompNodeRebootCleanupFail_ID = 22,
   saAmfCompRecoveryOnError_ID = 23,
   saAmfCompNumMaxInstantiate_ID = 24,
   saAmfCompNumMaxInstantiateWithDelay_ID = 25,
   saAmfCompNumMaxAmStartAttempts_ID = 26,
   saAmfCompNumMaxAmStopAttempts_ID = 27,
   saAmfCompDisableRestart_ID = 28,
   saAmfCompNumMaxActiveCsi_ID = 29,
   saAmfCompNumMaxStandbyCsi_ID = 30,
   saAmfCompNumCurrActiveCsi_ID = 31,
   saAmfCompNumCurrStandbyCsi_ID = 32,
   saAmfCompOperState_ID = 33,
   saAmfCompReadinessState_ID = 34,
   saAmfCompPresenceState_ID = 35,
   saAmfCompRestartCount_ID = 36,
   saAmfCompCurrProxyName_ID = 37,
   saAmfCompAMEnable_ID = 38,
   saAmfCompProxyStatus_ID = 39,
   saAmfCompType_ID,
} AVSV_AMF_COMP_ATTR_ID; 

/* Attribute ID enum for the SaAmfHealthcheck class */
typedef enum
{
   saAmfHealthcheckPeriod_ID = 1,
   saAmfHealthcheckMaxDuration_ID = 2,
} AVSV_AMF_HEALTHCHECK_ATTR_ID; 


#define AVSV_COMMON_SUB_ID_DEFAULT_VAL 1
#define SA_AMF_PRESENCE_ORPHANED (SA_AMF_PRESENCE_TERMINATION_FAILED+1)

#endif
