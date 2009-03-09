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

#include "ifd.h"


static uns32 ipxs_ifd_lib_create (IPXS_LIB_CREATE *create);

static uns32 ipxs_ifd_lib_destroy(void );

static uns32 ipxs_ifd_ipaddr_info_bcast(IPXS_CB *cb, NCS_IPPFX *ip_pfx, 
                                 uns8  pfx_cnt, NCS_BOOL add_flag, 
                                 NCS_BOOL del_flag, uns32 if_index);
uns32 gl_ipxs_cb_hdl;

static uns32 ipxs_reg_with_mab(IPXS_CB *cb);

static uns32 ipxs_unreg_with_mab(IPXS_CB *cb);


static uns32 ipxs_ifd_ipinfo_bcast(IPXS_CB *cb, 
                            NCS_IPXS_IPINFO *ip_info, 
                            NCS_IFSV_IFINDEX if_index, NCS_BOOL netlink_msg);

static uns32 ifd_ipxs_proc_ifip_info(IPXS_CB *cb, IPXS_EVT *ipxs_evt, IFSV_SEND_INFO *sinfo);

static uns32 ipxs_ifd_proc_ipaddr(IPXS_CB *cb, IPXS_IFIP_INFO *ifip_info, 
                           NCS_IPXS_IPINFO *ip_info);


/****************************************************************************
 * Name          : ipxs_ifd_lib_req
 *
 * Description   : This is the NCS SE API which is used to Create/destroy library
 *                 of the IPXS.
 *
 * Arguments     : req_info  - This is the pointer to the input information 
 *                             which SBOM gives.  
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ipxs_ifd_lib_req (IPXS_LIB_REQ_INFO *req_info)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   switch (req_info->op)
   {      
      case NCS_LIB_REQ_CREATE:
         rc = ipxs_ifd_lib_create(&req_info->info.create);
         break;
      case NCS_LIB_REQ_DESTROY:
         rc = ipxs_ifd_lib_destroy( );
         break;
      default:
         break;
   }
   return (rc);
}

/****************************************************************************
 * Name          : ipxs_ifd_lib_create
 *
 * Description   : This is the function which initalize the IPXS databases
 *                 and library.
 *                 
 * Arguments     : pointer to NCS_LIB_CREATE
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
ipxs_ifd_lib_create (IPXS_LIB_CREATE *create)
{

   IPXS_CB                 *cb = NULL;
   uns32                   rc, i, index=0;
   NCS_PATRICIA_PARAMS     params;

   /* Malloc the Control Block for IPXS */
   cb = m_MMGR_ALLOC_IPXS_CB;

   if(cb == NULL)
   {
      m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,0);
      return NCSCC_RC_FAILURE;
   }
   m_NCS_MEMSET(cb, 0, sizeof(IPXS_CB));

   cb->hm_pid     = create->pool_id;
   cb->my_svc_id  = create->svc_id;
   cb->my_mds_hdl = create->mds_hdl;
   cb->oac_hdl    = create->oac_hdl;

   /* Initialize the IF-IP Table in IPXS CB */
   params.key_size = sizeof(uns32);
   params.info_size = 0;
   if ((ncs_patricia_tree_init(&cb->ifip_tbl, &params)) 
                                                   != NCSCC_RC_SUCCESS)
   {
      rc = NCSCC_RC_FAILURE;
      goto ipxs_db_init_fail;
   }
   cb->ifip_tbl_up = TRUE;

   /* Init the IP Table */
   for(i=NCS_IP_ADDR_TYPE_IPV4; i < NCS_IP_ADDR_TYPE_MAX; i++)
   {
      m_IPXS_DB_INDEX_FROM_ADDRTYPE_GET(index, i);

      if(index == IPXS_AFI_MAX)   /* This address type is not supported */
         continue;

      cb->ip_tbl[index].type = i;
      if((rc = ipxs_ipdb_init(&cb->ip_tbl[index])) != NCSCC_RC_SUCCESS)
      {
         goto ipxs_db_init_fail;
      }
   }

   /* Handle Manager Init Routines */
   if((cb->cb_hdl = ncshm_create_hdl(cb->hm_pid, 
               cb->my_svc_id, (NCSCONTEXT)cb)) == 0)
   {
      rc = NCSCC_RC_FAILURE;
      m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_HDL_MGR_FAIL,IFSV_COMP_IFND);
      goto ipxs_hdl_fail;            
   }

   if((rc = ipxs_reg_with_mab(cb)) != NCSCC_RC_SUCCESS)
   {
      goto ipxs_oaa_init_fail;
   }

   /* Register all the tables with MIBLIB */
   ncsifsvifipentry_tbl_reg( );

   ncsifsvipentry_tbl_reg( );

   m_IPXS_STORE_HDL(cb->cb_hdl);

   return NCSCC_RC_SUCCESS;

