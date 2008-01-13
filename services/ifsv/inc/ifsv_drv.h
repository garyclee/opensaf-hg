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
FILE NAME: IFSV_DRV.H
DESCRIPTION: Macros, Structures and Function prototypes used by IfSv Driver.
******************************************************************************/
#if 1

#ifndef IFSV_DRV_H
#define IFSV_DRV_H

#include <stdarg.h>
/* Get the compile time options */
#include "ncs_opt.h"
/* Get General Definations */
#include "gl_defs.h"
#include "t_suite.h"
#include "ncs_svd.h"
#include "usrbuf.h"
#include "ncsft.h"
#include "ncsft_rms.h"
#include "ncs_ubaid.h"
#include "ncsencdec.h"
#include "ncs_stack.h"
#include "ncs_mib.h"
#include "ncsmiblib.h"
#include "ncs_mtbl.h"
#include "ncs_log.h"
#include "ncs_lib.h"
#include "ncsdlib.h"
#include "ncs_edu_pub.h"
/* From targsvcs/common/inc */
#include "mds_papi.h"
#include "ncs_util.h"
#if 0
#include "sysf_mds.h"  
#if (NCS_MDS == 1)
#include "mds_inc.h"
#endif
#endif
#include "ifsv_papi.h"

#define NCS_IFSV_DEF_VRID           (1)

/* Sub service ID of IfSv serices */
typedef enum ifsv_drv_service_sub_id
{
   IFSV_DRV_SVC_SUB_ID_IFDRV_CB = 1,
   IFSV_DRV_SVC_SUB_ID_PORT_TBL,
   IFSV_DRV_SVC_SUB_ID_IFDRV_EVT,
   IFSV_DRV_SVC_SUB_ID_MAX
}IFSV_DRV_SERVICE_SUB_ID;

#define m_MMGR_ALLOC_IFSV_DRV_CB (IFSV_DRV_CB*)m_NCS_MEM_ALLOC(sizeof(IFSV_DRV_CB), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFDRV, \
                                   IFSV_DRV_SVC_SUB_ID_IFDRV_CB)
#define m_MMGR_FREE_IFSV_DRV_CB(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFDRV, \
                                   IFSV_DRV_SVC_SUB_ID_IFDRV_CB)

#define m_MMGR_ALLOC_IFSV_DRV_PORT_TBL (IFSV_DRV_PORT_REG_TBL*)m_NCS_MEM_ALLOC(sizeof(IFSV_DRV_PORT_REG_TBL), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFDRV, \
                                   IFSV_DRV_SVC_SUB_ID_PORT_TBL)
#define m_MMGR_FREE_IFSV_DRV_PORT_TBL(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFDRV, \
                                   IFSV_DRV_SVC_SUB_ID_PORT_TBL)

/* these are the macros used to allocate and free the message send by the 
 * IDIM to Interface hardware driver 
 */
#define m_MMGR_ALLOC_IFSV_DRV_IDIM_MSG (NCS_IFSV_HW_INFO*)m_NCS_MEM_ALLOC(sizeof(NCS_IFSV_HW_INFO), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFDRV, \
                                   IFSV_DRV_SVC_SUB_ID_IFDRV_EVT)
#define m_MMGR_FREE_IFSV_DRV_IDIM_MSG(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFDRV, \
                                   IFSV_DRV_SVC_SUB_ID_IFDRV_EVT)
#define m_MMGR_ALLOC_IFSV_DRV_REQ_MSG (NCS_IFSV_HW_DRV_REQ*)m_NCS_MEM_ALLOC(sizeof(NCS_IFSV_HW_DRV_REQ), \
                                   NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFDRV, \
                                   IFSV_DRV_SVC_SUB_ID_IFDRV_EVT)
#define m_MMGR_FREE_IFSV_DRV_REQ_MSG(ptr) m_NCS_MEM_FREE(ptr,NCS_MEM_REGION_PERSISTENT, \
                                   NCS_SERVICE_ID_IFDRV, \
                                   IFSV_DRV_SVC_SUB_ID_IFDRV_EVT)

#define m_IFSV_DRV_STORE_HDL(hdl) glifsv_drv_hdl=hdl
#define m_IFSV_DRV_GET_HDL        glifsv_drv_hdl

#define IFSV_DRV_PVT_SUBPART_VERSION 1
#define DRV_WRT_IFND_SUBPART_VER_AT_MIN_MSG_FMT 1
#define DRV_WRT_IFND_SUBPART_VER_AT_MAX_MSG_FMT 1
#define DRV_WRT_IFND_SUBPART_VER_RANGE             \
        (DRV_WRT_IFND_SUBPART_VER_AT_MAX_MSG_FMT - \
         DRV_WRT_IFND_SUBPART_VER_AT_MIN_MSG_FMT +1)

