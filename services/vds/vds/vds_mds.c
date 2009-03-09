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
  MODULE NAME:       vds_mds.c


  DESCRIPTION:

******************************************************************************
*/

#include "vds.h"

static MDS_CLIENT_MSG_FORMAT_VER
    gl_vds_wrt_vda_msg_fmt_array[VDS_WRT_VDA_SUBPART_VER_RANGE]={
           1 /*msg format version for VDS subpart version 1*/};

/* Static function prototypes */
static uns32 vds_mds_callback    (struct ncsmds_callback_info *info);
static uns32 vds_mds_role_ack    (VDS_CB *vds, struct ncsmds_callback_info *info);
static uns32 vds_mds_cb_cpy      (VDS_CB *vds, struct ncsmds_callback_info *info);
static uns32 vds_mds_cb_enc      (VDS_CB *vds, struct ncsmds_callback_info *info);
static uns32 vds_mds_cb_dec      (VDS_CB *vds, struct ncsmds_callback_info *info);
static uns32 vds_mds_cb_rcv      (VDS_CB *vds, struct ncsmds_callback_info *info);
static uns32 vds_mds_cb_enc_flat (VDS_CB *vds, struct ncsmds_callback_info *info);
static uns32 vds_mds_cb_dec_flat (VDS_CB *vds, struct ncsmds_callback_info *info);
static uns32 vds_mds_cb_svc_event(VDS_CB *vds, struct ncsmds_callback_info *info);
static uns32 dummy               (VDS_CB *vds, struct ncsmds_callback_info *info);