ipxs_oaa_init_fail:

   /* Destroy the Handle */
   ncshm_destroy_hdl(create->svc_id, cb->cb_hdl);

ipxs_hdl_fail:

ipxs_db_init_fail:
   /* Free the database */
   if(cb->ifip_tbl_up)
      ncs_patricia_tree_destroy(&cb->ifip_tbl);

   for(i=NCS_IP_ADDR_TYPE_IPV4; i < NCS_IP_ADDR_TYPE_MAX; i++)
   {
      m_IPXS_DB_INDEX_FROM_ADDRTYPE_GET(index, i);

      if(index == IPXS_AFI_MAX)   /* This address type is not supported */
         continue;

      if(cb->ip_tbl[index].up)
         ipxs_ipdb_destroy(&cb->ip_tbl[index]);
   }

   /* Free the Control Block */
   m_MMGR_FREE_IPXS_CB(cb);

   return rc;
}

/****************************************************************************
 * Name          : ipxs_ifd_lib_destroy
 *
 * Description   : This is the function which destroy the IPXS libarary.
 *                
 *
 * Arguments     : pointer to NCS_LIB_DESTROY
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 ipxs_ifd_lib_destroy ( )
{ 
   uns32    ipxs_hdl;
   IPXS_CB   *cb = NULL;
   uns32     svc_id, i, index=IPXS_AFI_MAX;

   /* Get the Handle Manager Handle */
   ipxs_hdl = m_IPXS_CB_HDL_GET( );

   cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, ipxs_hdl);

   if (cb == NULL)
   {
      m_IFD_LOG_API_L(IFSV_LOG_SE_DESTROY_FAILURE,IFSV_COMP_IFND);
      return (NCSCC_RC_FAILURE);
   }

   svc_id = cb->my_svc_id;
   

   if(cb->my_svc_id == NCS_SERVICE_ID_IFD)
   {
      /* Un-register with the MAB */
      ipxs_unreg_with_mab(cb);
   }

   /* Free the databases */
   if(cb->ifip_tbl_up)
      ipxs_ifip_db_destroy(&cb->ifip_tbl, &cb->ifip_tbl_up);

   for(i=NCS_IP_ADDR_TYPE_IPV4; i < NCS_IP_ADDR_TYPE_MAX; i++)
   {
      m_IPXS_DB_INDEX_FROM_ADDRTYPE_GET(index, i);

      if(index == IPXS_AFI_MAX)   /* This address type is not supported */
         continue;

      if(cb->ip_tbl[index].up)
         ipxs_ipdb_destroy(&cb->ip_tbl[index]);
   }

   /* TBD:RSR Free the subscription linked list */

   /* Give the Handle */
   ncshm_give_hdl(ipxs_hdl);

   /* Destroy the Handle */
   ncshm_destroy_hdl(svc_id, ipxs_hdl);

   /* Free the Control Block */
   m_MMGR_FREE_IPXS_CB(cb);

   return (NCSCC_RC_SUCCESS);
}

/******************************************************************************
  Function :  ipxs_mib_tbl_req
  
  Purpose  :  High Level MIB Access Routine for Interface MIB table
 
  Input    :  struct ncsmib_arg*
    
  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ipxs_mib_tbl_req (struct ncsmib_arg *args)
{
   IPXS_CB            *cb = NULL;
   uns32               status = NCSCC_RC_SUCCESS, svc_id;
   NCSMIBLIB_REQ_INFO  miblib_req;
   uns32              hdl = m_IPXS_CB_HDL_GET( );

   /* Get the CB pointer from the CB handle */
   svc_id = NCS_SERVICE_ID_IFD;
   cb = (IPXS_CB *) ncshm_take_hdl(svc_id, hdl);

   if(cb == NULL)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_IFD_IPXS_EVT_INFO,\
                     "ncshm_take_hdl returned NULL",0);

      return NCSCC_RC_FAILURE;
   }
   m_NCS_OS_MEMSET(&miblib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

   miblib_req.req = NCSMIBLIB_REQ_MIB_OP;
   miblib_req.info.i_mib_op_info.args = args;
   miblib_req.info.i_mib_op_info.cb = cb;
   
   /* call the mib routine handler */ 
   status = ncsmiblib_process_req(&miblib_req);

   ncshm_give_hdl(hdl);
   
   return status; 
}


