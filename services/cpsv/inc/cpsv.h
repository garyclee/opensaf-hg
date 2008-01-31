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

  This module is the main include file for the entire CheckPoint Service.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef CPSV_H
#define CPSV_H

#include "ncsgl_defs.h"

#include "t_suite.h"
#include "ncs_saf.h"
#include "ncs_mib.h"
#include "ncs_lib.h"
#include "mds_papi.h"
#include "ncs_edu_pub.h"
#include "ncs_mib_pub.h"
#include "ncsmiblib.h"
#include "ncs_main_pvt.h"

#if ( (NCS_CPD_LOG == 1) || (NCS_CPND_LOG == 1) || (NCS_CPA_LOG ==1) || (NCS_CPSV_LOG == 1) )
#include "ncs_log.h"
#include "dta_papi.h"
#endif

/* EDU Includes... */
#include "ncs_edu_pub.h"
#include "ncsencdec_pub.h"
#include "ncs_saf_edu.h"

/* Versioning 3.0.B related common Macros */
#include "ncs_util.h"

#include "cpsv_evt.h"
#include "cpsv_mem.h"
/* CPSV Common Macros */
/*** Macro used to get the AMF version used ****/
#define m_CPSV_GET_AMF_VER(amf_ver) amf_ver.releaseCode='B'; amf_ver.majorVersion=0x01; amf_ver.minorVersion=0x01;

#define m_COMPARE_CREATE_ATTR(attr1, attr2)                       \
  (((attr1)->creationFlags == (attr2)->creationFlags) &&          \
   ((attr1)->checkpointSize == (attr2)->checkpointSize) &&        \
   ((attr1)->retentionDuration == (attr2)->retentionDuration) &&  \
   ((attr1)->maxSections == (attr2)->maxSections) &&              \
   ((attr1)->maxSectionSize == (attr2)->maxSectionSize) &&        \
   ((attr1)->maxSectionIdSize == (attr2)->maxSectionIdSize))      \

#define m_IS_SA_CKPT_CHECKPOINT_COLLOCATED(attr)                  \
    ((attr)->creationFlags & SA_CKPT_CHECKPOINT_COLLOCATED)
                                   
#define m_IS_ASYNC_UPDATE_OPTION(attr)                            \
   ((attr)->creationFlags & (SA_CKPT_WR_ACTIVE_REPLICA | SA_CKPT_WR_ACTIVE_REPLICA_WEAK))

#define FUNC_NAME(DS) \
       cpsv_edp_##DS##_info

#define ARRAY_NAME(DS) \
       cpsv_edp_##DS##_rules

#define TEST_FUNC(DS) \
       cpsv_edp_##DS##_fnc

#define FUNC_DECLARATION(DS)   uns32 \
                   FUNC_NAME(DS)(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, \
                   NCSCONTEXT ptr, uns32 *ptr_data_len, \
                   EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, \
                   EDU_ERR *o_err) 



#define TEST_FUNC_DECLARATION(DS) uns32 \
                         TEST_FUNC(DS)(NCSCONTEXT arg)

#define NCS_ENC_DEC_DECLARATION(DS) \
                 uns32 rc = NCSCC_RC_SUCCESS; \
                 DS   *struct_ptr = NULL, **d_ptr = NULL

#define NCS_ENC_DEC_ARRAY(DS) \
         EDU_INST_SET      ARRAY_NAME(DS)[] =

#define NCS_ENC_DEC_REM_FLOW(DS)  \
 \
   switch (op) \
   { \
      case EDP_OP_TYPE_ENC: \
         struct_ptr = (DS *)ptr; \
         break; \
                 \
      case EDP_OP_TYPE_DEC: \
          d_ptr = (DS **)ptr;  \
          if(*d_ptr == NULL)                          \
          { \
             *o_err = EDU_ERR_MEM_FAIL;               \
             return NCSCC_RC_FAILURE;                 \
          }                                           \
          m_NCS_MEMSET(*d_ptr, '\0', sizeof(DS)); \
          struct_ptr = *d_ptr;                                \
         break; \
                   \
      default:  \
         struct_ptr = ptr;                                   \
         break; \
      }  \
   \
   rc = m_NCS_EDU_RUN_RULES(edu_hdl, edu_tkn,ARRAY_NAME(DS), \
       struct_ptr, ptr_data_len, buf_env, op, o_err); \
   \
   return rc;



/* DTSv version support */
#define CPSV_LOG_VERSION 1 

#define m_NCS_OS_SPRINTF         sprintf

#define m_CPSV_CONVERT_SATIME_TEN_MILLI_SEC(t)      (t)/(10000000) /* 10^7 */


#endif  /* CPSV_H */
