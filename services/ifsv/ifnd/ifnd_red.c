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
FILE NAME: ifd_red.C
DESCRIPTION: Ifd redundancy routines.
******************************************************************************/

#include "ifsv.h"
#include "ifnd.h"
#include "ifnd_red.h"

static uns32 ifnd_mds_dest_add (MDS_DEST *mds_dest, IFSV_CB *cb, NCSMDS_SVC_ID type);

/****************************************************************************
 * Name          : ifnd_mds_svc_evt_ifa
 *
 * Description   : This callback function which is called when IFAs comes up. 
 *                 This function will prepare the MDS_DEST data base.
 *
 * Arguments     : ifd_dest  - This is the MDS_DEST of the IfA.
 *                 cb        - Ifnd Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifnd_mds_svc_evt_ifa (MDS_DEST *ifa_dest, IFSV_CB *cb)
{
   if (ifnd_mds_dest_add(ifa_dest, cb, NCSMDS_SVC_ID_IFA)
       != NCSCC_RC_SUCCESS)
   {
      m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
            "ifnd_mds_svc_evt_ifa : ifnd_mds_dest_add failed, ifa_dest = ",*ifa_dest);
          return NCSCC_RC_FAILURE;
   }

   return (NCSCC_RC_SUCCESS);

} /* function ifnd_mds_svc_evt_ifa() ends here */

/****************************************************************************
 * Name          : ifnd_mds_svc_evt_ifdrv
 *
 * Description   : This callback function which is called when IFDRVs comes up.
 *                 This function will prepare the MDS_DEST data base.
 *
 * Arguments     : ifd_dest  - This is the MDS_DEST of the IfA.
 *                 cb        - Ifnd Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_mds_svc_evt_ifdrv (MDS_DEST *drv_dest, IFSV_CB *cb)
{
   if (ifnd_mds_dest_add(drv_dest, cb, NCSMDS_SVC_ID_IFDRV)
       != NCSCC_RC_SUCCESS)
   {
      m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
            "ifnd_mds_svc_evt_ifdrv : ifnd_mds_dest_add failed, ifa_dest = ",*drv_dest);
          return NCSCC_RC_FAILURE;
   }

   return (NCSCC_RC_SUCCESS);

} /* function ifnd_mds_svc_evt_ifdrv() ends here */

/****************************************************************************
 * Name          : ifnd_mds_dest_add
 *
 * Description   : This function will add IFND_MDS_TYPE_INFO in its data base.
 *
 * Arguments     : ifd_dest  - This is the MDS_DEST of the IfA.
 *                 cb        - Ifnd Control Block.
 *                 type      - IfA or Drv.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
ifnd_mds_dest_add (MDS_DEST *mds_dest, IFSV_CB *cb, NCSMDS_SVC_ID type)
{
  NCS_PATRICIA_TREE  *p_tree = &cb->mds_dest_tbl;
  IFND_MDS_DEST_INFO_REC *rec = NULL;

  rec = m_MMGR_ALLOC_MDS_DEST_INFO_REC;
  if(rec == IFSV_NULL)
  {
    m_IFND_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
    return (NCSCC_RC_FAILURE);
  }

  m_NCS_MEMSET(rec, 0, sizeof(IFND_MDS_DEST_INFO_REC));

  rec->info.mds_dest = *mds_dest;
  rec->info.type = type;
  rec->pat_node.key_info = (uns8*)&rec->info.mds_dest;

  /* This is the idea of find and add */
  if (ncs_patricia_tree_get(p_tree, (uns8*)mds_dest) == NULL)
  {
     if (ncs_patricia_tree_add (p_tree, &rec->pat_node) != NCSCC_RC_SUCCESS)
     {
       m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
            "ifnd_mds_dest_add : ncs_patricia_tree_add failed, mds_dest = ",*mds_dest);

        m_MMGR_FREE_MDS_DEST_INFO_REC(rec);
        return (NCSCC_RC_FAILURE);
     }
  }

   return (NCSCC_RC_SUCCESS);

} /* function ifnd_mds_dest_add() ends here */

/****************************************************************************
 * Name          : ifnd_mds_dest_type_get 
 *
 * Description   : This gets mds destination of IfAs/Drv from patricia tree. 
 *
 * Arguments     : mds_dest - mds dest info.
 *                 cb   - This is the interace control block.
 *
 * Return Values : Pointer to the structure IFND_MDS_TYPE_INFO if found, else
 *                 NULL.
 *
 * Notes         : None.
 *****************************************************************************/