/****************************************************************************
  PROCEDURE NAME:   ipxs_reg_with_mab

  DESCRIPTION:      The function registers all the SNMP MIB tables supported
                    by IPXS with MAB
  ARGUMENTS:     
        
  RETURNS:
        NCSCC_RC_SUCCESS
        NCSCC_RC_FAILURE

  NOTES:
*****************************************************************************/
static uns32 ipxs_reg_with_mab(IPXS_CB *cb)
{

   NCSOAC_SS_ARG      mab_arg;
   NCSMIB_TBL_ID      tbl_id;   
   
   m_NCS_OS_MEMSET(&mab_arg, 0, sizeof(NCSOAC_SS_ARG));
   mab_arg.i_oac_hdl = cb->oac_hdl;

   for(tbl_id = NCSMIB_TBL_IPXS_BASE; 
       tbl_id <= NCSMIB_TBL_IPXS_IFIPTBL;
       tbl_id ++)
   {
      mab_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
      mab_arg.i_tbl_id = tbl_id;
      mab_arg.info.tbl_owned.i_mib_req = ipxs_mib_tbl_req;
      mab_arg.info.tbl_owned.i_ss_id = cb->my_svc_id;
      mab_arg.info.tbl_owned.i_mib_key = cb->cb_hdl;
      
      if(NCSCC_RC_SUCCESS != ncsoac_ss(&mab_arg))
      {
        m_IFD_LOG_EVT_L(IFSV_LOG_IFD_IPXS_EVT_INFO,\
                     "Reg with mab failed, SVC id : ",cb->my_svc_id);
        return NCSCC_RC_FAILURE;       
      }
      /* regiser the Filter for each table */
      mab_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;          
      mab_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_ANY;
      mab_arg.info.row_owned.i_fltr.fltr.any.meaningless = 0x123;
      
      if(NCSCC_RC_SUCCESS != ncsoac_ss(&mab_arg))
      {
        m_IFD_LOG_EVT_L(IFSV_LOG_IFD_IPXS_EVT_INFO,\
                     "Reg with mab failed, SVC id : ",cb->my_svc_id);
         return NCSCC_RC_FAILURE;       
      }
   }/* End of for loop to register the tables */

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  PROCEDURE NAME:   ipxs_unreg_with_mab

  DESCRIPTION:      The function de registers all the SNMP MIB tables supported
                    by IPXS with MAB
  ARGUMENTS:     
        
  RETURNS:
        NCSCC_RC_SUCCESS
        NCSCC_RC_FAILURE

  NOTES:
*****************************************************************************/
static uns32 ipxs_unreg_with_mab(IPXS_CB *cb)
{
   NCSOAC_SS_ARG      mab_arg;
   NCSMIB_TBL_ID      tbl_id;   
   
   m_NCS_OS_MEMSET(&mab_arg, 0, sizeof(NCSOAC_SS_ARG));   
   mab_arg.i_oac_hdl = cb->cb_hdl;   
   mab_arg.i_op = NCSOAC_SS_OP_TBL_GONE;     
   
   for(tbl_id = NCSMIB_TBL_IPXS_BASE; 
       tbl_id <= NCSMIB_TBL_IPXS_IFIPTBL;
       tbl_id ++)
   {
      mab_arg.i_tbl_id = tbl_id;      
      if(NCSCC_RC_SUCCESS != ncsoac_ss(&mab_arg))
      {
        m_IFD_LOG_EVT_L(IFSV_LOG_IFD_IPXS_EVT_INFO,\
                     "Unreg with mab failed, SVC id : ",cb->my_svc_id);
        return NCSCC_RC_FAILURE;       
      }
   }
   return NCSCC_RC_SUCCESS;
}

uns32 ipxs_ifd_ipinfo_process (IPXS_CB *cb, IPXS_IP_NODE *ip_node)
{
   IPXS_IFIP_NODE    *ifip_node=0;
   uns32             index=IPXS_AFI_MAX, rc= NCSCC_RC_SUCCESS, if_index =0;
   IPXS_IP_INFO      *ip_info = &ip_node->ipinfo;
   IPXS_IFIP_IP_INFO ippfx;
   NCS_BOOL          add_flag = FALSE;
   NCS_BOOL          del_flag = FALSE;

   m_IPXS_DB_INDEX_FROM_ADDRTYPE_GET(index, ip_info->addr.type);

   if(index == IPXS_AFI_MAX)   /* This address type is not supported */
      return NCSCC_RC_NO_INSTANCE;

   /* Find the interface record */
   if(cb->ifip_tbl_up)
      ifip_node = ipxs_ifip_record_get(&cb->ifip_tbl, ip_info->if_index);

   if(!ifip_node)
   {
      /* IFIP Record is not there, delete the IPRecord & Retuen No instance */
      ipxs_ip_record_del(cb, &cb->ip_tbl[index], ip_node);

      return NCSCC_RC_NO_INSTANCE;

   }
   
   if(ifip_node->ifip_info.is_v4_unnmbrd == NCS_SNMP_TRUE) 
   {
     /* We cann't process this request. */
    ipxs_ip_record_del(cb, &cb->ip_tbl[index], ip_node);
    return NCSCC_RC_FAILURE;
   }

   if_index = ip_info->if_index;
   m_NCS_OS_MEMSET(&ippfx, 0, sizeof(IPXS_IFIP_IP_INFO));
   ippfx.ipaddr.ipaddr   = ip_node->ipinfo.addr;
   ippfx.ipaddr.mask_len = ip_node->ipinfo.mask;

   if(ip_node->ipinfo.status == NCS_ROW_ACTIVE)
   {
      /* Add IP address to ifip info */
      rc = ipxs_ifip_ippfx_add(cb, &ifip_node->ifip_info, &ippfx, 1, &add_flag);
   }
   else if(ip_node->ipinfo.status == NCS_ROW_DESTROY)
   {
      /* Delete IP address to ifip info */
      rc = ipxs_ifip_ippfx_del(cb, &ifip_node->ifip_info, &ippfx, 1, &del_flag);

      /* Also delete the IP Record from IP database */
      ipxs_ip_record_del(cb, &cb->ip_tbl[index], ip_node);
   }
   else
      return rc;

   rc = ipxs_ifd_ipaddr_info_bcast(cb, &ippfx.ipaddr, 1, add_flag, 
                                  del_flag, if_index);

   return rc;
}


static uns32 ipxs_ifd_ipaddr_info_bcast(IPXS_CB *cb, NCS_IPPFX *ip_pfx, 
                                 uns8  pfx_cnt, NCS_BOOL add_flag, 
                                 NCS_BOOL del_flag, uns32 if_index)
{
   IPXS_EVT    ipxs_evt;
   IFSV_EVT    evt;
   uns32       rc = NCSCC_RC_SUCCESS;
   uns32       rc1 = NCSCC_RC_SUCCESS;
   uns32 ifsv_hdl;
   IFSV_CB *ifsv_cb;

   ifsv_hdl = m_IFD_CB_HDL_GET();

   ifsv_cb = (IFSV_CB *) ncshm_take_hdl(NCS_SERVICE_ID_IFD, ifsv_hdl);

   if(ifsv_cb == NULL)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_IFD_IPXS_EVT_INFO,\
                     "ncshm_take_hdl returned NULL",cb->my_svc_id);
      return (NCSCC_RC_FAILURE);
   }


   if((add_flag == FALSE) && (del_flag == FALSE))
   {
      return NCSCC_RC_SUCCESS;
   }

   m_NCS_OS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
   m_NCS_OS_MEMSET(&ipxs_evt, 0, sizeof(IPXS_EVT));

   evt.info.ipxs_evt = (NCSCONTEXT)&ipxs_evt;

   evt.type = IFSV_IPXS_EVT;

   ipxs_evt.type = IPXS_EVT_DTOND_IFIP_INFO;
   ipxs_evt.info.nd.dtond_upd.if_index = if_index;
   ipxs_evt.info.nd.dtond_upd.ip_info.ip_attr = NCS_IPXS_IPAM_ADDR;
   if(add_flag)
   {
      ipxs_evt.info.nd.dtond_upd.ip_info.addip_cnt = pfx_cnt;
      /* Mem alloc for addlist */
      ipxs_evt.info.nd.dtond_upd.ip_info.addip_list = 
         (IPXS_IFIP_IP_INFO *) m_MMGR_ALLOC_IPXS_DEFAULT(pfx_cnt*sizeof(IPXS_IFIP_IP_INFO));

      if(!ipxs_evt.info.nd.dtond_upd.ip_info.addip_list)
      {
         m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,0);
         return NCSCC_RC_OUT_OF_MEM;
      }
      m_NCS_OS_MEMCPY(ipxs_evt.info.nd.dtond_upd.ip_info.addip_list, ip_pfx,
                   (pfx_cnt * sizeof(IPXS_IFIP_IP_INFO)));
   }
   else if(del_flag)
   {
      ipxs_evt.info.nd.dtond_upd.ip_info.delip_cnt = pfx_cnt;
      
      ipxs_evt.info.nd.dtond_upd.ip_info.delip_list = 
          (NCS_IPPFX *) m_MMGR_ALLOC_IPXS_DEFAULT(pfx_cnt*sizeof(NCS_IPPFX));
      if(!ipxs_evt.info.nd.dtond_upd.ip_info.delip_list)
      {
         m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,0);
         return NCSCC_RC_OUT_OF_MEM;
      }
      m_NCS_OS_MEMCPY(ipxs_evt.info.nd.dtond_upd.ip_info.delip_list, ip_pfx,
                   (pfx_cnt * sizeof(NCS_IPPFX)));
   }

   /* Send the trigger point to Standby IfD. */
   rc1 = ifd_a2s_async_update(ifsv_cb, IFD_A2S_IPXS_MSG,
                              (uns8 *)(&ipxs_evt.info.d.ndtod_upd));
   if(rc1 == NCSCC_RC_SUCCESS)
     /* Log the error. */

   rc = ifsv_mds_bcast_send(cb->my_mds_hdl, 
                 NCSMDS_SVC_ID_IFD, &evt, NCSMDS_SVC_ID_IFND);

   return rc;
}


