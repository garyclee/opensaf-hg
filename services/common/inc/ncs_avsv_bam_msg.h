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

  This file lists definitions for messages between AVSV and BAM modules.
  
******************************************************************************
*/

#ifndef NCS_AVSV_BAM_MSG_H
#define NCS_AVSV_BAM_MSG_H

/* Message format versions  */
#define AVSV_AVD_BAM_MSG_FMT_VER_1    1


typedef enum
{
   /* Put all the messages from BAM to AVD first */
   BAM_AVD_CFG_DONE_MSG, 
   AVD_BAM_CFG_RDY,
   AVD_BAM_MSG_RESTART,
   AVD_BAM_MSG_MAX,
} AVD_BAM_MSG_TYPE;

/* This message will be same for both BAM and PSR */
typedef struct avsv_avd_cfg_done{
         uns32                check_sum; /* number of cfg messages sent */
} AVSV_AVD_CFG_DONE;

/* Message structure used by AVD for communication between
 * the active AVD and BAM.
 */
typedef struct avd_bam_msg
{
   AVD_BAM_MSG_TYPE msg_type;
   union {
      AVSV_AVD_CFG_DONE msg;
   } msg_info;
} AVD_BAM_MSG;

#define NCS_SERVICE_BAM_AVD_SUB_ID_DEFAULT_VAL 1

#define m_MMGR_ALLOC_AVD_BAM_MSG (AVD_BAM_MSG *)m_NCS_MEM_ALLOC(sizeof(AVD_BAM_MSG), \
                                     NCS_MEM_REGION_PERSISTENT,\
                                     NCS_SERVICE_ID_BAM_AVD,\
                                     NCS_SERVICE_BAM_AVD_SUB_ID_DEFAULT_VAL)

#define m_MMGR_FREE_AVD_BAM_MSG(p) m_NCS_MEM_FREE(p, \
                                     NCS_MEM_REGION_PERSISTENT, \
                                     NCS_SERVICE_ID_BAM_AVD,\
                                     NCS_SERVICE_BAM_AVD_SUB_ID_DEFAULT_VAL)
                                   


#endif

