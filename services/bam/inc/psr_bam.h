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

  MODULE NAME: PSR_BAM.H

..............................................................................

  DESCRIPTION: Contains definitions for PSSv definitions and constants required
               by PSS for interacting with BAM instance.


******************************************************************************
*/

#ifndef PSR_BAM_H
#define PSR_BAM_H

#include "psr_def.h"
#include "ncs_ubaid.h"
#include "mds_papi.h"


/* Message format versions  */
#define NCS_BAM_PSS_MSG_FMT_VER_1    1

typedef enum
{
    PSS_BAM_EVT_WARMBOOT_REQ,
    PSS_BAM_EVT_CONF_DONE,
    PSS_BAM_EVT_MAX
}PSS_BAM_EVT;

typedef enum
{
   BAM_MEM_PSS_BAM_MSG,
   BAM_MEM_PSS_BAM_WARMBOOT_REQ,
   BAM_MEM_PCN_STRING 
}BAM_PSS_MEM_ID;

/* Warmboot Request(of linked-list type) to BAM */
typedef struct pss_bam_warmboot_req_tag
{
  char *pcn;

  /* MIB table-IDs can be added here, when required. */

  struct pss_bam_warmboot_req_tag *next;
}PSS_BAM_WARMBOOT_REQ;


/* Config done event from BAM to PSS */
typedef struct pss_bam_conf_done_tag
{
  MAB_PSS_CLIENT_LIST pcn_list;
}PSS_BAM_CONF_DONE; 


typedef struct pss_bam_msg_tag
{
   struct pss_bam_msg_tag    *next;     /* for a linked list of them */
   PSS_BAM_EVT               i_evt;

   NCSCONTEXT                yr_hdl;  /* MDS has this value as its MAC/MAS/OAC handle */
   NCSCONTEXT                pwe_hdl; /* PWE of the sender. */

   union {
      PSS_BAM_WARMBOOT_REQ   warmboot_req;
      PSS_BAM_CONF_DONE      conf_done;
   }info;
}PSS_BAM_MSG;


#define m_MMGR_ALLOC_PSS_BAM_MSG (PSS_BAM_MSG*)m_NCS_MEM_ALLOC(sizeof(PSS_BAM_MSG), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_BAM, BAM_MEM_PSS_BAM_MSG)

#define m_MMGR_ALLOC_PSS_BAM_WARMBOOT_REQ  (PSS_BAM_WARMBOOT_REQ*)m_NCS_MEM_ALLOC(sizeof(PSS_BAM_WARMBOOT_REQ), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_BAM, BAM_MEM_PSS_BAM_WARMBOOT_REQ)

#define m_MMGR_ALLOC_BAM_PCN_STRING(cnt)  (char *)m_NCS_MEM_ALLOC((sizeof(char)*cnt), \
                                   NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, BAM_MEM_PCN_STRING)

#define m_MMGR_FREE_PSS_BAM_MSG(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_BAM, BAM_MEM_PSS_BAM_MSG)

#define m_MMGR_FREE_PSS_BAM_WARMBOOT_REQ(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_BAM, BAM_MEM_PSS_BAM_WARMBOOT_REQ)

#define m_MMGR_FREE_BAM_PCN_STRING(p)     m_NCS_MEM_FREE(p, NCS_MEM_REGION_TRANSIENT, \
                                   NCS_SERVICE_ID_MAB, BAM_MEM_PCN_STRING)

EXTERN_C MABCOM_API uns32     pss_bam_mds_enc(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT msg, 
                               SS_SVC_ID to_svc,     NCS_UBAID* uba);
EXTERN_C MABCOM_API uns32     pss_bam_mds_snd(NCSCONTEXT mds_hdl,    NCSCONTEXT  msg,
                                          MDS_SVC_ID fr_svc,     MDS_SVC_ID  to_svc,
                                          MDS_DEST   to_dest);

EXTERN_C MABCOM_API uns32     pss_bam_mds_dec(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT* msg, 
                               SS_SVC_ID to_svc,     NCS_UBAID*  uba);

EXTERN_C MABCOM_API uns32     pss_bam_mds_cpy(MDS_CLIENT_HDL yr_svc_hdl, NCSCONTEXT  msg,
                               SS_SVC_ID to_svc,     NCSCONTEXT*  cpy,
                               NCS_BOOL   last                       );

#endif