static uns32 ipxs_ifd_ipinfo_bcast(IPXS_CB *cb, 
                            NCS_IPXS_IPINFO *ip_info, 
                            NCS_IFSV_IFINDEX if_index, NCS_BOOL netlink_msg)
{
   IPXS_EVT    ipxs_evt;
   IFSV_EVT    evt;
   uns32       rc;

   m_NCS_OS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
   m_NCS_OS_MEMSET(&ipxs_evt, 0, sizeof(IPXS_EVT));

   evt.info.ipxs_evt = (NCSCONTEXT)&ipxs_evt;

   evt.type = IFSV_IPXS_EVT;

   ipxs_evt.type = IPXS_EVT_DTOND_IFIP_INFO;
   ipxs_evt.netlink_msg = netlink_msg;
   ipxs_evt.info.nd.dtond_upd.if_index = if_index;

   /* Copy the IP info & its internal pointers */
   m_NCS_OS_MEMCPY(&ipxs_evt.info.nd.dtond_upd.ip_info, 
                                  ip_info, sizeof(NCS_IPXS_IPINFO));

   /* Nullify the internal pointers */
   ip_info->addip_list = 0;
   ip_info->delip_list = 0;

   rc = ifsv_mds_bcast_send(cb->my_mds_hdl, 
                 NCSMDS_SVC_ID_IFD, &evt, NCSMDS_SVC_ID_IFND);

   return rc;
}


