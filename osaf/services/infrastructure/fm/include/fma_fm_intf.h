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

DESCRIPTION:

******************************************************************************
*/

#ifndef FMA_FM_INTF_H
#define FMA_FM_INTF_H

#define m_MSG_FMT_VER_GET(rem_ver,my_ver_min,my_ver_max,msg_fmt_array) \
   (((rem_ver) < (my_ver_min))?0:\
   (((rem_ver) >= (my_ver_max))?((msg_fmt_array)[(my_ver_max) - (my_ver_min)]):\
   ((msg_fmt_array)[(rem_ver) - (my_ver_min)])))

#define m_MSG_FORMAT_IS_VALID(                                    \
   msg_fmt_version,rem_ver_min,rem_ver_max,msg_fmt_array) \
   (((msg_fmt_version) > (msg_fmt_array)[(rem_ver_max) - (rem_ver_min)])?\
   0:(((msg_fmt_version) < (msg_fmt_array)[0])?0:1))

typedef enum {
	/* following three sent by AVM to FM */
	FMA_FM_EVT_HB_LOSS,
	FMA_FM_EVT_HB_RESTORE,
	FMA_FM_EVT_CAN_SMH_SW,
	/* sent by AVM to FM and vice-versa */
	FMA_FM_EVT_NODE_RESET_IND,
	/* sent by FM to AVM */
	FMA_FM_EVT_SYSMAN_SWITCH_REQ,
	FMA_FM_EVT_SMH_SW_RESP,
	/* Request type MAX */
	FMA_FM_EVT_MAX
} FMA_FM_MSG_TYPE;

typedef enum {
	FMA_FM_SMH_CAN_SW,
	FMA_FM_SMH_CANNOT_SW
} FMA_FM_SMH_SW_RESP;

typedef struct {
	uns8 slot;
	uns8 site;
} FMA_FM_PHY_ADDR;

typedef struct fma_fm_msg {
	FMA_FM_MSG_TYPE msg_type;
	union {
		FMA_FM_SMH_SW_RESP response;
		FMA_FM_PHY_ADDR phy_addr;
	} info;
} FMA_FM_MSG;

typedef struct fma_mbx_evt {
	MDS_DEST fr_dest;
	FMA_FM_MSG *msg;
} FMA_MBX_EVT_T;

#define FMA_MBX_EVT_NULL ((FMA_MBX_EVT_T *)0)
#define FMA_FM_MSG_NULL  ((FMA_FM_MSG *)0)

#define FMA_FM_MEM_SUB_ID  1

#define m_MMGR_ALLOC_FMA_FM_MSG  (FMA_FM_MSG*)m_NCS_MEM_ALLOC(sizeof(FMA_FM_MSG), \
                                              NCS_MEM_REGION_PERSISTENT, \
                                              NCS_SERVICE_ID_FMA_GFM, \
                                              FMA_FM_MEM_SUB_ID)

#define m_MMGR_FREE_FMA_FM_MSG(p)     m_NCS_MEM_FREE(p,\
                                              NCS_MEM_REGION_PERSISTENT, \
                                              NCS_SERVICE_ID_FMA_GFM, \
                                              FMA_FM_MEM_SUB_ID)

/*****************************************************************************
                       Extern Function Declarations
*****************************************************************************/
EXTERN_C uns32 fma_fm_mds_cpy(MDS_CALLBACK_COPY_INFO *cpy_info);
EXTERN_C uns32 fma_fm_mds_enc(MDS_CALLBACK_ENC_INFO *enc_info);
EXTERN_C uns32 fma_fm_mds_dec(MDS_CALLBACK_DEC_INFO *dec_info);

#endif
