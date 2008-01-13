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

#if 1


/*****************************************************************************
 * This is the function prototype of the IDIM Event handler.
 *****************************************************************************/
/* forward decleration */
struct ifsv_idim_evt;
typedef uns32 (*IFSV_IDIM_EVT_HANDLER) (struct ifsv_idim_evt* evt, struct ifsv_idim_cb *idim_cb);

/*****************************************************************************
 * IDIM event enum type (Events which IDIM would receive ).
 *****************************************************************************/
typedef enum ifsv_idim_evt_type
{
   IFSV_IDIM_EVT_BASE, 
   IFSV_IDIM_EVT_RECV_HW_DRV_MSG = IFSV_IDIM_EVT_BASE,
   IFSV_IDIM_EVT_GET_HW_STATS,       /* get interface statistics */
   IFSV_IDIM_EVT_SET_HW_PARM,        /* Set interface parameters */
   IFSV_IDIM_EVT_HEALTH_CHK,         /* do health check on idim  */
   IFSV_IDIM_EVT_MAX
} IFSV_IDIM_EVT_TYPE;


/*****************************************************************************
 * Structure used by the IfND to request IDIM for the HW status..
 *****************************************************************************/
typedef struct ifsv_idim_evt_get_hw_stats_info
{
   NCSMDS_SVC_ID           app_svc_id;
   MDS_DEST                app_dest;
   NCS_IFSV_SPT            slot_port;
   uns32                   usr_hdl;
} IFSV_IDIM_EVT_GET_HW_STATS_INFO;

/*****************************************************************************
 * Structure used by the IfND to request IDIM to set HW paramaters
 *****************************************************************************/
typedef struct ifsv_idim_evt_set_hw_param_info
{   
   NCS_IFSV_SPT            slot_port;
   NCS_IFSV_INTF_INFO      hw_param;
} IFSV_IDIM_EVT_SET_HW_PARAM_INFO;

/*****************************************************************************
 * Structure used by the IfND to do health check on IDIM.
 *****************************************************************************/
typedef struct ifsv_idim_evt_health_chk_info
{
   NCSCONTEXT sem;
   SaAmfHealthcheckInvocationT chk_type;
}IFSV_IDIM_EVT_HEALTH_CHK_INFO;


/*****************************************************************************
 * IDIM event structure.
 *****************************************************************************/
typedef struct ifsv_idim_evt
{
   struct ifsv_idim_evt *next;
   IFSV_IDIM_EVT_TYPE evt_type;   
   NCS_IPC_PRIORITY    priority;
   union
   {
      NCS_IFSV_HW_INFO                hw_info;       /* IFSV_IDIM_EVT_RECV_HW_DRV_MSG */
      IFSV_IDIM_EVT_GET_HW_STATS_INFO get_stats;     /* IFSV_IDIM_EVT_GET_HW_STATS    */
      IFSV_IDIM_EVT_HEALTH_CHK_INFO   health_chk;    /* IFSV_IDIM_EVT_HEALTH_CHK      */
      IFSV_IDIM_EVT_SET_HW_PARAM_INFO set_hw_parm;   /* IFSV_IDIM_EVT_SET_HW_PARM     */
   } info;
} IFSV_IDIM_EVT;

/****** IDIM Function prototype *****/
void
idim_process_mbx (SYSF_MBX *mbx);
void
idim_evt_destroy (IFSV_IDIM_EVT *evt);
uns32
idim_send_ifnd_up_evt (MDS_DEST *to_dest, IFSV_CB *cb);
uns32 
idim_send_hw_drv_msg (void* info, NCS_IFSV_HW_DRV_MSG_TYPE msg_type, 
                      MDS_DEST *to_card, MDS_HDL mds_hdl);

#endif