uns32 ipxs_ifd_ifip_info_bcast(IPXS_CB *cb, 
                        IPXS_IFIP_INFO *ifip_info, uns32 am)
{
   IPXS_EVT    ipxs_evt;
   IFSV_EVT    evt;
   uns32       rc = NCSCC_RC_SUCCESS;
   uns32       rc1 = NCSCC_RC_SUCCESS;
   uns32 ifsv_hdl;
   IFSV_CB *ifsv_cb;

   ifsv_hdl = m_IFD_CB_HDL_GET();

   ifsv_cb = (IFSV_CB *) ncshm_take_hdl(NCS_SERVICE_ID_IFD, ifsv_hdl);

   if(ifsv_cb == NULL)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_IFD_IPXS_EVT_INFO,\
                     "ncshm_take_hdl returned NULL",cb->my_svc_id);
      return (NCSCC_RC_FAILURE);
   }

   m_NCS_OS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
   m_NCS_OS_MEMSET(&ipxs_evt, 0, sizeof(IPXS_EVT));

   evt.info.ipxs_evt = (NCSCONTEXT)&ipxs_evt;

   evt.type = IFSV_IPXS_EVT;

   ipxs_evt.type = IPXS_EVT_DTOND_IFIP_INFO;
   ipxs_evt.info.nd.dtond_upd.if_index = ifip_info->ifindexNet;
   ipxs_evt.info.nd.dtond_upd.ip_info.ip_attr = am;
   if(am & NCS_IPXS_IPAM_ADDR)
   {
      if(ifip_info->ipaddr_cnt)
      {
         ipxs_evt.info.nd.dtond_upd.ip_info.addip_cnt = ifip_info->ipaddr_cnt;
         m_NCS_OS_MEMCPY(ipxs_evt.info.nd.dtond_upd.ip_info.addip_list,
                   ifip_info->ipaddr_list,
                   (ifip_info->ipaddr_cnt * sizeof(IPXS_IFIP_IP_INFO)));
      }
   }

   if(am & NCS_IPXS_IPAM_UNNMBD)
      ipxs_evt.info.nd.dtond_upd.ip_info.is_v4_unnmbrd = ifip_info->is_v4_unnmbrd;

   /* Send the trigger point to Standby IfD. */
   rc1 = ifd_a2s_async_update(ifsv_cb, IFD_A2S_IPXS_MSG,
                              (uns8 *)(&ipxs_evt.info.d.ndtod_upd));
   if(rc1 == NCSCC_RC_SUCCESS)
   {
     /* Log the error. */
   }
   
   rc = ifsv_mds_bcast_send(cb->my_mds_hdl, 
                 NCSMDS_SVC_ID_IFD, &evt, NCSMDS_SVC_ID_IFND);

   return rc;
}