typedef struct ifsv_drv_cb
{
   uns32            node_id;       /* Node ID where this lib is going to be located    */
   uns32            cb_hdl;        /* Drv control block handle           */
   uns32            vrid;          /* Virtual router ID                                */
   MDS_DEST         my_dest;       /* Absoulte Address of this node                    */
   MDS_HDL           mds_hdl;       /* MDS handle of this Interface Hardware DRV module */
   NCS_BOOL         ifnd_up;       /* Check whether IDIM is UP(TRUE)/DOWN(FALSE)       */   
   EDU_HDL          edu_hdl;       /* EDU handle, for performing encode/decode operations. */
   MDS_DEST         ifnd_dest;     /* Adest to communicate with the IDIM             */
   NCS_LOCK         port_reg_lock; /* This is lock to safe gaurd the port registeration table */
   NCS_DB_LINK_LIST port_reg_tbl;  /* This is the registration table which holds the registration information of the hardware driver for the specific ports */
} IFSV_DRV_CB;

typedef struct ifsv_drv_port_reg_tbl
{
   NCS_DB_LINK_LIST_NODE   q_node;
   NCS_IFSV_PORT_REG       port_reg;   
} IFSV_DRV_PORT_REG_TBL;

/****** Function Prototype ******/
/*****************************************************************************
 * routine used by the Hardware driver to register with the IDIM.
 *****************************************************************************/
uns32 ifsv_register_hw_driver(NCS_IFSV_PORT_REG *reg_msg);

/*****************************************************************************
 * routine which Hardware drv used to send any information to IDIM.
 *****************************************************************************/
uns32 ifsv_send_intf_info(NCS_IFSV_HW_INFO *stats_msg);

/*****************************************************************************
 * routine which needs to be called for IfSv driver initialization.
 *****************************************************************************/
uns32 ifsv_drv_init (uns32 vrid, uns8 pool_id);


/*****************************************************************************
 * routine which needs to be called for IfSv driver destroy.
 *****************************************************************************/
uns32 ifsv_drv_destroy(uns32 vrid);

/*******MDS prototypes *******/
int ifsv_drv_hw_resp_evt_test_type_fnc(NCSCONTEXT arg);
int ifsv_drv_hw_req_evt_test_type_fnc(NCSCONTEXT arg);

uns32 ifsv_drv_edp_intf_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                         NCSCONTEXT ptr, uns32 *ptr_data_len, 
                         EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                         EDU_ERR *o_err);
uns32 ifsv_drv_edp_idim_ifnd_up_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                                 NCSCONTEXT ptr, uns32 *ptr_data_len, 
                                 EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                                 EDU_ERR *o_err);
uns32 ifsv_drv_edp_idim_hw_req(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                           NCSCONTEXT ptr, uns32 *ptr_data_len, 
                           EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                           EDU_ERR *o_err);
uns32 ifsv_drv_edp_idim_port_type(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                              NCSCONTEXT ptr, uns32 *ptr_data_len, 
                              EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                              EDU_ERR *o_err);
uns32 ifsv_drv_edp_idim_hw_req_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                                NCSCONTEXT ptr, uns32 *ptr_data_len, 
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                                EDU_ERR *o_err);
uns32 ifsv_drv_edp_idim_port_status(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                                NCSCONTEXT ptr, uns32 *ptr_data_len, 
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                                EDU_ERR *o_err);
uns32 ifsv_drv_edp_idim_port_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                              NCSCONTEXT ptr, uns32 *ptr_data_len, 
                              EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                              EDU_ERR *o_err);
uns32 ifsv_drv_edp_stats_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                          NCSCONTEXT ptr, uns32 *ptr_data_len, 
                          EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                          EDU_ERR *o_err);
uns32 ifsv_drv_edp_idim_port_stats(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                                NCSCONTEXT ptr, uns32 *ptr_data_len, 
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                                EDU_ERR *o_err);
uns32 ifsv_drv_edp_idim_hw_rcv_info(EDU_HDL *edu_hdl, EDU_TKN *edu_tkn, 
                                NCSCONTEXT ptr, uns32 *ptr_data_len, 
                                EDU_BUF_ENV *buf_env, EDP_OP_TYPE op, 
                                EDU_ERR *o_err);

#endif /* IFSV_DRV_H */
#endif