IFND_MDS_TYPE_INFO * 
ifnd_mds_dest_type_get (MDS_DEST mds_dest, IFSV_CB *cb)

{  
  NCS_PATRICIA_TREE  *p_tree = &cb->mds_dest_tbl;
  IFND_MDS_DEST_INFO_REC *rec = NULL;
  IFND_MDS_TYPE_INFO *info = NULL; 
  
  rec = (IFND_MDS_DEST_INFO_REC *)ncs_patricia_tree_get(p_tree, 
      (uns8 *)&mds_dest);   

  if (rec != NULL)
  {
    info = &rec->info;
  } 
  else
  {
    info = NULL;
  }

   return (info);

} /* function ifnd_mds_dest_type_get() ends here */

/****************************************************************************
 * Name          : ifnd_mds_dest_destroy_all 
 *
 * Description   : This is called to destroy all mds destination info 
 *                 from the patricia tree. 
 *
 * Arguments     : cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32 
ifnd_mds_dest_destroy_all (IFSV_CB *cb)
{  
  NCS_PATRICIA_NODE  *node = NULL;
  IFND_MDS_DEST_INFO_REC *rec = NULL;
  MDS_DEST mds_dest;

  m_NCS_MEMSET(&mds_dest, 0, sizeof(MDS_DEST));

  node = ncs_patricia_tree_getnext(&cb->mds_dest_tbl, (uns8*)&mds_dest);

  rec = (IFND_MDS_DEST_INFO_REC*)node;

  while (rec != NULL)
  {      
    m_NCS_MEMCPY(&mds_dest, &rec->info.mds_dest, sizeof(MDS_DEST));

    ifnd_mds_dest_del(rec->info.mds_dest, cb);            

    node = ncs_patricia_tree_getnext(&cb->mds_dest_tbl, (uns8*)&mds_dest);

    rec = (IFND_MDS_DEST_INFO_REC *)node;
  }

  return(NCSCC_RC_SUCCESS);

} /* function ifnd_mds_dest_destroy_all() ends here */

/****************************************************************************
 * Name          : ifnd_send_idim_ifndup_event
 *
 * Description   : This is called to send all the drivers that IfND is UP.
 *
 * Arguments     : cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_send_idim_ifndup_event (IFSV_CB *cb)
{
  NCS_PATRICIA_NODE  *node = NULL;
  IFND_MDS_DEST_INFO_REC *rec = NULL;
  MDS_DEST mds_dest;

  m_NCS_MEMSET(&mds_dest, 0, sizeof(MDS_DEST));

  node = ncs_patricia_tree_getnext(&cb->mds_dest_tbl, (uns8*)&mds_dest);

  rec = (IFND_MDS_DEST_INFO_REC*)node;

  while (rec != NULL)
  {
    m_NCS_MEMCPY(&mds_dest, &rec->info.mds_dest, sizeof(MDS_DEST));
    
    if(rec->info.type == NCSMDS_SVC_ID_IFDRV)
       idim_send_ifnd_up_evt(&mds_dest, cb);

    node = ncs_patricia_tree_getnext(&cb->mds_dest_tbl, (uns8*)&mds_dest);

    rec = (IFND_MDS_DEST_INFO_REC *)node;
  }

  return(NCSCC_RC_SUCCESS);
 
} /* function ifnd_send_idim_ifndup_event() ends here */

/****************************************************************************
 * Name          : ifnd_mds_dest_del
 *
 * Description   : This is called to destroy mds destination record from the 
 *                 patricia tree.
 *
 * Arguments     : cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_mds_dest_del (MDS_DEST mds_dest, IFSV_CB *cb)
{
  NCS_PATRICIA_TREE  *p_tree = &cb->mds_dest_tbl;
  NCS_PATRICIA_NODE  *node = NULL;
  
  node = ncs_patricia_tree_get (p_tree, (uns8 *)&mds_dest); 
  
  if (node == IFSV_NULL)
  {
     m_IFSV_LOG_HEAD_LINE(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
        IFSV_LOG_MDS_DEST_TBL_DEL_FAILURE, m_NCS_NODE_ID_FROM_MDS_DEST(mds_dest), 0);
     m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
            "ifnd_mds_dest_del : ncs_patricia_tree_get failed, mds_dest = ",mds_dest);
     return (NCSCC_RC_FAILURE);
  }   
   
   ncs_patricia_tree_del (p_tree, node);   
   
   /* stop the cleanp timer if it is running */
   m_MMGR_FREE_MDS_DEST_INFO_REC(node);

   return (NCSCC_RC_SUCCESS);

} /* function ifnd_mds_dest_del() ends here */