static uns32 ifd_ipxs_proc_ifip_info(IPXS_CB *cb, IPXS_EVT *ipxs_evt, IFSV_SEND_INFO *sinfo)
{
   NCS_IFSV_IFINDEX  if_index;
   NCS_IPXS_IPINFO   *ipinfo;
   uns32    rc=NCSCC_RC_SUCCESS;
   uns32    rc1=NCSCC_RC_SUCCESS;
   IPXS_EVT    o_ipxs_evt;
   IFSV_EVT    evt;
   uns32 ifsv_hdl;
   IFSV_CB *ifsv_cb;

   ifsv_hdl = m_IFD_CB_HDL_GET();

   ifsv_cb = (IFSV_CB *) ncshm_take_hdl(NCS_SERVICE_ID_IFD, ifsv_hdl);

   if(ifsv_cb == NULL)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_IFD_IPXS_EVT_INFO,\
                     "ncshm_take_hdl returned NULL",cb->my_svc_id);
      return (NCSCC_RC_FAILURE);
   }

   m_NCS_OS_MEMSET(&evt, 0, sizeof(IFSV_EVT));
   m_NCS_OS_MEMSET(&o_ipxs_evt, 0, sizeof(IPXS_EVT));

   ipinfo = &ipxs_evt->info.d.ndtod_upd.ip_info;
   
   /* Send the trigger point to Standby IfD. */
   rc1 = ifd_a2s_async_update(ifsv_cb, IFD_A2S_IPXS_MSG, 
                              (uns8 *)(&ipxs_evt->info.d.ndtod_upd));

   rc = ifd_ipxs_proc_data_ifip_info(cb, ipxs_evt, &if_index);

   if(rc1 == NCSCC_RC_SUCCESS)
   {
     /* Log the error. */ 
   }

   evt.info.ipxs_evt = (NCSCONTEXT)&o_ipxs_evt;

   evt.type = IFSV_IPXS_EVT;
   o_ipxs_evt.type = IPXS_EVT_DTOND_IFIP_INFO;
   if(rc == NCSCC_RC_SUCCESS)
      o_ipxs_evt.error = NCS_IFSV_NO_ERROR;
   else
      o_ipxs_evt.error = NCS_IFSV_INT_ERROR;

   o_ipxs_evt.info.nd.dtond_upd.if_index = if_index;
   
   /* Copy the IP info & its internal pointers */
   m_NCS_OS_MEMCPY(&o_ipxs_evt.info.nd.dtond_upd.ip_info, 
                    ipinfo, sizeof(NCS_IPXS_IPINFO));

   /* Sync resp to IfND.*/
   rc = ifsv_mds_send_rsp(cb->my_mds_hdl, NCSMDS_SVC_ID_IFD,
                          sinfo, &evt);

   if(rc == NCSCC_RC_SUCCESS)
      rc = ipxs_ifd_ipinfo_bcast(cb, ipinfo, if_index, ipxs_evt->netlink_msg);

   return rc;
}

/****************************************************************************
 * Name          : ifd_ipxs_proc_data_ifip_info
 *
 * Description   : Function to update the data base. This function segregates
 *                 the data base update from sending the info to IfND.
 *
 * Arguments     : cb - pointer to the control block.
 *                 ipxs_evt - Pointer to the message
 *                 ipinfo - Poniter to the IP structure.
 *                 if_index - Pointer to the if index.
 *                 
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * Notes         : None.
 *****************************************************************************/
