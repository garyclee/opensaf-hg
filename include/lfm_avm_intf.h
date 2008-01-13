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

  $Header:  $

  MODULE NAME: lfm_avm_intf.h

..............................................................................

  DESCRIPTION:  This file declares the interface for LFM and AvM.

******************************************************************************
*/

#ifndef LFM_AVM_INTF_H
#define LFM_AVM_INTF_H

/*
** Includes
*/
#include <stdio.h>
#include "ncsgl_defs.h"

#include "ncs_svd.h"
#include "SaHpi.h"
#include "mds_papi.h"
#include "ncs_ubaid.h"

/* Message format versions  */
#define NCS_AVM_LFM_MSG_FMT_VER_1    1

typedef struct avm_lfm_fru_hsstate_info
{
   SaHpiEntityPathT  fru;
   SaHpiHsStateT  hs_state;
   struct avm_lfm_fru_hsstate_info *next;
}AVM_LFM_FRU_HSSTATE_INFO;

typedef enum
{
   HEALTH_STATUS_PLANE_A = 0,
   HEALTH_STATUS_PLANE_B,   
   HEALTH_STATUS_SAM_A,
   HEALTH_STATUS_SAM_B,
   HEALTH_STATUS_PLANE_A_SAM_A,
   HEALTH_STATUS_PLANE_B_SAM_B,
   HEALTH_STATUS_NONE
} HEALTH_STATUS_ENTITY;

typedef enum
{
   STATUS_UNHEALTHY,
   STATUS_HEALTHY,
   STATUS_NONE
}HEALTH_STATUS;

typedef struct health_status_update_info
{
   HEALTH_STATUS_ENTITY plane;
   HEALTH_STATUS        plane_status;
   HEALTH_STATUS_ENTITY sam;
   HEALTH_STATUS        sam_status;
}HEALTH_STATUS_UPDATE_INFO;


/* Message types for messages between AvM and LFM */

typedef enum
{
   /* Messages from AvM --> LFM */
   AVM_LFM_HEALTH_STATUS_RSP = 1,
   AVM_LFM_NODE_RESET_IND,
   AVM_LFM_FRU_HSSTATUS,
   AVM_LFM_MSG_MAX,

   /* Messages from LFM --> AVM */
   LFM_AVM_HEALTH_STATUS_REQ = AVM_LFM_MSG_MAX,
   LFM_AVM_HEALTH_STATUS_UPD,
   LFM_AVM_HEALTH_ALL_FRU_HS_REQ,
   LFM_AVM_NODE_RESET_IND,
   LFM_AVM_SWOVER_REQ,
   LFM_AVM_MSG_MAX
}LFM_AVM_MSG_TYPE;




typedef struct lfm_avm_msg
{
   LFM_AVM_MSG_TYPE msg_type;
   union
   {
       HEALTH_STATUS_UPDATE_INFO    hlt_status_rsp;
       /* this entity path is for node reset indication 
        * msg in both directions */
       SaHpiEntityPathT      node_reset_ind; 
       AVM_LFM_FRU_HSSTATE_INFO  *fru_hsstate_info;
       
       HEALTH_STATUS_ENTITY          hlt_status_req;
       HEALTH_STATUS_UPDATE_INFO    hlt_status_upd;
   }info;
}LFM_AVM_MSG, LFM_AVM_MSG_T;

/* MEMORY related sub-ids add enum when necessary for now just one*/
#define LFM_AVM_MEM_SUB_ID 1
#define LFM_AVM_MEM_FRU_HS_SUB_ID 2

/* Add service_ids 
 * 
 * NCS_SERVICE_ID_LFM_AVM to enum NCS_SERVICE_ID in 
 * base/leap_basic/pub_inc/ncs_svd.h
 */

#define m_MMGR_ALLOC_LFM_AVM_MSG   (LFM_AVM_MSG *)m_NCS_MEM_ALLOC(sizeof(LFM_AVM_MSG), \
      NCS_MEM_REGION_PERSISTENT, \
      NCS_SERVICE_ID_LFM_AVM, \
      LFM_AVM_MEM_SUB_ID)

#define m_MMGR_FREE_LFM_AVM_MSG(p)   m_NCS_MEM_FREE(p, \
      NCS_MEM_REGION_PERSISTENT, \
      NCS_SERVICE_ID_LFM_AVM, \
      LFM_AVM_MEM_SUB_ID)

#define m_MMGR_ALLOC_LFM_AVM_FRU_HSSTATE_INFO (AVM_LFM_FRU_HSSTATE_INFO *)m_NCS_MEM_ALLOC(sizeof(AVM_LFM_FRU_HSSTATE_INFO), \
      NCS_MEM_REGION_PERSISTENT, \
      NCS_SERVICE_ID_LFM_AVM, \
      LFM_AVM_MEM_FRU_HS_SUB_ID)

#define m_MMGR_FREE_LFM_AVM_FRU_HSSTATE_INFO(p)   m_NCS_MEM_FREE(p, \
      NCS_MEM_REGION_PERSISTENT, \
      NCS_SERVICE_ID_LFM_AVM, \
      LFM_AVM_MEM_FRU_HS_SUB_ID)
                     
/* Function declarations */
EXTERN_C uns32 lfm_avm_mds_enc(MDS_CALLBACK_ENC_INFO *);
EXTERN_C uns32 lfm_avm_mds_dec(MDS_CALLBACK_DEC_INFO *);
EXTERN_C uns32 lfm_avm_mds_cpy(MDS_CALLBACK_COPY_INFO *);

void lfm_avm_msg_free(LFM_AVM_MSG *);

#endif /* LFM_AVM_INTF_H */