/****************************************************************************
 * Name          : ifnd_data_retrival_from_ifd
 *
 * Description   : This function will be called when the state of IfND is in
 *                 NCS_IFSV_IFND_DATA_RETRIVAL_STATE and ifND gets 
 *                 IFND_EVT_INTF_CREATE event. It updates the record in its data
 *                 base, in two cases: 
 *                 1. it belogs to application, which is at the same 
 *                 node and alive.
 *                 2. it belongs to some other node than this IfND belongs.
 *                 If all the interface record is retrieved, then it changes
 *                 state from NCS_IFSV_IFND_DATA_RETRIVAL_STATE to 
 *                 NCS_IFSV_IFND_OPERATIONAL_STATE.
 *
 * Arguments     : evt  - This is the pointer which holds the event structure.
 *                 cb   - This is the interace control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None. 
 *****************************************************************************/
uns32 
ifnd_data_retrival_from_ifd (IFSV_CB *ifsv_cb, 
                             IFSV_INTF_CREATE_INFO *create_intf, 
                             IFSV_SEND_INFO *sinfo)
{
  uns32 res = NCSCC_RC_SUCCESS;
  uns32 last_msg = FALSE;
  IFSV_EVT send_evt;
  IFSV_INTF_ATTR attr = NCS_IFSV_IAM_OPRSTATE;
  IFSV_INTF_DATA *rec_data = NULL;
  IFSV_EVT *msg_rcvd = NULL;

  /* Don't process the events from IfA and Drv in this state.*/ 
  if(create_intf->evt_orig == NCS_IFSV_EVT_ORIGN_HW_DRV)
  {
     return NCSCC_RC_FAILURE;
  }
  else if(create_intf->evt_orig == NCS_IFSV_EVT_ORIGN_IFA)
  {
     /* Now send sync resp to the IfA. */
     m_NCS_OS_MEMSET(&send_evt, 0, sizeof(IFSV_EVT));
     send_evt.type = IFA_EVT_INTF_CREATE_RSP;
     send_evt.error = NCS_IFSV_IFND_RESTARTING_ERROR;
     send_evt.info.ifa_evt.info.if_add_rsp_idx =
                                create_intf->intf_data.if_index;
     /* Sync resp */
     res = ifsv_mds_send_rsp(ifsv_cb->my_mds_hdl, NCSMDS_SVC_ID_IFND,
                               sinfo, &send_evt);
     return res;
  }

  if(create_intf->intf_data.no_data == TRUE)
  {
    if(ifsv_cb->ifnd_state == NCS_IFSV_IFND_DATA_RETRIVAL_STATE)
    {
        ifsv_cb->ifnd_state = NCS_IFSV_IFND_OPERATIONAL_STATE;
        /* Send message to all the drivers that IfND is operational. */
        ifnd_send_idim_ifndup_event(ifsv_cb);
        ifnd_mds_dest_destroy_all(ifsv_cb);
    }
    else
      res = NCSCC_RC_FAILURE;
    return res;

  }
    
  if(create_intf->intf_data.last_msg == TRUE)
    last_msg = TRUE;  

  if(m_NCS_NODE_ID_FROM_MDS_DEST(create_intf->intf_data.originator_mds_destination) == m_NCS_NODE_ID_FROM_MDS_DEST(ifsv_cb->my_dest))
  {
    if(ifnd_mds_dest_type_get(create_intf->intf_data.originator_mds_destination,
                           ifsv_cb) != NULL)
    {
      /* This means that the record owner is alive */
         res = ifsv_intf_rec_add(&create_intf->intf_data, ifsv_cb);
         if(res != NCSCC_RC_SUCCESS)
         {
           m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
            "ifnd_data_retrival_from_ifd : Same Node : ifsv_intf_rec_add failed, Index = ",create_intf->intf_data.if_index);
           goto end;
         }
      /* send a message to all the application which has registered
       * with the IFA */
         ifsv_ifa_app_if_info_indicate(&create_intf->intf_data,
                   IFSV_INTF_REC_ADD, create_intf->if_attr, ifsv_cb);
         if(res != NCSCC_RC_SUCCESS)
         {
           goto end;
         }
    }
    else
    {
      /* The interface might have been deleted while IfND was DOWN. So, store 
         it in the data base with oper status DOWN and
         mark it delete and sends this info to all API and IfD. */
         create_intf->intf_data.if_info.oper_state = NCS_STATUS_DOWN;

         res = ifsv_intf_rec_add(&create_intf->intf_data, ifsv_cb);

         /* Send an MDS message for delete */
         ifnd_sync_send_to_ifd(&create_intf->intf_data, IFSV_INTF_REC_DEL, 0,
                               ifsv_cb, &msg_rcvd);


         if(res != NCSCC_RC_SUCCESS)
         {
           m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
            "ifnd_data_retrival_from_ifd :  Same Node :No Rec : ifsv_intf_rec_add failed, Index = ",create_intf->intf_data.if_index);
           goto end;
         }

         if((rec_data = ifsv_intf_rec_find (create_intf->intf_data.if_index, 
                        ifsv_cb)) != IFSV_NULL)
             res = ifsv_intf_rec_marked_del(rec_data, &attr, ifsv_cb);

         if(res != NCSCC_RC_SUCCESS)
         {
           m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
            "ifnd_data_retrival_from_ifd :  Same Node :No Rec : ifsv_intf_rec_marked_del failed, Index = ",create_intf->intf_data.if_index);
           goto end;
         }
    }
  }
  else
  {
    /* This means that the record belongs to other node. So, just add it in data
      base and tell the applications. */
    res = ifsv_intf_rec_add(&create_intf->intf_data, ifsv_cb);
    if(res != NCSCC_RC_SUCCESS)
    {
      m_IFND_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
       "ifnd_data_retrival_from_ifd : Diff Node : No Rec : ifsv_intf_rec_add failed, Index = ",create_intf->intf_data.if_index);
      goto end;
    }
    /* send a message to all the application which has registered
     * with the IFA */
    res = ifsv_ifa_app_if_info_indicate(&create_intf->intf_data,
                   IFSV_INTF_REC_ADD, create_intf->if_attr, ifsv_cb);
    if(res != NCSCC_RC_SUCCESS)
    {
      goto end;
    }
  }