/****************************************************************************
  Name          :  vds_role_agent_callback

  Description   :  This routine creates & initializes the PWE of VDS. 

  Arguments     :  vds - pointer to VDS control block  
                   role  - role that needs to be assigned 
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
  Notes         :  None
******************************************************************************/
uns32 vds_role_agent_callback(VDS_CB *vds, V_DEST_RL role)
{
   NCSMDS_INFO svc_info;
   NCS_SPIR_REQ_INFO spir_req;
   MDS_SVC_ID subscribe_list = NCSMDS_SVC_ID_VDA;
   uns32 rc = NCSCC_RC_SUCCESS;

   VDS_TRACE1_ARG2("vds_role_agent_callback:role=%d\n", role);

   if (vds->mds_init_done)
   {
      rc =  vda_chg_role_vdest(&vds->vds_vdest, role);
       
      if (rc != NCSCC_RC_SUCCESS)
      {
          m_VDS_LOG_MDS(VDS_LOG_VDA_ROLECHANGE,
                             VDS_LOG_MDS_FAILURE,
                                     NCSFL_SEV_ERROR);
      }
      else
      {
         m_VDS_LOG_MDS(VDS_LOG_VDA_ROLECHANGE,
                            VDS_LOG_MDS_SUCCESS,
                                    NCSFL_SEV_INFO);
     }

     return rc;
   }

   vds->mds_init_done = TRUE;

   /* We have reached here first time. We need to do mds-initialization */
   m_NCS_MEMSET(&spir_req, 0, sizeof(spir_req));

   spir_req.type = NCS_SPIR_REQ_LOOKUP_CREATE_INST;
   spir_req.i_environment_id = 1;
   m_MDS_FIXED_VDEST_TO_INST_NAME(VDS_VDEST_ID, &spir_req.i_instance_name);
   spir_req.i_sp_abstract_name = m_MDS_SP_ABST_NAME;
   spir_req.info.lookup_create_inst.i_inst_attrs.number = 0;
   rc = ncs_spir_api(&spir_req);
   if (rc != NCSCC_RC_SUCCESS)
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   vds->mds_hdl = spir_req.info.lookup_create_inst.o_handle;

   /* STEP : Install on VDEST with MDS as service NCSMDS_SVC_ID_VDS. */
   m_NCS_MEMSET(&svc_info, 0, sizeof(NCSMDS_INFO));
   svc_info.i_mds_hdl = vds->mds_hdl;
   svc_info.i_svc_id  = NCSMDS_SVC_ID_VDS;
   svc_info.i_op      = MDS_INSTALL;
   svc_info.info.svc_install.i_mds_q_ownership = FALSE;
   svc_info.info.svc_install.i_svc_cb = vds_mds_callback;
   svc_info.info.svc_install.i_yr_svc_hdl = (MDS_CLIENT_HDL)vds->cb_hdl;
   svc_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;
   svc_info.info.svc_install.i_mds_svc_pvt_ver = VDS_SVC_PVT_VER;

   /* Invoke MDS api */
   rc = ncsmds_api(&svc_info);
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_VDS_LOG_MDS_VER("MDS_INTALL","FAILURE",NCSMDS_SVC_ID_VDS,\
                        "VDS_SVC_PVT_VER",svc_info.info.svc_install.i_mds_svc_pvt_ver);

      return NCSCC_RC_FAILURE;
   }
   else
   {

      m_VDS_LOG_MDS_VER("MDS_INTALL","SUCCESS",NCSMDS_SVC_ID_VDS,\
                        "VDS_SVC_PVT_VER",svc_info.info.svc_install.i_mds_svc_pvt_ver);

   }

   /* STEP : Set the role of the VDEST that I am living on */
   vds->vds_vdest = svc_info.info.svc_install.o_dest;

   /* STEP : Subscribe to VDA up/down events */
   svc_info.i_op = MDS_SUBSCRIBE;
   svc_info.info.svc_subscribe.i_num_svcs = 1;
   svc_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_NONE;
   svc_info.info.svc_subscribe.i_svc_ids = &subscribe_list;

   /* Invoke MDS api */
   rc = ncsmds_api(&svc_info);
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_VDS_LOG_MDS(VDS_LOG_MDS_SUBSCRIBE,
                            VDS_LOG_MDS_FAILURE,
                                    NCSFL_SEV_ERROR);
      
      vds_mds_unreg(vds);                      /* review MDS vds_mds_unreg should be 
                                                      call */
      vds->mds_init_done = FALSE;
      return NCSCC_RC_FAILURE;
   }
   else
   {
      m_VDS_LOG_MDS(VDS_LOG_MDS_SUBSCRIBE,
                            VDS_LOG_MDS_SUCCESS,
                                NCSFL_SEV_INFO);
   }

   /* STEP : Set the role of the VDEST that I am living on */
   rc = vda_chg_role_vdest(&vds->vds_vdest, role);
   if (rc != NCSCC_RC_SUCCESS)
   {

      m_VDS_LOG_MDS(VDS_LOG_VDA_ROLECHANGE,
                             VDS_LOG_MDS_FAILURE,
                                     NCSFL_SEV_ERROR);
      return rc;
   }
   else
   {
      m_VDS_LOG_MDS(VDS_LOG_VDA_ROLECHANGE,
                            VDS_LOG_MDS_SUCCESS,
                                    NCSFL_SEV_INFO);
   }

    /* Initialize the interface with CkPt */
   rc = vds_ckpt_initialize(vds);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_VDS_LOG_CKPT(VDS_LOG_CKPT_INITIALIZE,
                             VDS_LOG_CKPT_FAILURE,
                                     NCSFL_SEV_ERROR, rc);
       ncshm_give_hdl((uns32)gl_vds_hdl);
      vds_destroy(VDS_DONT_CANCEL_THREAD);
      return rc;   
   }
   else
   {
     m_VDS_LOG_CKPT(VDS_LOG_CKPT_INITIALIZE,
                            VDS_LOG_CKPT_SUCCESS,
                                    NCSFL_SEV_INFO, rc);
   }

  return rc;
}


