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

/*****************************************************************************
..............................................................................

  MODULE NAME: NCS_SNMPTM.H 

..............................................................................

  DESCRIPTION: This file describes the API structure definitions used to 
               interact with the SNMPTM application. 
..............................................................................


******************************************************************************/
#ifndef  NCS_SNMPTM_H
#define  NCS_SNMPTM_H

#define NSC_SNMPTM 1
/**************************************************************************
           Request types to be made to the SNMPTM LM Routines
**************************************************************************/
typedef enum
{
   NCSSNMPTM_LM_REQ_SNMPTM_CREATE,   /* to create SNMPTM */
   NCSSNMPTM_LM_REQ_SNMPTM_DESTROY,  /* to destroy SNMPTM */
   NCSSNMPTM_LM_REQ_MAX
} NCSSNMPTM_LM_REQ_TYPE;

/**************************************************************************
                        Create SNMPTM structure 
**************************************************************************/
typedef struct ncssnmptm_lm_snmptm_create_req_info
{
   uns32       i_oac_hdl;        /* OAC handle Info */
   NCSCONTEXT  i_mds_vdest;      /* MDS absolute destination */
   MDS_DEST    i_vdest;          /* Holds VCARD info */
   uns8        i_node_id;        /* Node Id,to make instance context str*/
   uns8        i_hmpool_id;      /* Handle Manager Variable */
   char        i_pcn[256];       /* PSSv Client Name */
} NCSSNMPTM_LM_SNMPTM_CREATE_REQ_INFO;

/**************************************************************************
                    The SNMPTM LM Request Structure 
**************************************************************************/
typedef struct ncssnmptm_lm_request_info
{
   NCSSNMPTM_LM_REQ_TYPE  i_req;
   NCSSNMPTM_LM_SNMPTM_CREATE_REQ_INFO  io_create_snmptm;

} NCSSNMPTM_LM_REQUEST_INFO;


/*************************************************************************
                           SNMPTM APIs
*************************************************************************/
EXTERN_C uns32 ncssnmptm_lm_request (struct ncssnmptm_lm_request_info*);

#endif  /* NCS_SNMPTM_H */