end: 

  if(last_msg == TRUE)
  {
   if(ifsv_cb->ifnd_state == NCS_IFSV_IFND_DATA_RETRIVAL_STATE)
   {
      ifsv_cb->ifnd_state = NCS_IFSV_IFND_OPERATIONAL_STATE;
      /* Send message to all the drivers that IfND is operational. */
      ifnd_send_idim_ifndup_event(ifsv_cb); 
      ifnd_mds_dest_destroy_all(ifsv_cb);
   }
   else
     res = NCSCC_RC_FAILURE;
  }

  return res;
} /* function ifnd_data_retrival_from_ifd () ends here */

/****************************************************************************
 * Name          : ifnd_ifa_same_dst_intf_rec_and_mds_dest_del
 *
 * Description   : This event callback function which would be called when
 *                 ifa will go down, when IfND is in 
 *                 NCS_IFSV_IFND_DATA_RETRIVAL_STATE. This would delete
 *                 the interface records if found in the data base and delete
 *                 MDS_DEST from MDS data base.
 *
 * Arguments     : mds_dest  - mds_dest.
 *                 cb   - control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_ifa_same_dst_intf_rec_and_mds_dest_del (MDS_DEST *mds_dest, IFSV_CB *cb)
{
  uns32 res = NCSCC_RC_SUCCESS;

  /* Delete from the data base. */ 
  ifnd_same_dst_intf_rec_del(mds_dest, cb);

  /* Delete MDS_DEST, so that when other intf rec comes from IfD, then IfND
     accept those, but runs aging timer. */
   res = ifnd_mds_dest_del(*mds_dest, cb);

  return res;
} /* function ifnd_ifa_same_dst_intf_rec_and_mds_dest_del () ends here */

/****************************************************************************
 * Name          : ifnd_drv_same_dst_intf_rec_and_mds_dest_del
 *
 * Description   : This event callback function which would be called when
 *                 ifa will go down, when IfND is in
 *                 NCS_IFSV_IFND_DATA_RETRIVAL_STATE. This would delete
 *                 the interface records if found in the data base and delete
 *                 MDS_DEST from MDS data base.
 *
 * Arguments     : mds_dest  - mds_dest.
 *                 cb   - control block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifnd_drv_same_dst_intf_rec_and_mds_dest_del (MDS_DEST *mds_dest, IFSV_CB *cb)
{
  uns32 res = NCSCC_RC_SUCCESS;

  /* Delete from the data base. */
  ifnd_same_dst_intf_rec_del(mds_dest, cb);

  /* Delete MDS_DEST, so that when other intf rec comes from IfD, then IfND
     accept those, but runs aging timer. */
   res = ifnd_mds_dest_del(*mds_dest, cb);

  return res;
} /* function ifnd_drv_same_dst_intf_rec_and_mds_dest_del () ends here */

/*********************  file ends here ************************************/