/****************************************************************************
  Name          :  vds_mds_unreg
 
  Description   :  This routine unregisters the VDS service from MDS.

  Arguments     :  vds - ptr to the VDS Control Block
 
  Return Values :  NCSCC_RC_SUCCESS (or)
                   NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32  vds_mds_unreg(VDS_CB *vds)
{
   NCSMDS_INFO  mds_info;
   uns32        rc = NCSCC_RC_SUCCESS;

   VDS_TRACE1_ARG1("vds_mds_unreg\n");

   vds->mds_init_done = FALSE;

   m_NCS_OS_MEMSET(&mds_info, 0, sizeof(NCSMDS_INFO));

   mds_info.i_mds_hdl = vds->mds_hdl;
   mds_info.i_svc_id  = NCSMDS_SVC_ID_VDS;
   mds_info.i_op = MDS_UNINSTALL;

   rc = ncsmds_api(&mds_info);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_VDS_LOG_MDS(VDS_LOG_MDS_UNREG,
                               VDS_LOG_MDS_FAILURE,
                                        NCSFL_SEV_ERROR);
   }
   else
   {
      m_VDS_LOG_MDS(VDS_LOG_MDS_UNREG,
                            VDS_LOG_MDS_SUCCESS,
                                    NCSFL_SEV_INFO);
   }
   return rc;
}


/****************************************************************************
  Name          :  vds_mds_callback
  Description   :  This callback routine will be called by MDS on event arrival
  Arguments     :  info - a pointer to mds_callback_info structure

  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
  Notes         :  None
******************************************************************************/
uns32 vds_mds_callback(struct ncsmds_callback_info *info)
{
   VDS_CB *vds = NULL;
   uns32  rc = NCSCC_RC_SUCCESS;

   VDS_TRACE1_ARG1("vds_mds_callback\n");

   m_VDS_LOG_MDS(VDS_LOG_MDS_CALL_BACK,
                           VDS_LOG_MDS_NOTHING,
                                  NCSFL_SEV_INFO); 
   
   static vds_mds_cbk_routine cb_set[MDS_CALLBACK_SVC_MAX] = {
         vds_mds_cb_cpy,       /* MDS_CALLBACK_COPY      */
         vds_mds_cb_enc,       /* MDS_CALLBACK_ENC       */
         vds_mds_cb_dec,       /* MDS_CALLBACK_DEC       */
         vds_mds_cb_enc_flat,  /* MDS_CALLBACK_ENC_FLAT  */
         vds_mds_cb_dec_flat,  /* MDS_CALLBACK_DEC_FLAT  */
         vds_mds_cb_rcv,       /* MDS_CALLBACK_RECEIVE   */
         vds_mds_cb_svc_event, /* MDS_CALLBACK_SVC_EVENT */
         dummy,                /* MDS_CALLBACK_SYS_EVENT */
         vds_mds_role_ack,     /* MDS_CALLBACK_QUIESCED_ACK */
         dummy                 /* MDS_CALLBACK_DIRECT_RECEIVE */
   };

   /* retrieve vds cb */
   if ((vds = (VDS_CB *)ncshm_take_hdl(NCS_SERVICE_ID_VDS, 
                              (uns32)info->i_yr_svc_hdl)) == NULL)
   {
      /* LOG the message */
      return NCSCC_RC_FAILURE;
   }

   rc = (*cb_set[info->i_op])(vds, info);

   ncshm_give_hdl((uns32)info->i_yr_svc_hdl);

   return rc;
}


