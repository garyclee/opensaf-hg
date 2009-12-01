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
#define AVSV_AVD_SSN_REF   (uns32)1

/* units of nano seconds value 300 milli second */
#define AVSV_SND_HB_INTVL ((SaTimeT)300000000)

/* units of nano seconds value 2 second */
#define AVSV_RCV_HB_INTVL ((SaTimeT)2000000000)

/* units of nano seconds value 2 seconds */
#define AVSV_CLUSTER_INIT_INTVL ((SaTimeT)2000000000)

/* units of nano seconds value 2 second */
#define AVSV_INST_TIMEOUT ((SaTimeT)2000000000)

/* units of nano seconds value 2 second */
#define AVSV_TERM_TIMEOUT ((SaTimeT)2000000000)

/* units of nano seconds value 2 second */
#define AVSV_CLEAN_TIMEOUT ((SaTimeT)2000000000)

/* units of nano seconds value 2 second */
#define AVSV_AM_START_TIMEOUT ((SaTimeT)2000000000)

/* units of nano seconds value 2 second */
#define AVSV_AM_STOP_TIMEOUT ((SaTimeT)2000000000)

/* units of nano seconds value 2 second */
#define AVSV_TERM_CB_TIMEOUT ((SaTimeT)2000000000)

/* units of nano seconds value 2 second */
#define AVSV_CSI_SET_CB_TIMEOUT ((SaTimeT)2000000000)

/* units of nano seconds value 2 second */
#define AVSV_QUES_DONE_TIMEOUT ((SaTimeT)2000000000)

/* units of nano seconds value 2 second */
#define AVSV_CSI_RMV_CB_TIMEOUT ((SaTimeT)2000000000)

/* units of nano seconds value 2 second */
#define AVSV_PROX_INST_CB_TIMEOUT ((SaTimeT)2000000000)

/* units of nano seconds value 2 second */
#define AVSV_PROX_CL_CB_TIMEOT ((SaTimeT)2000000000)

/* units of nano seconds value 1 second */
#define AVSV_DEFAULT_HEALTH_CHECK_PERIOD ((SaTimeT)1000000000)

/* units of nano seconds value 500 milisecond */
#define AVSV_DEFAULT_HEALTH_CHECK_MAX_DURATION ((SaTimeT)500000000)

/* units of nano seconds value 10 milisecond */
#define AVSV_DEFAULT_HEALTH_CHECK_MIN_TIME ((SaTimeT)10000000)

/* The defines for the script size and other 
 * misc string sizes. same as name size. 
 */
#define AVSV_MISC_STR_MAX_SIZE 256

/* Minimum preferred num of su in 2N, N+M and NWay red model*/
#define SG_2N_PREF_INSVC_SU_MIN 2

/* Max value for a handle given by avsv to APP */
#define AVSV_UNS32_HDL_MAX 0xffffffff

/* Trap retention timeout */
#define AVSV_TRAP_RETENTION_TIMEOUT  200000000

/* Trap channel open timeout */
#define AVSV_TRAP_CHANNEL_OPEN_TIMEOUT 200000000

/* Trap pattern array lengths */
#define AVSV_TRAP_PATTERN_ARRAY_LEN  2
#define AVD_SHUT_FAIL_TRAP_PATTERN_ARRAY_LEN 1

/* Maximum number for component instantiation */
#define AVSV_MAX_INST 1

/* Maximum number for AM start attempts */
#define AVSV_MAX_AMSTART 1

/* Define the Ranks for the persistent MIB tables */
typedef enum {
	AVSV_TBL_RANK_MIN = 20,
	AVSV_TBL_RANK_NCSSG = AVSV_TBL_RANK_MIN,
	AVSV_TBL_RANK_AMFSG,
	AVSV_TBL_RANK_NCSND,
	AVSV_TBL_RANK_AMFND,
	AVSV_TBL_RANK_AMFSU,
	AVSV_TBL_RANK_AMFCOMP,
	AVSV_TBL_RANK_AMFSI,
	AVSV_TBL_RANK_AMFSI_DEP,
	AVSV_TBL_RANK_AMFCSTYPEPARAM,
	AVSV_TBL_RANK_AMFCSINV,
	AVSV_TBL_RANK_AMFCSI,
	AVSV_TBL_RANK_MAX
} AVSV_TBL_RANK;

