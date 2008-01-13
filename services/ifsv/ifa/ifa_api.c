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
FILE NAME: IFA_API.C
DESCRIPTION: Interface Agent Routines.
******************************************************************************/

#include "ifa.h"

/*****************************************************************************
  PROCEDURE NAME    :   ncs_ifsv_svc_req
  DESCRIPTION       :   This is the SE-API, used by applications to:
                        1. Subscribe and un-subscribe with IfSv for the 
                           interface change notifications
                        2. Getting the interface records.
                        3. Creating and deleting the logical interfaces.

  ARGUMENTS         :   NCS_IFSV_SVC_REQ *arg : Pointer to service req info.

  RETURNS           :   NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
  NOTES             :   This API will be used for both the internal as well 
                        external interfaces. The struct ncs_ifsv_spt has been 
                        modified to have one enum NCS_IFSV_IF_SCOPE to indicate
                        whether the record being created is for internal or 
                        external interfaces. 
                        The user of this API is supposed to fill this field. If
                        this field is empty then this API fills the value as 
                        external interface. This is for backward compatibility. 
*****************************************************************************/

IFADLL_API uns32 ncs_ifsv_svc_req(NCS_IFSV_SVC_REQ *arg)
{
   NCS_IFSV_SVC_REQ_TYPE   type = 0;
   uns32                   rc = NCSCC_RC_FAILURE;
   IFA_CB                  *ifa_cb = NULL;
   uns32                   cb_hdl;

   /* Validate the received information */
   if(arg == NULL)
   {
      /* Log the Error RSR:TBD */
      return NCSCC_RC_FAILURE;
   }

   type = arg->i_req_type;

   cb_hdl = m_IFA_GET_CB_HDL();
   ifa_cb = (IFA_CB *) ncshm_take_hdl(NCS_SERVICE_ID_IFA, cb_hdl);

   if(ifa_cb == NULL)
   {
      m_IFA_LOG_API_LL(IFSV_LOG_IFA_EVT_INFO,"ncshm_take_hdl returned NULL",0);
      return NCSCC_RC_FAILURE;
   }
                                                   
   switch(type)
   {
   case NCS_IFSV_SVC_REQ_SUBSCR:
      rc = ifsv_ifa_subscribe(ifa_cb, &arg->info.i_subr);
      break;

   case NCS_IFSV_SVC_REQ_UNSUBSCR:
      rc = ifsv_ifa_unsubscribe(ifa_cb, &arg->info.i_unsubr);
      break;

   case NCS_IFSV_SVC_REQ_IFREC_GET:
      rc = ifsv_ifa_ifrec_get(ifa_cb, &arg->info.i_ifget, 
                                       &arg->o_ifget_rsp);
      break;

   case NCS_IFSV_SVC_REQ_IFREC_ADD:
      rc = ifsv_ifa_ifrec_add(ifa_cb, &arg->info.i_ifadd);
      if (rc == NCSCC_RC_REQ_TIMOUT)
      {
         rc = SA_AIS_ERR_TRY_AGAIN;
      }

      break;

   case NCS_IFSV_SVC_REQ_IFREC_DEL:
      rc = ifsv_ifa_ifrec_del(ifa_cb, &arg->info.i_ifdel);
      break;

   case NCS_IFSV_REQ_SVC_DEST_UPD:
      rc = ifsv_ifa_svcd_upd(ifa_cb, &arg->info.i_svd_upd);
      break;

   case NCS_IFSV_REQ_SVC_DEST_GET:
      rc = ifsv_ifa_svcd_get(ifa_cb, &arg->info.i_svd_get);
      break;
#if (NCS_VIP == 1)
   case NCS_IFSV_REQ_VIP_INSTALL:
      rc = ncs_ifsv_vip_install(ifa_cb,&arg->info.i_vip_install);
      break;
   case NCS_IFSV_REQ_VIP_FREE:
      rc = ncs_ifsv_vip_free(ifa_cb,&arg->info.i_vip_free);
      break;
#endif

   default:
      break;
   }

   /* Usage of USR_HDL for this struct has finished, return the hdl */
   ncshm_give_hdl(cb_hdl);

   return rc;
}


/****************************************************************************
 * Name          : ncs_ifa_lib_req
 *
 * Description   : This is the NCS SE API which is used to Create/destroy PWE's
 *                 of the IFA.
 *
 * Arguments     : req_info  - This is the pointer to the input information 
 *                             which SBOM gives.  
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE..
 *
 * Notes         : None.
 *****************************************************************************/
IFADLL_API uns32
ifa_lib_req (NCS_LIB_REQ_INFO *req_info)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   switch (req_info->i_op)
   {      
      case NCS_LIB_REQ_CREATE:
         rc = ifa_lib_init(&req_info->info.create);
         break;
      case NCS_LIB_REQ_DESTROY:
         rc = ifa_lib_destroy(&req_info->info.destroy);
         break;
      default:
         break;
   }
   return (rc);
}

/* Header TBD */
NCS_SVDEST* ncs_ifsv_svdest_list_alloc(uns32 i_svd_cnt)
{
   NCS_SVDEST *svd_list;

   svd_list = m_MMGR_ALLOC_IFSV_NCS_SVDEST(i_svd_cnt);
   return svd_list;
}