static uns32 dummy(VDS_CB *vds, struct ncsmds_callback_info *info)
{

   VDS_TRACE1_ARG1("dummy\n");
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          :  vds_mds_cb_cpy
  Description   :  

  Arguments     :  vds - ptr to VDS_CB
                   ncsmds_callback_info - ptr to ncsmds_callback_info 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
  Notes         :  None
******************************************************************************/
uns32 vds_mds_cb_cpy(VDS_CB *vds, struct ncsmds_callback_info *info)
{
   NCSVDA_INFO *req_vda_info, *rsp_vda_info;
   MDS_CLIENT_MSG_FORMAT_VER  msg_fmt_version;

   /* Need NOT allocate a new copy. VDA always uses blocking MDS calls,
    * The blocking call made by VDA does not return until the
    * VDS receive callback is called (even in the case of a timeout).
    */
   VDS_TRACE1_ARG1("vds_mds_cb_cpy\n");

   m_VDS_LOG_MDS(VDS_LOG_MDS_CPY_CBK,
                           VDS_LOG_MDS_NOTHING,
                                  NCSFL_SEV_INFO); 
   
   if (info->info.cpy.o_cpy == NULL)
      return NCSCC_RC_FAILURE;

   msg_fmt_version = m_NCS_VDS_ENC_MSG_FMT_GET(
                               info->info.cpy.i_rem_svc_pvt_ver,
                               VDS_WRT_VDA_SUBPART_VER_MIN,
                               VDS_WRT_VDA_SUBPART_VER_MAX,
                               gl_vds_wrt_vda_msg_fmt_array); 
   if (0 == msg_fmt_version)
   {

      m_VDS_LOG_MDS_VER("MDS Copy Call Back","Failure Msg to ",info->info.cpy.i_to_svc_id,\
                        "Remote Service Pvt Version",info->info.cpy.i_rem_svc_pvt_ver);

      return NCSCC_RC_FAILURE;

   }
   info->info.cpy.o_msg_fmt_ver = msg_fmt_version;  

   req_vda_info  = (NCSVDA_INFO *)info->info.cpy.i_msg;
   rsp_vda_info  = (NCSVDA_INFO *)info->info.cpy.o_cpy;
   *rsp_vda_info = *req_vda_info;

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  vds_mds_cb_dec
  Description   :  
  Arguments     :  vds - ptr to VDS_CB
                   ncsmds_callback_info - ptr to ncsmds_callback_info 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
  Notes         :  None
******************************************************************************/
uns32 vds_mds_cb_dec(VDS_CB *vds, struct ncsmds_callback_info *info)
{
   NCSVDA_INFO *vda_info;
   NCS_UBAID   *uba;

   VDS_TRACE1_ARG1("vds_mds_cb_dec\n");
   m_VDS_LOG_MDS(VDS_LOG_MDS_DEC_CBK,
                           VDS_LOG_MDS_NOTHING,
                                  NCSFL_SEV_INFO); 
   if( 0 == m_NCS_VDS_MSG_FORMAT_IS_VALID(info->info.dec.i_msg_fmt_ver,
                               VDS_WRT_VDA_SUBPART_VER_MIN,
                               VDS_WRT_VDA_SUBPART_VER_MAX,
                               gl_vds_wrt_vda_msg_fmt_array)) 
     {
        m_VDS_LOG_MDS_VER("Decode Call Back","Failure : Msg From",info->info.dec.i_fr_svc_id,\
                        "Message Format Version",info->info.dec.i_msg_fmt_ver);      
        return NCSCC_RC_FAILURE;
     }
 
   vda_info = m_MMGR_ALLOC_NCSVDA_INFO;
   if (vda_info == NULL)
   {
      m_VDS_LOG_MDS(VDS_LOG_MEM_VDA_INFO,
                            VDS_LOG_MEM_ALLOC_FAILURE,
                                    NCSFL_SEV_CRITICAL);             
      return NCSCC_RC_FAILURE;
   }
   m_NCS_OS_MEMSET(vda_info, 0, sizeof(*vda_info));

   info->info.dec.o_msg = vda_info;
   uba = info->info.dec.io_uba;

   vda_info->req = vda_util_dec_8bit(uba);

   switch (vda_info->req)
   {
   case NCSVDA_VDEST_CREATE:
      /* DECODE: An 8-bit flag */
      vda_info->info.vdest_create.i_persistent = vda_util_dec_8bit(uba);

      /* DECODE: An 8-bit enum value */
      vda_info->info.vdest_create.i_policy = vda_util_dec_8bit(uba);

      /* DECODE: String name */
      vda_util_dec_vdest_name(uba, &vda_info->info.vdest_create.info.named.i_name);
      break;

   case NCSVDA_VDEST_LOOKUP:
      /* ENCODE: String name */
      vda_util_dec_vdest_name(uba, &vda_info->info.vdest_lookup.i_name);
      break;

   case NCSVDA_VDEST_DESTROY:
      /* ENCODE: String name */
      vda_util_dec_vdest_name(uba, &vda_info->info.vdest_destroy.i_name);
      break;

   default:
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  vds_mds_cb_enc  
  Description   :                                                              
  Arguments     :  vds - ptr to VDS_CB
                   ncsmds_callback_info - ptr to ncsmds_callback_info 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
  Notes         :  None
******************************************************************************/
uns32 vds_mds_cb_enc(VDS_CB *vds, struct ncsmds_callback_info *info)
{
   NCSVDA_INFO *vda_info;
   NCS_UBAID   *uba = info->info.enc.io_uba;
   MDS_CLIENT_MSG_FORMAT_VER  msg_fmt_version;

   VDS_TRACE1_ARG1("vds_mds_cb_enc\n");



   msg_fmt_version = m_NCS_VDS_ENC_MSG_FMT_GET(
                               info->info.enc.i_rem_svc_pvt_ver,
                               VDS_WRT_VDA_SUBPART_VER_MIN,
                               VDS_WRT_VDA_SUBPART_VER_MAX,
                               gl_vds_wrt_vda_msg_fmt_array); 
   if (0 == msg_fmt_version)
   {
         m_VDS_LOG_MDS_VER("Encode Callback ","Failure: Msg to" ,info->info.enc.i_to_svc_id,\
                        "Remote Service Pvt Version",info->info.enc.i_rem_svc_pvt_ver);

         return NCSCC_RC_FAILURE;

   }
   info->info.enc.o_msg_fmt_ver = msg_fmt_version;


   m_VDS_LOG_MDS(VDS_LOG_MDS_ENC_CBK,
                           VDS_LOG_MDS_NOTHING,
                                  NCSFL_SEV_INFO); 
   /* The is the VDS's response to a NCSVDA_VDEST_CREATE request */
   vda_info = info->info.enc.i_msg;

   vda_util_enc_8bit(uba, (uns8)vda_info->req);
   vda_util_enc_8bit(uba, (uns8)vda_info->o_result);

   switch (vda_info->req)
   {
   case NCSVDA_VDEST_CREATE:
      /* ENCODE: vdest */
      vda_util_enc_vdest(uba, &vda_info->info.vdest_create.info.named.o_vdest);
      break;

   case NCSVDA_VDEST_LOOKUP:
      /* ENCODE: vdest */
      vda_util_enc_vdest(uba, &vda_info->info.vdest_lookup.o_vdest);
      break;

   case NCSVDA_VDEST_DESTROY:
      /* ENCODE: vdest */
      vda_util_enc_vdest(uba, &vda_info->info.vdest_destroy.i_vdest);
      break;

   default:
      return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  vds_mds_cb_enc_flat 
  Description   :  This function encodes an events sent from VDA
  Arguments     :  vds - ptr to VDS_CB
                   ncsmds_callback_info - ptr to ncsmds_callback_info 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
  Notes         :  None
******************************************************************************/
uns32 vds_mds_cb_enc_flat(VDS_CB *vds, struct ncsmds_callback_info *info)
{
   NCSVDA_INFO *vda_info;
   NCS_UBAID   *uba = info->info.enc.io_uba;
   MDS_CLIENT_MSG_FORMAT_VER  msg_fmt_version;

   VDS_TRACE1_ARG1("vds_mds_cb_enc_flat\n");
   

   msg_fmt_version = m_NCS_VDS_ENC_MSG_FMT_GET(
                               info->info.enc_flat.i_rem_svc_pvt_ver,
                               VDS_WRT_VDA_SUBPART_VER_MIN,
                               VDS_WRT_VDA_SUBPART_VER_MAX,
                               gl_vds_wrt_vda_msg_fmt_array); 
   if (0 == msg_fmt_version)
   {
      m_VDS_LOG_MDS_VER("Encode Flat Call BK" ,"Failure:Msg to",info->info.enc_flat.i_to_svc_id,\
                        "Remote Service Pvt Version",info->info.enc_flat.i_rem_svc_pvt_ver);
      return NCSCC_RC_FAILURE;

   }
   info->info.enc_flat.o_msg_fmt_ver = msg_fmt_version;


   /* Structure is already flat. This can be done real quick */
   vda_info = info->info.enc.i_msg;
   vda_util_enc_n_octets(uba, sizeof(*vda_info), (uns8*)vda_info);

   m_VDS_LOG_MDS(VDS_LOG_MDS_FLAT_ENC_CBK,
                           VDS_LOG_MDS_NOTHING,
                                  NCSFL_SEV_INFO); 
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  vds_mds_role_ack

  Description   :  Got the acknowledgement/confirmation from MDS about the 
                   ROLE change, now act accordingly.

  Arguments     :  vds - ptr to VDS_CB
                   ncsmds_callback_info - ptr to ncsmds_callback_info 

  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
  Notes         :  None
******************************************************************************/
uns32 vds_mds_role_ack(VDS_CB *vds, struct ncsmds_callback_info *info)
{

   SaAisErrorT status = SA_AIS_OK;
   VDS_TRACE1_ARG1("vds_mds_role_ack\n");

   if (vds->amf.ha_cstate == VDS_HA_STATE_ACTIVE) 
      vds_db_scavanger(vds); 

      /* Now response to AMF */
   if(vds_role_ack_to_state == VDS_HA_STATE_QUIESCED)
   {
     vds->amf.ha_cstate = VDS_HA_STATE_QUIESCED;  

            if((status = saAmfResponse(vds->amf.amf_handle, vds->amf.invocation, SA_AIS_OK))!= SA_AIS_OK)
            {
               m_VDS_LOG_AMF(VDS_LOG_AMF_RESPONSE,
                                  VDS_LOG_AMF_FAILURE,
                                          NCSFL_SEV_ERROR, status);
            }
            vds_role_ack_to_state = VDS_HA_STATE_NULL;

     m_VDS_LOG_AMF(VDS_LOG_AMF_RESPONSE,
                                VDS_LOG_AMF_SUCCESS,
                                        NCSFL_SEV_NOTICE, status);
   }
   else
   if(vds_role_ack_to_state == VDS_HA_STATE_QUIESCING)
   { 
     vds->amf.ha_cstate = VDS_HA_STATE_QUIESCED;  
       /* QUIESCING is complete */
    if((status = saAmfCSIQuiescingComplete(vds->amf.amf_handle, vds->amf.invocation,
                                          SA_AIS_OK))!= SA_AIS_OK)
    {
       m_VDS_LOG_AMF(VDS_LOG_AMF_RESPONSE,
                                  VDS_LOG_AMF_FAILURE,
                                          NCSFL_SEV_ERROR, status);
    } 
    vds_role_ack_to_state = VDS_HA_STATE_NULL;
   }

   m_VDS_LOG_MDS(VDS_LOG_MDS_ROLE_ACK,
                           VDS_LOG_MDS_NOTHING,
                                  NCSFL_SEV_NOTICE); 
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  vds_mds_cb_dec_flat
  Description   :  This function decodes an events came from VDA
  Arguments     :  vds - ptr to VDS_CB
                   ncsmds_callback_info - ptr to ncsmds_callback_info
                   (info to decode) 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
  Notes         :  None
******************************************************************************/
uns32 vds_mds_cb_dec_flat(VDS_CB *vds, struct ncsmds_callback_info *info)
{
   NCSVDA_INFO *vda_info;
   NCS_UBAID   *uba;

   VDS_TRACE1_ARG1("vds_mds_cb_dec_flat\n");
 
   if( 0 == m_NCS_VDS_MSG_FORMAT_IS_VALID(info->info.dec_flat.i_msg_fmt_ver,
                               VDS_WRT_VDA_SUBPART_VER_MIN,
                               VDS_WRT_VDA_SUBPART_VER_MAX,
                               gl_vds_wrt_vda_msg_fmt_array)) 
    {
      m_VDS_LOG_MDS_VER("Decode Flat Call Back","Failure Msg From",info->info.dec_flat.i_fr_svc_id,\
           "Message Format Version" ,info->info.dec_flat.i_msg_fmt_ver);
      return NCSCC_RC_FAILURE;
  
    } 

   vda_info = m_MMGR_ALLOC_NCSVDA_INFO;
   if (vda_info == NULL)
   {  
      m_VDS_LOG_MDS(VDS_LOG_MEM_VDA_INFO,
                            VDS_LOG_MEM_ALLOC_FAILURE,
                                    NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }
   info->info.dec_flat.o_msg = vda_info;
   uba = info->info.dec_flat.io_uba;

   vda_util_dec_n_octets(uba, sizeof(*vda_info), (uns8*)vda_info);

   m_VDS_LOG_MDS(VDS_LOG_MDS_FLAT_DEC_CBK,
                           VDS_LOG_MDS_NOTHING,
                                  NCSFL_SEV_INFO); 

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  vds_mds_cb_rcv
  Description   :  MDS will call this function on receiving event from VDA.
  Arguments     :  vds - ptr to VDS_CB
                   ncsmds_callback_info - ptr to ncsmds_callback_info 
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
  Notes         :  None
******************************************************************************/
uns32 vds_mds_cb_rcv(VDS_CB *vds, struct ncsmds_callback_info *info)
{
   VDS_EVT *vds_evt = NULL;
   uns32   rc = NCSCC_RC_SUCCESS;
   VDS_VDA_EVT vda_evt_msg;

   VDS_TRACE1_ARG1("vds_mds_cb_rcv\n");
   
   m_VDS_LOG_MDS(VDS_LOG_MDS_RCV_CBK,
                           VDS_LOG_MDS_NOTHING,
                                  NCSFL_SEV_INFO); 
   if( 0 == m_NCS_VDS_MSG_FORMAT_IS_VALID(info->info.receive.i_msg_fmt_ver,
                               VDS_WRT_VDA_SUBPART_VER_MIN,
                               VDS_WRT_VDA_SUBPART_VER_MAX,
                               gl_vds_wrt_vda_msg_fmt_array))
   {
      m_VDS_LOG_MDS_VER("Receive Call Back","Failure : Msg From",info->info.receive.i_fr_svc_id,\
                        "Message Format Version" ,info->info.receive.i_msg_fmt_ver);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(&vda_evt_msg,0,sizeof(VDS_VDA_EVT));
 
    vda_evt_msg.i_msg_ctxt = info->info.receive.i_msg_ctxt;
    vda_evt_msg.dest = info->info.receive.i_fr_dest;
    vda_evt_msg.vda_info = (NCSVDA_INFO*)info->info.receive.i_msg;
 
   /* Create an event for VDS */
   vds_evt = vds_evt_create((NCSCONTEXT)&vda_evt_msg,
                            VDS_VDA_EVT_TYPE);
   /* send the event */
   if (vds_evt)
      rc = m_NCS_IPC_SEND(&vds->mbx, vds_evt, NCS_IPC_PRIORITY_NORMAL);

   /* if failure, free the event */
   if ((rc != NCSCC_RC_SUCCESS) && (vds_evt))
      vds_evt_destroy(vds_evt);
  
   return rc;
}


/****************************************************************************
  Name          :  vds_mds_cb_svc_event
  Description   :  VDS is informed about the occurance of events that are
                   subscribed by VDS. 

  Arguments     :  vds - ptr to VDS_CB 
                   ncsmds_callback_info - ptr to ncsmds_callback_info

  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
  Notes         :  None
******************************************************************************/
uns32 vds_mds_cb_svc_event(VDS_CB *vds, struct ncsmds_callback_info *info)
{
   VDS_TRACE1_ARG1("vds_mds_cb_svc_event\n");
   VDS_EVT *vds_evt = NULL;
   uns32   rc = NCSCC_RC_SUCCESS;

   if (info->info.svc_evt.i_svc_id != NCSMDS_SVC_ID_VDA) 
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   if (info->info.svc_evt.i_svc_id == NCSMDS_SVC_ID_VDA)
   {
      switch (info->info.svc_evt.i_change)
      {
      case NCSMDS_UP:
          VDS_TRACE1_ARG2("vds_mds_cb_svc_event:chng=%s\n", "UP");
          /* Nothing much to do here */
          break;

      case NCSMDS_DOWN:

          if (vds->amf.ha_cstate != VDS_HA_STATE_ACTIVE) 
             return NCSCC_RC_SUCCESS;

           /* Create an event for VDS */
           vds_evt = vds_evt_create((NCSCONTEXT)&info->info.svc_evt.i_dest,
                                               VDS_VDA_DESTROY_EVT_TYPE);
           if (vds_evt)
             rc = m_NCS_IPC_SEND(&vds->mbx, vds_evt, NCS_IPC_PRIORITY_NORMAL);

           if ((rc != NCSCC_RC_SUCCESS) && (vds_evt))
             vds_evt_destroy(vds_evt);
             break;


      default:
          break;
      }
   }
   
   m_VDS_LOG_MDS(VDS_LOG_MDS_SVEVT_CBK,
                           VDS_LOG_MDS_NOTHING,
                                  NCSFL_SEV_INFO); 
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  vds_mds_send

  Description   :   This routine sends the VDS message to VDA 
                   
  Arguments     :  vds  - ptr to VDS_CB
                   vds_evt - ptr to VDS_EVT
 
  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
  Notes         :  None
******************************************************************************/
uns32 vds_mds_send(VDS_CB *vds, VDS_EVT *vds_evt)
{
   NCSMDS_INFO send_info;
   uns32 rc = NCSCC_RC_SUCCESS;

   VDS_TRACE1_ARG1("vds_mds_send\n");

   m_NCS_OS_MEMSET(&send_info, 0, sizeof(send_info));

   send_info.i_op = MDS_SEND;
   send_info.i_svc_id = NCSMDS_SVC_ID_VDS;
   send_info.i_mds_hdl = vds->mds_hdl;

   send_info.info.svc_send.i_sendtype = MDS_SENDTYPE_RSP;
   send_info.info.svc_send.i_msg = vds_evt->evt_data.vda_evt_msg.vda_info;
   send_info.info.svc_send.i_to_svc = NCSMDS_SVC_ID_VDA;
   send_info.info.svc_send.i_priority = MDS_SEND_PRIORITY_LOW;
   send_info.info.svc_send.info.rsp.i_msg_ctxt = vds_evt->evt_data.vda_evt_msg.i_msg_ctxt;
   send_info.info.svc_send.info.rsp.i_sender_dest = vds_evt->evt_data.vda_evt_msg.dest;
   
   rc = ncsmds_api(&send_info); 
   if (rc == NCSCC_RC_SUCCESS)
   {
      m_VDS_LOG_MDS(VDS_LOG_MDS_SEND,
                        VDS_LOG_MDS_SUCCESS,
                            NCSFL_SEV_INFO);
   }
   else
   {
      m_VDS_LOG_MDS(VDS_LOG_MDS_SEND,
                        VDS_LOG_MDS_FAILURE,
                            NCSFL_SEV_ERROR);
   }

   return rc;
}