/* comp capability model definition */
typedef enum ncs_comp_capability_model {
	NCS_COMP_CAPABILITY_X_ACTIVE_AND_Y_STANDBY = 1,
	NCS_COMP_CAPABILITY_X_ACTIVE_OR_Y_STANDBY = 2,
	NCS_COMP_CAPABILITY_1_ACTIVE_OR_Y_STANDBY = 3,
	NCS_COMP_CAPABILITY_1_ACTIVE_OR_1_STANDBY = 4,
	NCS_COMP_CAPABILITY_X_ACTIVE = 5,
	NCS_COMP_CAPABILITY_1_ACTIVE = 6,
	NCS_COMP_CAPABILITY_NO_STATE = 7
} NCS_COMP_CAPABILITY_MODEL;

typedef enum {
	NCS_COMP_TYPE_SA_AWARE,
	NCS_COMP_TYPE_PROXIED_LOCAL_PRE_INSTANTIABLE,
	NCS_COMP_TYPE_PROXIED_LOCAL_NON_PRE_INSTANTIABLE,
	NCS_COMP_TYPE_EXTERNAL_PRE_INSTANTIABLE,
	NCS_COMP_TYPE_EXTERNAL_NON_PRE_INSTANTIABLE,
	NCS_COMP_TYPE_NON_SAF,
} NCS_COMP_TYPE_VAL;

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
} NCS_AVSV_ATTR_NAME_VAL;

/* This structure is the list of CSI
 * attribute name value pairs .
 */
typedef struct {
	uns32 number;
	NCS_AVSV_ATTR_NAME_VAL *list;
} NCS_AVSV_CSI_ATTRS;

/* The value to toggle a SI.*/
typedef enum {
	AVSV_SI_TOGGLE_STABLE,
	AVSV_SI_TOGGLE_SWITCH
} SaToggleState;

/* The value to re adjust a SG.*/
typedef enum {
	AVSV_SG_STABLE,
	AVSV_SG_ADJUST
} SaAdjustState;

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
} SA_AMF_NODE_ATTR_ID; 

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
} SA_AMF_SG_ATTR_ID; 

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
} SA_AMF_SU_ATTR_ID; 

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
   saAmfCompType_ID,
} SA_AMF_COMP_ATTR_ID; 

/* Attribute ID enum for the SaAmfHealthcheck class */
typedef enum
{
   saAmfHealthcheckPeriod_ID = 1,
   saAmfHealthcheckMaxDuration_ID = 2,
} SA_AMF_HEALTHCHECK_ATTR_ID; 

/* Macros for data to MIB arg modification */
#define m_AVSV_OCTVAL_TO_PARAM(param,buffer,len,val) \
{\
   param->i_fmat_id = NCSMIB_FMAT_OCT; \
   param->i_length = len; \
   memcpy((uns8 *)buffer,val,param->i_length); \
   param->info.i_oct = (uns8 *)buffer; \
}

#define m_AVSV_LENVAL_TO_PARAM(param,buffer,saname) \
m_AVSV_OCTVAL_TO_PARAM(param,buffer,saname.length,saname.value)

   /* since it is 64 bit length fill 8 */
#define m_AVSV_UNS64_TO_PARAM(param,buffer,val64) \
{\
   param->i_fmat_id = NCSMIB_FMAT_OCT; \
   param->i_length = 8; \
   param->info.i_oct = (uns8 *)buffer; \
   m_NCS_OS_HTONLL_P(param->info.i_oct,val64); \
}

#define NCS_SERVICE_AVSV_COMMON_SUB_ID_DEFAULT_VAL 1
#define SA_AMF_PRESENCE_ORPHANED (SA_AMF_PRESENCE_TERMINATION_FAILED+1)

#endif