uns32
ifd_ipxs_proc_data_ifip_info(IPXS_CB *cb, IPXS_EVT *ipxs_evt, 
                             NCS_IFSV_IFINDEX *if_index)
{
   IPXS_IFIP_INFO    *ifip_info = NULL;
   uns32    rc=NCSCC_RC_SUCCESS;
   NCS_IPXS_IPINFO *ipinfo = NULL;
#if (NCS_VIP == 1)
   IPXS_IFIP_IP_INFO *pIfipIpInfo;
   uns32    ii;
#endif
   *if_index = ipxs_evt->info.d.ndtod_upd.if_index;
   ipinfo = &ipxs_evt->info.d.ndtod_upd.ip_info;

      /* Get the record for the given ifKey */
   rc = ipxs_ifiprec_get(cb, (*if_index), &ifip_info);

   if(ifip_info == NULL)
   {
      m_IFD_LOG_EVT_L(IFSV_LOG_IFD_IPXS_EVT_INFO,\
                     "No IFIP info exists for index : ",\
                      *if_index);
      goto free_mem;
   }

   if(rc == NCSCC_RC_FAILURE)
   {
      goto free_mem;
   }

   if(m_NCS_IPXS_IS_IPAM_UNNMBD_SET(ipinfo->ip_attr))
   {
      m_NCS_IPXS_IPAM_UNNMBD_SET(ifip_info->ipam);
      if(ifip_info->is_v4_unnmbrd == NCS_SNMP_TRUE)
      {
            /* This means that there is no IP address on this intf. */
            ifip_info->is_v4_unnmbrd = ipinfo->is_v4_unnmbrd;
      }
      else if(ifip_info->is_v4_unnmbrd == NCS_SNMP_FALSE)
      {
           /* This means that there may or may not be IP address assinged.
              So, if assigned then delete all the interfaces.*/
           if(ipinfo->is_v4_unnmbrd == NCS_SNMP_TRUE)
              ipxs_ip_record_list_del(cb, ifip_info);
           ifip_info->is_v4_unnmbrd = ipinfo->is_v4_unnmbrd;
      }
   }
#if (NCS_VIP == 1)
   if((m_NCS_IPXS_IS_IPAM_VIP_REFCNT_SET(ipinfo->ip_attr)) &&
           (m_NCS_IPXS_IS_IPAM_VIP_SET(ipinfo->ip_attr))) 
   {
      for ( ii = 0; ii < ifip_info->ipaddr_cnt ; ii ++)
      {
         pIfipIpInfo = &ifip_info->ipaddr_list[ii];
         if (( m_NCS_MEMCMP(&pIfipIpInfo->ipaddr,&ipinfo->addip_list->ipaddr,
                                     sizeof(NCS_IPPFX)))== 0)
         {
            if(ipinfo->addip_list->refCnt == 0)
            {
               pIfipIpInfo->refCnt--;
            }
            else if(ipinfo->addip_list->refCnt == 1)
            {
               pIfipIpInfo->refCnt++;
            }
         }           /* end of MEMCMP */
      }              /* end of for loop */
   }                 /* end of REF_CNT_SET */
#endif

   if((m_NCS_IPXS_IS_IPAM_ADDR_SET(ipinfo->ip_attr))
 #if (NCS_VIP == 1)
        || ((m_NCS_IPXS_IS_IPAM_VIP_SET(ipinfo->ip_attr)) &&
           (!m_NCS_IPXS_IS_IPAM_VIP_REFCNT_SET(ipinfo->ip_attr)))
#endif
           )
   {
    if(ifip_info->is_v4_unnmbrd == NCS_SNMP_FALSE)
    {
      /* This means that we are allowed to add the interfaces. */
      m_NCS_IPXS_IPAM_ADDR_SET(ifip_info->ipam);
      rc = ipxs_ifd_proc_ipaddr(cb, ifip_info, ipinfo);
    }
   }

   if(m_NCS_IPXS_IS_IPAM_RRTRID_SET(ipinfo->ip_attr))
   {
      m_NCS_IPXS_IPAM_RRTRID_SET(ifip_info->ipam);
      ifip_info->rrtr_rtr_id = ipinfo->rrtr_rtr_id;
   }

   if(m_NCS_IPXS_IS_IPAM_RMT_AS_SET(ipinfo->ip_attr))
   {
      m_NCS_IPXS_IPAM_RMT_AS_SET(ifip_info->ipam);
      ifip_info->rrtr_as_id = ipinfo->rrtr_as_id;
   }

   if(m_NCS_IPXS_IS_IPAM_RMTIFID_SET(ipinfo->ip_attr))
   {
      m_NCS_IPXS_IPAM_RMTIFID_SET(ifip_info->ipam);
      ifip_info->rrtr_if_id = ipinfo->rrtr_if_id;
   }


   if(m_NCS_IPXS_IS_IPAM_UD_1_SET(ipinfo->ip_attr))
   {
      m_NCS_IPXS_IPAM_UD_1_SET(ifip_info->ipam);
      ifip_info->ud_1 = ipinfo->ud_1;
   }

#if (NCS_VIP == 1)
     if((m_NCS_IPXS_IS_IPAM_VIP_SET(ipinfo->ip_attr)) &&
           (!m_NCS_IPXS_IS_IPAM_VIP_REFCNT_SET(ipinfo->ip_attr)))
     {
        m_NCS_STRCPY(ifip_info->intfName,ipinfo->intfName);
        ifip_info->shelfId    = ipinfo->shelfId;
        ifip_info->slotId     = ipinfo->slotId;
        /* embedding subslot changes */
        ifip_info->subslotId     = ipinfo->subslotId; 
        ifip_info->nodeId     = ipinfo->nodeId;
     }
#endif

   return NCSCC_RC_SUCCESS;

free_mem:

   return rc;

} /* function ifd_ipxs_proc_data_ifip_info() ends here. */

uns32 ipxs_ifd_proc_v4_unnmbrd(IPXS_CB *cb, IPXS_IFIP_INFO *ifip_info, uns32 unnmbrd)
{
   if(unnmbrd == NCS_SNMP_TRUE)
   {
      ipxs_ip_record_list_del(cb, ifip_info);
   }

   return NCSCC_RC_SUCCESS;
}

