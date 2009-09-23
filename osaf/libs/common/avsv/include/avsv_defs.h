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

/* readiness state definition */
typedef enum ncs_readiness_state {
	NCS_OUT_OF_SERVICE = 1,
	NCS_IN_SERVICE = 2,
	NCS_STOPPING = 3
} NCS_READINESS_STATE;

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

typedef enum ncs_pres_state_tag {
	NCS_PRES_UNINSTANTIATED = 1,
	NCS_PRES_INSTANTIATING,
	NCS_PRES_INSTANTIATED,
	NCS_PRES_TERMINATING,
	NCS_PRES_RESTARTING,
	NCS_PRES_INSTANTIATIONFAILED,
	NCS_PRES_TERMINATIONFAILED,
	NCS_PRES_ORPHANED,
	NCS_PRES_MAX
} NCS_PRES_STATE;

typedef enum {
	AVSV_SG_RED_MODL_2N = 1,
	AVSV_SG_RED_MODL_NPM,
	AVSV_SG_RED_MODL_NWAY,
	AVSV_SG_RED_MODL_NWAYACTV,
	AVSV_SG_RED_MODL_NORED
} SaReduntantModelT;

/* 
 * SaAmfRecommendedRecoveryT definition does not include escalated recovery
 * actions like su-restart, su-failover etc. The following enum definition
 * captures them..
 */
typedef enum avsv_err_rcvr {
	/* recovery specified in SaAmfRecommendedRecoveryT */
	AVSV_AMF_NO_RECOMMENDATION = 1,
	AVSV_AMF_COMPONENT_RESTART = 2,
	AVSV_AMF_COMPONENT_FAILOVER = 3,
	AVSV_AMF_NODE_SWITCHOVER = 4,
	AVSV_AMF_NODE_FAILOVER = 5,
	AVSV_AMF_NODE_FAILFAST = 6,
	AVSV_AMF_CLUSTER_RESET = 7,

	/* escalated recovery */
	AVSV_ERR_RCVR_SU_RESTART,
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

/* Macro for AvSv assert */
#define m_AVSV_ASSERT(condition) if(!(condition)) assert(0)

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

/* Macros for allocating and freeing memory in different sub modules of avsv */

#define NCS_SERVICE_AVSV_COMMON_SUB_ID_DEFAULT_VAL 1

#define m_MMGR_ALLOC_AVSV_COMMON_DEFAULT_VAL(mem_size)  m_NCS_MEM_ALLOC( \
                                                mem_size, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVSV, \
                                                NCS_SERVICE_AVSV_COMMON_SUB_ID_DEFAULT_VAL)
#define m_MMGR_FREE_AVSV_COMMON_DEFAULT_VAL(p)  m_NCS_MEM_FREE(p, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_AVSV, \
                                                NCS_SERVICE_AVSV_COMMON_SUB_ID_DEFAULT_VAL)

#endif
