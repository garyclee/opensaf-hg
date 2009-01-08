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

#ifndef IFSV_TMR_H
#define IFSV_TMR_H
#include "ifsv.h"
/*****************************************************************************
 * Macro for aging timer period, suppose to be high.
 *****************************************************************************/
/* This is the should be large just for Unit Testing it has been kept as 30 sec  - TBD*/
#define IFSV_CLEANUP_TMR_PERIOD (1000)
/* Timer value to start when IfND goes DOWN. */
#define NCS_IFSV_IFD_RET_TMR_PERIOD (4000)
/* Time period for getting MDS_DEST of all IfA and Drivers. It is proposed as
   2 seconds. */
#define NCS_IFSV_IFND_MDS_DEST_GET_TMR_PERIOD (200)

/* Time period for stdby IfD getting active assignment */
/* for First pass this value is kept 30 secs for testing */
#define NCS_IFSV_IFD_IFND_FLUSH_TMR_PERIOD  (3000) 

/*****************************************************************************
 * IfSv interface timer structure.
 *****************************************************************************/
typedef enum ncs_ifsv_tmr_type
{
   /* Timer type for IFND. */
   NCS_IFSV_IFND_MDS_DEST_GET_TMR = 1,
   NCS_IFSV_IFND_EVT_INTF_AGING_TMR,

   /* Timer type for IFD.*/
   NCS_IFSV_IFD_EVT_INTF_AGING_TMR,
   NCS_IFSV_IFD_EVT_RET_TMR, /*Retention timer for IfND, when it goes down. */
   NCS_IFSV_IFD_IFND_REC_FLUSH_TMR,
   NCS_IFSV_TMR_MAX
} NCS_IFSV_TMR_TYPE;

typedef struct ifsv_tmr
{
  NCS_IFSV_TMR_TYPE  tmr_type; /* Types of timers. To be filled and used by 
                                  user.*/
  NCS_SERVICE_ID svc_id; /* IfD or IfND. To be filled and used by user.*/

  union /* To be filled and used by user. */
  {
    /* Based on timer type, we can use the following fields. */
    uns32  ifindex; /* For NCS_IFSV_IFND_EVT_INTF_AGING_TMR and 
                       NCS_IFSV_IFD_EVT_INTF_AGING_TMR.*/
    NODE_ID node_id; /* Used for IFND node id when it goes down. */
    uns32  reserved; /* Right now, not in used. Added just for uniformity. For
                        NCS_IFSV_IFND_MDS_DEST_GET_TMR */
  } info;

  tmr_t  id; /* For internal use. Not to be filled by the user. */    
  uns32  uarg; /* For internal use. Not to be filled by the user. */
  NCS_BOOL  is_active; /* For internal use. Not to be filled by the user. */
} IFSV_TMR;


#endif /* IFSV_TMR_H*/