static uns32 ipxs_ifd_proc_ipaddr(IPXS_CB *cb, IPXS_IFIP_INFO *ifip_info, 
                           NCS_IPXS_IPINFO *ip_info)
{
   uns32               i, index=0, rc=NCSCC_RC_SUCCESS;
   NCS_IP_ADDR_TYPE    type=0;
   IPXS_IP_KEY         ipkeyNet;
   IPXS_IP_NODE        *ip_node; 
   NCS_BOOL            flag = TRUE;
   IPXS_IFIP_IP_INFO   ippfx;
   
   /* First Process the Add list & then process the delete list */
   for(i=0; i<ip_info->addip_cnt; i++)
   {
      type = ip_info->addip_list[i].ipaddr.ipaddr.type;

      ipxs_get_ipkey_from_ipaddr(&ipkeyNet, &ip_info->addip_list[i].ipaddr.ipaddr);
   
      /* Get the record for the given ifKey */
      rc = ipxs_iprec_get(cb, &ipkeyNet, type, &ip_node, FALSE);
      
      if(ip_node->ipinfo.if_index == 0)
      {
         /* This is the new IP info record, update the record if if_index */
         ip_node->ipinfo.if_index = ifip_info->ifindexNet;
         ip_node->ipinfo.status = NCS_ROW_ACTIVE;
         ip_node->ipinfo.mask = ip_info->addip_list[i].ipaddr.mask_len;
         
         m_NCS_OS_MEMSET(&ippfx, 0, sizeof(IPXS_IFIP_IP_INFO));
         ippfx.ipaddr.ipaddr   = ip_info->addip_list[i].ipaddr.ipaddr;
         ippfx.ipaddr.mask_len = ip_info->addip_list[i].ipaddr.mask_len;
#if (NCS_VIP == 1) 
         if (m_NCS_IPXS_IS_IPAM_VIP_SET(ip_info->ip_attr))
         {
            /* TBD : RECEIVED MSG TO ADD VIP INFO */
            ippfx.poolHdl    = ip_info->addip_list[i].poolHdl; 
            ippfx.ipPoolType = ip_info->addip_list[i].ipPoolType;
            /* Since this is the first one ref cnt will be 1 */
            ippfx.refCnt     = ip_info->addip_list[i].refCnt;
            ippfx.vipIp      = ip_info->addip_list[i].vipIp;

         }
#endif         
         /* Add IP address to ifip info */
         rc = ipxs_ifip_ippfx_add(cb, ifip_info, &ippfx, 1, &flag);
      }
      else
      {
         /* This address is assigned to some interface, just ignore this  */
         /* TBD Log */
      }
   }
  
   for(i=0; i<ip_info->delip_cnt; i++)
   {
      type = ip_info->delip_list[i].ipaddr.type;

      ipxs_get_ipkey_from_ipaddr(&ipkeyNet, &ip_info->delip_list[i].ipaddr);
   
      /* Get the record for the given ifKey */
      rc = ipxs_iprec_get(cb, &ipkeyNet, type, &ip_node, TRUE);
      
      if(ip_node)
      {
         ipxs_ip_record_del(cb, &cb->ip_tbl[index], ip_node);
         
         m_NCS_OS_MEMSET(&ippfx, 0, sizeof(IPXS_IFIP_IP_INFO));
         ippfx.ipaddr.ipaddr   = ip_info->delip_list[i].ipaddr;
         ippfx.ipaddr.mask_len = ip_info->delip_list[i].mask_len;
         
         /* Del IP address to ifip info */
         rc = ipxs_ifip_ippfx_del(cb, ifip_info, &ippfx, 1, &flag);
      }
   }
   
   return NCSCC_RC_SUCCESS;
}

uns32 ifd_ipxs_evt_process(IFSV_CB *cb, IFSV_EVT *evt)
{
   uns32    rc = NCSCC_RC_SUCCESS;
   IPXS_CB  *ipxs_cb;
   uns32    ipxs_hdl;
   IPXS_EVT *ipxs_evt = (IPXS_EVT *) evt->info.ipxs_evt;

   if(!ipxs_evt)
      return rc;

   /* Get the IPXS CB */
   ipxs_hdl = m_IPXS_CB_HDL_GET( );
   ipxs_cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, ipxs_hdl);

   switch(ipxs_evt->type)
   {
   case IPXS_EVT_NDTOD_IFIP_INFO:
      rc = ifd_ipxs_proc_ifip_info(ipxs_cb, ipxs_evt, &evt->sinfo);
      break;

   default:
      rc = NCSCC_RC_FAILURE;
      break;
   }

   /* Free the received event */
    m_MMGR_FREE_IPXS_EVT(ipxs_evt); 
    evt->info.ipxs_evt = NULL;

   /* Give the Handle */
   ncshm_give_hdl(ipxs_hdl);

   return rc;
}
