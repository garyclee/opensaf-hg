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

  MODULE NAME: SNMPTM_CB.H 

..............................................................................

  DESCRIPTION: Defines Control Block structure for SNMPTM

..............................................................................

******************************************************************************/

#ifndef  SNMPTM_CB_H
#define  SNMPTM_CB_H

/******************************************************************************
                       SNMPTM Control Block structure  
******************************************************************************/
typedef struct snmptm_cb
{
   uns32            oac_hdl;     /* OAC handle info */
   MDS_DEST         vcard;
   NCSCONTEXT       mds_vdest;   /* MDS virtual dest */
   uns8             node_id;     /* Node Id, to make instance context str used
                                    as an OAC index */
   char             pcn_name[256]; /* PSSv Client Name */

   /* Control Block Lock, Access permissions are NCS_LOCK_WRITE and NCS_LOCK_READ
    * NCS_LOCK_WRITE: A truly mutual exclusion request. The lock will be pending
    *               until all other tasks with any semaphore ownership releases
    *               the ownership.
    * NCS_LOCK_READ:  This request will be pending only if another task has write
    *                ownership. It is allowable to have multiple read ownerships.
    */
   NCS_LOCK          snmptm_cb_lock;

   SNMPTM_SCALAR_TBL scalars;       /* holds scalar table information   */
   NCS_PATRICIA_TREE tblone_tree;   /* Holds table one information      */
   NCS_PATRICIA_TREE tblthree_tree; /* Holds table three information    */
   SNMPTM_TBLFOUR    *tblfour;      /* Holds table four information     */
   NCS_PATRICIA_TREE tblfive_tree;  /* Holds table five information     */
   NCS_PATRICIA_TREE tblsix_tree;   /* Holds table six information      */
   SNMPTM_TBLSEVEN    *tblseven;    /* Holds table seven information    */
   NCS_PATRICIA_TREE tbleight_tree; /* Holds table eight information      */
   NCS_PATRICIA_TREE tblten_tree;   /* Holds table ten information      */


   uns32             hmcb_hdl;    /* Handle Manager Struct handle   */
   uns8              hmpool_id;   /* Handle Manager Variable        */

   EDU_HDL           edu_hdl;  /* EDU handle for performing
                                  encode/decode operations */

   /*************************************************************
                       EDA related params 
   *************************************************************/
   SaVersionT  safVersion; /* version info of the SAF implementation */

   SaEvtHandleT     evtHandle;   /* EDA Handle */
   SaNameT          publisherName; /* Publisher name */

   SaNameT             evtChannelName;   /* EDA Channel Name */
   SaEvtChannelHandleT evtChannelHandle; /* EDA Channel Handle */

   /* Event Filtering Pattern string */
   SaUint8T     evtPatternStr[SNMPTM_EDA_EVT_FILTER_PATTERN_LEN+1];

   SaEvtEventFilterT       evtFilter;  /* Event Pattern */
   SaEvtEventFilterArrayT  evtFilters; /* EDA Filter Array */

} SNMPTM_CB;

#define SNMPTM_CB_NULL ((SNMPTM_CB*)0)


/******************************************************************************
                        ANSI Function Prototypes
******************************************************************************/
EXTERN_C SNMPTM_CB *snmptm_cb_create(void);
EXTERN_C void      snmptm_cb_destroy(SNMPTM_CB *);
EXTERN_C uns32     snmptm_eda_finalize(struct snmptm_cb *snmptm_cb);
EXTERN_C uns32     snmptm_eda_initialize(struct snmptm_cb *snmptm_cb);
typedef  uns32     (*NCSSNMPTM_REQ_FNC)(NCSSNMPTM_LM_REQUEST_INFO *req);

#endif  /* SNMPTM_CB_H */
