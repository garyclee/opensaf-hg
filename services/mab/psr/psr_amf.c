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

  
  .....................................................................  
  
  DESCRIPTION: This file describes AMF Interface implementation for PSS
*****************************************************************************/ 
#include "psr.h"
#include "psr_rfmt.h"

/* following are the globals to be relooked at */ 
extern PSS_ATTRIBS gl_pss_amf_attribs; 

/* int gl_pss_mem_leak_debug = FALSE; */

/* Stethascope of AMF */ 
void 
pss_amf_health_check(SaInvocationT,const SaNameT *, SaAmfHealthcheckKeyT*); 

/* pest for Suicide */ 
void 
pss_amf_comp_terminate(SaInvocationT,  const SaNameT *); 

/* to create a new CSI or to change the state of an existing CSI */ 
void
pss_amf_csi_set(SaInvocationT,  const SaNameT *,
                SaAmfHAStateT, SaAmfCSIDescriptorT); 

/* to remove one or more CSIs from PSS */ 
void
pss_amf_csi_remove(SaInvocationT, const SaNameT*, 
                   const SaNameT*, SaAmfCSIFlagsT); 

/* to create a CSI in ACTIVE state */ 
uns32 
pss_amf_INIT_ACTIVE_process(void*, PW_ENV_ID, SaAmfCSIDescriptorT);

/* to move to ACTIVE state */ 
uns32 
pss_amf_ACTIVE_process (void*, PW_ENV_ID, SaAmfCSIDescriptorT);

/* to create a CSI in STANDBY state */ 
uns32 
pss_amf_INIT_STANDBY_process(void*, PW_ENV_ID, SaAmfCSIDescriptorT);

/* to STANDBY state */ 
uns32 
pss_amf_STANDBY_process(void*, PW_ENV_ID, SaAmfCSIDescriptorT);

/* to QUIESCED state */ 
uns32 
pss_amf_QUIESCED_process(void*, PW_ENV_ID, SaAmfCSIDescriptorT);

/* to QUIESCING state */ 
uns32 
pss_amf_QUIESCING_process(void*, PW_ENV_ID, SaAmfCSIDescriptorT); 

/* from QUIESCING to QUIESCED */ 
uns32 
pss_amf_QUIESCING_QUIESCED_process(void*, PW_ENV_ID, SaAmfCSIDescriptorT);

/* to create a new CSI */ 
void 
pss_amf_csi_new(SaInvocationT,  const SaNameT *,
                SaAmfHAStateT, SaAmfCSIDescriptorT);  

/* to do the role change for all the CSIs */ 
void 
pss_amf_csi_set_all(SaInvocationT,  const SaNameT *,
                SaAmfHAStateT, SaAmfCSIDescriptorT);  

/* do a role change for a specific CSI */ 
void 
pss_amf_csi_set_one(SaInvocationT,  const SaNameT *,
                SaAmfHAStateT, SaAmfCSIDescriptorT);  

/* to remove a single CSI */ 
SaAisErrorT 
pss_amf_csi_remove_one(const PSS_CSI_NODE*); 

/* to create a new Environment */ 
uns32
pss_amf_csi_create_and_install_env(PW_ENV_ID, SaAmfHAStateT, SaAmfCSIDescriptorT); 

/* to change the role of the VDEST */ 
uns32
pss_amf_csi_vdest_role_change(NCS_APP_AMF_CSI_ATTRIBS *, SaAmfHAStateT); 

/* serach by the env-id in the global CSI list */ 
PSS_CSI_NODE* 
pss_amf_csi_list_find_envid(PW_ENV_ID); 

/* search by CSI name in the global CSI list */ 
PSS_CSI_NODE* 
pss_amf_csi_list_find_csiname(const SaNameT *); 

/* delink this node from the global CSI list */ 
PSS_CSI_NODE*  
pss_amf_csi_list_delink(const SaNameT*); 

/* decodes the CSI attributes and gives the Environment ID */ 
PW_ENV_ID
pss_amf_csi_attrib_decode(SaAmfCSIAttributeListT); 

uns32
pss_amf_component_name_set(void); 

static void
pss_amf_sigusr2_handler(int i_sig_num);

EXTERN_C uns32
pss_stdby_oaa_down_list_update(MDS_DEST oaa_addr,NCSCONTEXT yr_hdl,PSS_STDBY_OAA_DOWN_BUFFER_OP buffer_op);


extern NCS_BOOL  gl_inited;
/****************************************************************************\
*  Name:          pss_mbx_amf_process                                        * 
*                                                                            *
*  Description:   This function processes the messages from AMF              *
*                 framework as well as native PSS events on its mail box.    *
*                                                                            *
*  Arguments:     SYSF_MBX * - PSS mail box                                  *
*                                                                            * 
*  Returns:       Nothing.                                                   *
*                                                                            *
*  NOTE:         This is the startup function for PSS thread.                * 
*                Following global varibles are accessed                      *
*                   - gl_pss_amf_attribs                                     *
\****************************************************************************/
void
pss_mbx_amf_process(SYSF_MBX   *pss_mbx)
{
    NCS_SEL_OBJ     numfds, ncs_amf_sel_obj;
#if(NCS_PSS_RED == 1)
    NCS_SEL_OBJ     ncs_mbcsv_sel_obj; 
#endif
    int             count;
    fd_set          readfds;
    
    SaAisErrorT     saf_status = SA_AIS_OK; 
    NCS_SEL_OBJ     mbx_fd;

    uns32           status = NCSCC_RC_FAILURE; 

#if (NCS_PSS_RED == 1)
    NCS_MBCSV_ARG   mbcsv_arg;
#endif

    int            ps_cnt = 0;
    uns32          ps_format_version, tmp_ps0, tmp_ps1;
    struct dirent  **ps_list;
    /* Function pointer for filter routine that need to be passed in scandir */
    int (*filter) (const struct dirent *);
    
    if (pss_mbx == NULL)
    {
        m_LOG_PSS_STR(NCSFL_SEV_CRITICAL, "pss_mbx_amf_process(): NULL mailbox"); 
        return;
    }

    /* install the signal handler for SIGUSR2, to dump the data structure contents */
    if ((ncs_app_signal_install(SIGUSR2,pss_amf_sigusr2_handler)) == -1)
    {
        m_LOG_PSS_STR(NCSFL_SEV_CRITICAL, "pss_mbx_amf_process(): SIGUSR2 install failed"); 
        return;
    }

#if (NCS_PSS_RED == 1)
    memset(&mbcsv_arg, 0, sizeof(NCS_MBCSV_ARG));
    mbcsv_arg.i_op = NCS_MBCSV_OP_DISPATCH;
    mbcsv_arg.i_mbcsv_hdl = gl_pss_amf_attribs.handles.mbcsv_hdl;
    mbcsv_arg.info.dispatch.i_disp_flags = SA_DISPATCH_ONE;
#endif

 /* 3.0.a addition: Get the existing persistent-store's format version.
                    Persistent store with format PSS_PS_FORMAT_VERSION
                    is created in pss_ts_create */
   if(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE)
   {
      /* Using snprintf directly, because the leap routine does not accept variable arguments, which leads to multiple function calls */
      filter = &persistent_file_filter;
      ps_cnt = scandir(NCS_PSS_DEF_PSSV_ROOT_PATH, &ps_list, filter, NULL);
      printf("ps_cnt: %d\n", ps_cnt);
      if (ps_cnt < 0)
          perror("scandir");
      if(ps_cnt > 2)
      {
         /* Log that there are multiple formats of persistent store */
         m_LOG_PSS_STR(NCSFL_SEV_CRITICAL, "Too many formats of persistent store found");
         for(;ps_cnt > 0; ps_cnt--)
            free(ps_list[ps_cnt-1]);
         free(ps_list);
     
         return;
      }
      
      if(ps_cnt >= 1 )
      {
         tmp_ps0 = atoi(ps_list[0]->d_name);
         printf("tmp_ps0: %d\n", tmp_ps0);
      }
      if(ps_cnt == 2)
      {
         tmp_ps1 = atoi(ps_list[1]->d_name);
         printf("tmp_ps0: %d\n", tmp_ps0);
      }
      if(tmp_ps0 == PSS_PS_FORMAT_VERSION)
      {
         if(ps_cnt == 1)
            ps_format_version = tmp_ps0;
         else
            ps_format_version = tmp_ps1;
         for(;ps_cnt > 0; ps_cnt--)
            free(ps_list[ps_cnt-1]);
         free(ps_list);
      }
      else if(tmp_ps1 == PSS_PS_FORMAT_VERSION)
      {
         ps_format_version = tmp_ps0;
         for(;ps_cnt > 0; ps_cnt--)
            free(ps_list[ps_cnt-1]);
         free(ps_list);
      }
      else
      {
         /* if none of the formats returned by scandir is PSS_PS_FORMAT_VERSION, return FAILURE */
         m_LOG_PSS_STR(NCSFL_SEV_CRITICAL, "Invalid formats of persistent store found");
         for(;ps_cnt > 0; ps_cnt--)
            free(ps_list[ps_cnt-1]);
         free(ps_list);
     
         return;
      }
      printf("ps_format_version: %d\n", ps_format_version);
   }
 /* End of 3.0.a addition */

   if(gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE && ps_format_version != 0)
   {
      pss_check_n_reformat(gl_pss_amf_attribs.pss_cb, ps_format_version);
   }

    /* wait for AMF and native PSS requests */     
    while (1)
    {
        /* re-intialize the FDs and count */ 
        numfds.raise_obj = 0;
        numfds.rmv_obj = 0;
        m_NCS_SEL_OBJ_ZERO(&readfds);
        
        /* add the AMF selection object to FD watch list */ 
        if (gl_pss_amf_attribs.amf_attribs.amfHandle != 0)
        {
            /* Use the following macro to set the saAmfSelectionObject in NCS_SEL_OBJ. */
            m_SET_FD_IN_SEL_OBJ((uns32)gl_pss_amf_attribs.amf_attribs.amfSelectionObject, ncs_amf_sel_obj);
            m_NCS_SEL_OBJ_SET(ncs_amf_sel_obj, &readfds);

            numfds = m_GET_HIGHER_SEL_OBJ(ncs_amf_sel_obj, numfds);
        }
        else /* added this case  as a part of fixing the bug 61101 */
           if((gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj.raise_obj > 0) && 
              (gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj.raise_obj <= 1024) &&
              (gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj.rmv_obj > 0) &&
              (gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj.rmv_obj <= 1024))
        {
           /*  Consider this selection object only if it is valid. */
           m_NCS_SEL_OBJ_SET(gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj, &readfds);
           numfds = m_GET_HIGHER_SEL_OBJ(gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj, numfds);
        }

#if (NCS_PSS_RED == 1)
        /* add the MBCSv Selection Object */
        if (gl_pss_amf_attribs.handles.mbcsv_hdl != 0)
        {
            /* Use the following macro to set the saAmfSelectionObject in NCS_SEL_OBJ. */
            m_SET_FD_IN_SEL_OBJ((uns32)gl_pss_amf_attribs.handles.mbcsv_sel_obj,
               ncs_mbcsv_sel_obj);
            m_NCS_SEL_OBJ_SET(ncs_mbcsv_sel_obj, &readfds);

            numfds = m_GET_HIGHER_SEL_OBJ(ncs_mbcsv_sel_obj, numfds);
        }
#endif
        /* add the traditional MBX fd to the FD watch list */
        mbx_fd = m_NCS_IPC_GET_SEL_OBJ(pss_mbx); 
        m_NCS_SEL_OBJ_SET(mbx_fd, &readfds);
        numfds = m_GET_HIGHER_SEL_OBJ(mbx_fd, numfds);

        /* Modified as a part of fixing the bug 61101 */
        /* if (gl_pss_amf_attribs.amf_attribs.amfHandle == 0)
        {
           m_NCS_SEL_OBJ_SET(gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj, &readfds);
           numfds = m_GET_HIGHER_SEL_OBJ(gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj, numfds);
        } */

        /* wait for the requests indefinately */ 
        count = m_NCS_SEL_OBJ_SELECT(numfds, &readfds, NULL, NULL, NULL);
        if (count > 0)
        {
            m_LOG_PSS_HDLN2_I(NCSFL_SEV_DEBUG, PSS_HDLN_SELECT_RETURN_VAL, count);

            /* AMF componentization is done first. */
            /* added amfhandle check as a part of fixing the bug 61101 */
            if ((gl_pss_amf_attribs.amf_attribs.amfHandle == 0) && 
                (m_NCS_SEL_OBJ_ISSET(gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj, &readfds)))
            {
                saf_status = SA_AIS_OK;
                status = pss_amf_componentize(&gl_pss_amf_attribs.amf_attribs, &saf_status);

                /* We should clear the selection object only for the following case :
                 *    - saf_status is NOT SA_AIS_ERR_TRY_AGAIN
                 */
                if(saf_status != SA_AIS_ERR_TRY_AGAIN)
                {
                   /*  Destroy the selection object if AMF componentization was 
                      successful. In case of RETRY from AMF, go back and listen on the object. */
                   m_NCS_SEL_OBJ_CLR(gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj, &readfds);
                   m_NCS_SEL_OBJ_RMV_IND(gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj, TRUE, TRUE);
                   m_NCS_SEL_OBJ_DESTROY(gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj);
                   gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj.raise_obj = 0;
                   gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj.rmv_obj = 0;
                }
            }

            /* AMF FD is selected */ 
            else if ((gl_pss_amf_attribs.amf_attribs.amfHandle != 0) && 
                     (m_NCS_SEL_OBJ_ISSET(ncs_amf_sel_obj, &readfds)))
            {
                /* PSS CB lock */
                m_PSS_LK_INIT;
                m_PSS_LK(&gl_pss_amf_attribs.pss_cb->lock);
                m_LOG_PSS_LOCK(PSS_LK_LOCKED,&gl_pss_amf_attribs.pss_cb->lock);

                m_LOG_PSS_HEADLINE2(NCSFL_SEV_DEBUG, PSS_HDLN_INVK_AMF_DISPATCH);
                saf_status = saAmfDispatch(gl_pss_amf_attribs.amf_attribs.amfHandle, 
                                        gl_pss_amf_attribs.amf_attribs.dispatchFlags);
                if (saf_status != SA_AIS_OK)
                {
                    m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR, 
                                      PSS_ERR_AMF_DISPATCH_FAILED, 
                                      saf_status);
                    m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN_RETURN_FROM_THREAD_HANDLER);
                    return; 
                }
                m_LOG_PSS_HDLN2_I(NCSFL_SEV_DEBUG, PSS_HDLN_AMF_DISPATCH_RETURN_STATUS, saf_status);

                /* PSS CB unlock */
                m_PSS_UNLK(&gl_pss_amf_attribs.pss_cb->lock);
                m_LOG_PSS_LOCK(PSS_LK_UNLOCKED,&gl_pss_amf_attribs.pss_cb->lock);

                /* clear the bit off for this fd */
                m_NCS_SEL_OBJ_CLR(ncs_amf_sel_obj, &readfds);
            } /* request from AMF is serviced */ 

            /* MBCSv Interface requests */
#if (NCS_PSS_RED == 1)
            else if (FD_ISSET(gl_pss_amf_attribs.handles.mbcsv_sel_obj, &readfds))
            {
                m_LOG_PSS_HEADLINE2(NCSFL_SEV_DEBUG, PSS_HDLN_INVK_MBCSV_DISPATCH);
                mbcsv_arg.i_mbcsv_hdl = gl_pss_amf_attribs.handles.mbcsv_hdl;
                status = ncs_mbcsv_svc(&mbcsv_arg);
                if (status != NCSCC_RC_SUCCESS)
                {
                    m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR, 
                                      PSS_ERR_MBCSV_DISPATCH_FAILED, 
                                      status);
                    m_LOG_PSS_HEADLINE2(NCSFL_SEV_ERROR, PSS_HDLN_RETURN_FROM_THREAD_HANDLER);
                    return;
                }
                m_LOG_PSS_HDLN2_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBCSV_DISPATCH_RETURN_STATUS, status);
            }
#endif

            /* Process the messages on the Mail box if the HA-role of any of PWEs
               is ACTIVE */ 
            else if (m_NCS_SEL_OBJ_ISSET(mbx_fd, &readfds))
            {
/*
                if((gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE) ||
                   (gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_STANDBY) ||
                   (gl_pss_amf_attribs.ha_state == NCS_APP_AMF_HA_STATE_NULL))
*/
                {
                   MAB_MSG *mmsg = NULL;

                   /* PSS CB lock */
                   m_PSS_LK_INIT;
                   m_PSS_LK(&gl_pss_amf_attribs.pss_cb->lock);
                   m_LOG_PSS_LOCK(PSS_LK_LOCKED,&gl_pss_amf_attribs.pss_cb->lock);

                   m_LOG_PSS_HEADLINE2(NCSFL_SEV_DEBUG, PSS_HDLN_INVK_MBX_DISPATCH);
                   /* process native requests of PSSv, only if in Active role */
                   mmsg = m_PSS_RCV_MSG(pss_mbx, mmsg);
                   if(mmsg != NULL)
                   {
                      if((NCS_APP_AMF_HA_STATE_STANDBY == gl_pss_amf_attribs.ha_state) &&
                         (NCSMDS_SVC_ID_OAC == mmsg->fr_svc)&&
                         (MAB_PSS_SVC_MDS_EVT == mmsg->op)&&
                         (NCSMDS_DOWN == mmsg->data.data.pss_mds_svc_evt.change)) 
                      {
                          char addr_str[255] = {0};
                          status = pss_stdby_oaa_down_list_update(mmsg->fr_card,mmsg->yr_hdl,PSS_STDBY_OAA_DOWN_BUFFER_ADD);
                          if(status != NCSCC_RC_SUCCESS)
                          {
                               if (m_NCS_NODE_ID_FROM_MDS_DEST(mmsg->fr_card) == 0)
                                  sprintf(addr_str, "VDEST:%d",m_MDS_GET_VDEST_ID_FROM_MDS_DEST(mmsg->fr_card));
                               else
                                  sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%d",
                                    m_NCS_NODE_ID_FROM_MDS_DEST(mmsg->fr_card),0,(uns32)(mmsg->fr_card));
                               ncs_logmsg(NCS_SERVICE_ID_PSS,  PSS_LID_HDLN_C, PSS_FC_HDLN,
                                   NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE,
                                   NCSFL_TYPE_TIC, PSS_HDLN_STDBY_OAA_DOWN_LIST_ADD_FAILED, addr_str);
                          }
                          else
                          {
                               if (m_NCS_NODE_ID_FROM_MDS_DEST(mmsg->fr_card) == 0)
                                  sprintf(addr_str, "VDEST:%d",m_MDS_GET_VDEST_ID_FROM_MDS_DEST(mmsg->fr_card));
                               else
                                  sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%d",
                                    m_NCS_NODE_ID_FROM_MDS_DEST(mmsg->fr_card),0,(uns32)(mmsg->fr_card));
                               ncs_logmsg(NCS_SERVICE_ID_PSS,  PSS_LID_HDLN_C, PSS_FC_HDLN,
                                   NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE,
                                   NCSFL_TYPE_TIC, PSS_HDLN_STDBY_OAA_DOWN_LIST_ADD_SUCCESS, addr_str);
                          }
                      }
                      else 
                      {
                          status = pss_do_evt(mmsg, TRUE);
                          if (status != NCSCC_RC_SUCCESS)
                          {
                             /* log the error */ 
                             m_LOG_PSS_ERROR_I(NCSFL_SEV_NOTICE, 
                                          PSS_ERR_DO_EVT_FAILED, 
                                          status);
                          }
                          else
                             m_LOG_PSS_HDLN2_I(NCSFL_SEV_DEBUG, PSS_HDLN_MBX_DISPATCH_RETURN_STATUS, status);
                      }   
                   }
                   else
                   {
                      status = NCSCC_RC_FAILURE;
                      m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR, PSS_ERR_MBX_RCV_MSG_FAIL, status);
                   }

                   /* PSS CB unlock */
                   m_PSS_UNLK(&gl_pss_amf_attribs.pss_cb->lock);
                   m_LOG_PSS_LOCK(PSS_LK_UNLOCKED,&gl_pss_amf_attribs.pss_cb->lock);
                }
/*
                else
                {
                   m_LOG_PSS_HDLN2_I(NCSFL_SEV_CRITICAL, PSS_HDLN_MBX_DISPATCH_RETURN_IN_HA_STATE, 
                      gl_pss_amf_attribs.ha_state);
                   m_LOG_PSS_HEADLINE2(NCSFL_SEV_CRITICAL, PSS_HDLN_INVK_MBX_FLUSH_EVTS);
                   pss_flush_mbx_evts(pss_mbx);
                }
*/
                m_NCS_SEL_OBJ_CLR(mbx_fd, &readfds);
            } /* request from Mail Box is serviced */ 
        } /* if (count > 0) */ 
        else
        {
            /* log the error */ 
            m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR, 
                                    PSS_ERR_SELECT_FAILED, count);
        }
    } /* while(1) */

    m_LOG_PSS_HEADLINE2(NCSFL_SEV_CRITICAL, PSS_HDLN_RETURN_FROM_THREAD_HANDLER);
    return;
}

/*****************************************************************************\
 *  Name:          pss_flush_mbx_evts                                         *
 *                                                                            *
 *  Description:   Frees all the events(MAB_MSG) present in the mailbox.      *
 *                                                                            *
 *  Arguments:     SYSF_MBX * Mailbox                                         *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
void pss_flush_mbx_evts(SYSF_MBX*  mbx)
{
  MAB_MSG   *msg = NULL;
  
  while ((msg = m_PSS_RCV_MSG(mbx, msg)) != NULL)
  {

    m_LOG_PSS_HDLN2_I(NCSFL_SEV_CRITICAL, PSS_HDLN_FLUSH_MBX_EVT_TYPE, msg->op);

    pss_free_mab_msg(msg, TRUE);

  }
  return; 
}

/*****************************************************************************\
 *  Name:          pss_amf_componentize                                       * 
 *                                                                            *
 *  Description:   Registers the following with AMF framework                 *
 *                   - AMF callbacks                                          *
 *                   - CSI HA state machine callbacks                         *
 *                   - Healthcheck key                                        *
 *                   - Recovery Policy                                        *
 *                   - Start the health check                                 *
 *                                                                            *
 *  Arguments:     NCS_APP_AMF_ATTRIBS * Amf attributes of PSS                *
 *                 SaAirErrorT        * o_saf_status                          *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
uns32
pss_amf_componentize(NCS_APP_AMF_ATTRIBS *amf_attribs, SaAisErrorT *o_saf_status)
{
    NCS_APP_AMF_HA_STATE_HANDLERS   ha_hdls; 
    SaAmfCallbacksT                 amf_clbks; 
    uns32                           status = NCSCC_RC_FAILURE; 

    if (amf_attribs == NULL)
    {
        m_LOG_PSS_STR(NCSFL_SEV_CRITICAL, "pss_amf_componentize(): received NULL to operate on"); 
        return NCSCC_RC_FAILURE; 
    }

    /* initialize the HA state machine callbacks */ 
    memset(&ha_hdls, 0, 
                    sizeof(NCS_APP_AMF_HA_STATE_HANDLERS)); 
    ha_hdls.invalidTrans = ncs_app_amf_invalid_state_process; 
    ha_hdls.initActive =  pss_amf_INIT_ACTIVE_process; 
    ha_hdls.activeTrans =  pss_amf_ACTIVE_process; 
    ha_hdls.initStandby = pss_amf_INIT_STANDBY_process; 
    ha_hdls.standbyTrans = pss_amf_STANDBY_process; 
    ha_hdls.quiescedTrans = pss_amf_QUIESCED_process; 
    ha_hdls.quiescingTrans = pss_amf_QUIESCING_process; 
    ha_hdls.quiescingToQuiesced = pss_amf_QUIESCING_QUIESCED_process; 

    /* intialize the AMF callbaclks */ 
    memset(&amf_clbks, 0, 
                    sizeof(SaAmfCallbacksT)); 
    amf_clbks.saAmfHealthcheckCallback = pss_amf_health_check; 
    amf_clbks.saAmfComponentTerminateCallback = pss_amf_comp_terminate; 
    amf_clbks.saAmfCSISetCallback = pss_amf_csi_set;
    amf_clbks.saAmfCSIRemoveCallback = pss_amf_csi_remove;

    /* intialize the AMF attributes */ 
    status = ncs_app_amf_attribs_init(amf_attribs, 
                         m_PSS_HELATH_CHECK_KEY, 
                         m_PSS_DISPATCH, 
                         m_PSS_HB_INVOCATION_TYPE, 
                         m_PSS_RECOVERY, 
                         &ha_hdls, 
                         &amf_clbks); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */ 
        m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR, 
                          PSS_ERR_AMF_ATTRIBS_INIT_FAILED, status);
        /* reset the attribs */ 
        memset(amf_attribs, 0, sizeof(NCS_APP_AMF_ATTRIBS)); 
        return status; 
    }
    
    /* set the COMPONENT name */ 
    status = pss_amf_component_name_set(); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* error is logged inside the function */ 
        memset(amf_attribs, 0, sizeof(NCS_APP_AMF_ATTRIBS)); 
        return status; 
    }

   /* initialize the interface with AMF */
   status = pss_amf_initialize(amf_attribs, o_saf_status);

   return status; 
}

/*****************************************************************************\
 *  Name:          pss_amf_health_check                                       * 
 *                                                                            *
 *  Description:   Does a self diagnose on the health of PSS, and reports     *
 *                 healthe to AMF.                                            *
 *                 This function is the callback for Health check of PSS      *
 *                 to the AMF agent.                                          *
 *                                                                            *
 *  Arguments:     SaInvocationT  - for AMF to correlate the response         * 
 *                 const SaNameT *compName - Name of the component (PSS)     *
 *                 SaAmfHealthcheckKeyT *healthcheckKey - Key of PSS          *                                      
 *                                                                            * 
 *  Returns:       SA_AIS_OK to AMF                                           *
 *                 Nothing - to the caller of this function                   *
 *                                                                            *
 *  NOTE:          For the time being PSS says its health is fine to AMF.     *
 *                 Not yet decided on what could be checked.                  * 
\*****************************************************************************/
void
pss_amf_health_check(SaInvocationT invocation,
                     const SaNameT *compName,
                     SaAmfHealthcheckKeyT *healthcheckKey)
{
    SaAisErrorT     saf_status = SA_AIS_OK; 
    PSS_CSI_NODE    *check_me = NULL; 
    PSS_CSI_NODE    *check_my_next = NULL; 

    /*  all the CSIs shall be in the same state  */ 
    check_me = gl_pss_amf_attribs.csi_list; 
    while (check_me)
    {
        check_my_next = check_me->next; 
        if (check_my_next == NULL)
            break; 

        if (check_me->csi_info.ha_state != check_my_next->csi_info.ha_state)
        {
            saf_status = SA_AIS_ERR_FAILED_OPERATION; 
            break; 
        }
        check_me = check_my_next; 
    }

    /* log the health status */ 
    if (saf_status == SA_AIS_OK)
        m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_AMF_HLTH_DOING_AWESOME); 
    else
        m_LOG_PSS_HEADLINE(NCSFL_SEV_CRITICAL, PSS_HDLN_AMF_HLTH_DOING_AWFUL); 
    
    /* send the response to AMF */     
    saAmfResponse(gl_pss_amf_attribs.amf_attribs.amfHandle, 
                  invocation, saf_status); 
    return; 
}

/*****************************************************************************\
 *  Name:          pss_amf_prepare_will                                       * 
 *                                                                            *
 *  Description:   To cleanup the mess that PSS enjoyed during its life time  *
 *                 Now, time to cleanup this...                               *
 *                                                                            *
 *  Arguments:     Nothing                                                    * 
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                          *
 *  NOTE:                                                                     * 
\*****************************************************************************/
uns32
pss_amf_prepare_will(void)
{
    uns32           status; 
    NCSVDA_INFO     vda_info; 
    SaAisErrorT     saf_status = SA_AIS_ERR_FAILED_OPERATION; 

    /* remove all the CSIs */ 
    saf_status = pss_amf_csi_remove_all();  
    if (saf_status == SA_AIS_ERR_FAILED_OPERATION)
    {
        /* log that component terminate failed because of */ 
        m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR, PSS_ERR_AMF_CSI_REMOVE_ALL_FAILED, saf_status); 
        return NCSCC_RC_FAILURE;
    }

    /* destroy the VDEST */ 
    memset(&vda_info, 0, sizeof(NCSVDA_INFO)); 
    vda_info.req = NCSVDA_VDEST_DESTROY; 
    vda_info.info.vdest_destroy.i_create_type = NCSVDA_VDEST_CREATE_SPECIFIC; 
    vda_info.info.vdest_destroy.i_vdest = PSR_VDEST_ID; 
    vda_info.info.vdest_destroy.i_make_vdest_non_persistent = FALSE; 
    status = ncsvda_api(&vda_info); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log that VDEST destroy failed */ 
        m_LOG_PSS_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                             PSS_ERR_VDEST_DESTROY_FAILED, status); 
        return status; 
    }

    return status; 
}

 /****************************************************************************\
 *  Name:          pss_amf_comp_terminate                                     * 
 *                                                                            *
 *  Description:   To terminate the PSS component                             *
 *                                                                            *
 *  Arguments:     SaInvocationT invocation - for AMF to correlate the resp   * 
 *                 const SaNameT *compName  - PSS component name              *
 *                                                                            * 
 *  Returns:       SA_AIS_OK to AMF                                           *
 *                 Nothing - to the caller of this function                   *
 *  NOTE:                                                                     * 
 \****************************************************************************/
void
pss_amf_comp_terminate(SaInvocationT invocation, const SaNameT *compName)
{
    SaAisErrorT     saf_status = SA_AIS_OK; 
    PSS_CB          *inst = NULL;

    /*
    printf("DUMP____________BEFORE_____________TERMINATE...........\n");
    printf("=======================================================\n");
    ncs_mem_whatsout_dump(); */

    inst = gl_pss_amf_attribs.pss_cb;

    /* Destroy the "hdl" before proceeding
       with the SVC instance destroy procedure. */
    ncshm_destroy_hdl(NCS_SERVICE_ID_PSS, inst->hm_hdl);

    /* Unregister PSS-MIB with OAA */
    (void)pss_unregister_with_oac(inst);

    (void)pss_amf_prepare_will();

    pss_clean_inst_cb(inst);

    /* gl_pss_mem_leak_debug = TRUE; */

    /* finalize the interface with DTSv */ 
    (void)pss_dtsv_unbind();
   
    /* time to send our response to AMF */ 
    saAmfResponse(gl_pss_amf_attribs.amf_attribs.amfHandle, invocation, saf_status); 

    saf_status = saAmfFinalize(gl_pss_amf_attribs.amf_attribs.amfHandle);

#if (NCS_PSS_RED == 1)
    pss_mbcsv_deregister(inst);
#endif

    /* if (saf_status == SA_AIS_OK) */
    m_PSS_LK_DLT(&inst->lock);
    m_MMGR_FREE_PSS_CB(inst);


    /* Cleanup the mailbox */  
    (void)m_NCS_IPC_DETACH(&gl_pss_amf_attribs.mbx_details.pss_mbx, NULL, NULL);
    m_NCS_IPC_RELEASE(&gl_pss_amf_attribs.mbx_details.pss_mbx, NULL);

    /*
    printf("DUMP____________AFTER_____________TERMINATE...........\n");
    printf("=======================================================\n");
    ncs_mem_whatsout_dump();
    fflush(stdout);
    */

    /* what are you waiting for? Jump into the well */ 
    exit(0); /* I am not sure of the effect of the parameter */ 
}

 /****************************************************************************\
 *  Name:          pss_amf_csi_set                                            * 
 *                                                                            *
 *  Description:   Callback for the CSI assignment by AMF                     *
 *                                                                            *
 *  Arguments:     SaInvocationT invocation - for AMF to correlate resp       * 
 *                 const SaNameT *compName  - PSS component                   *
 *                 SaAmfHAStateT haState    - New HA state to be assigned     *   
 *                 SaAmfCSIDescriptorT csiDescriptor - Details about the      * 
 *                                                     workload being assigned*
 *                                                                            * 
 *  Returns:       SA_AIS_OK to AMF                                           *
 *                 Nothing - to the caller of this function                   *
 *                                                                            *
 *  NOTE:                                                                     * 
 \****************************************************************************/
void 
pss_amf_csi_set(SaInvocationT invocation,
                const SaNameT *compName,
                SaAmfHAStateT haState,  
                SaAmfCSIDescriptorT csiDescriptor)
{
    uns8 tCompName[SA_MAX_NAME_LENGTH+1] = {0}; 
    uns8 tCsiName[SA_MAX_NAME_LENGTH+1] = {0}; 

    /* log the information about the received CSI assignment */ 
    memcpy(tCompName, compName->value, compName->length); 
    memcpy(tCsiName, csiDescriptor.csiName.value, csiDescriptor.csiName.length);
    m_LOG_PSS_CSI_DETAILS(NCSFL_SEV_NOTICE, 
                          PSS_HDLN_AMF_CSI_DETAILS, 
                          csiDescriptor.csiFlags, 
                          tCompName, tCsiName, haState); 

    switch (csiDescriptor.csiFlags)
    {
        case SA_AMF_CSI_ADD_ONE: 
        pss_amf_csi_new(invocation, compName, haState, csiDescriptor); 
        break; 

        case SA_AMF_CSI_TARGET_ONE: 
        pss_amf_csi_set_one(invocation, compName, haState, csiDescriptor); 
        break; 

        case SA_AMF_CSI_TARGET_ALL: 
        pss_amf_csi_set_all(invocation, compName, haState, csiDescriptor); 
        break; 

        default: 
        /* log the error code */ 
        m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR, PSS_ERR_AMF_CSI_FLAGS_ILLEGAL, 
                          csiDescriptor.csiFlags); 
        saAmfResponse(gl_pss_amf_attribs.amf_attribs.amfHandle, invocation, 
                      SA_AIS_ERR_FAILED_OPERATION); 
        break; 
    }
    return; 
}

/*****************************************************************************\
 *  Name:          pss_amf_csi_new                                            * 
 *                                                                            *
 *  Description:   Creates a new CSI either in ACTIVE or STANDBY state        *
 *                                                                            *
 *  Arguments:     SaInvocationT invocation - for AMF to correlate resp       * 
 *                 const SaNameT *compName  - PSS component                   *
 *                 SaAmfHAStateT haState    - Initial HA state to be assigned *   
 *                 SaAmfCSIDescriptorT csiDescriptor - Details about the      * 
 *                                                     workload being assigned*
 *                                                                            * 
 *  Returns:       SA_AIS_OK to AMF                                           *
 *                 Nothing - to the caller of this function                   *
 *  NOTE:                                                                     * 
\*****************************************************************************/
void 
pss_amf_csi_new(SaInvocationT invocation,
                const SaNameT *compName,
                SaAmfHAStateT haState,  
                SaAmfCSIDescriptorT csiDescriptor)
{
    uns32           status; 
    PW_ENV_ID       env_id; 
    PSS_CSI_NODE    *csi = NULL;  
    SaAisErrorT     saf_status = SA_AIS_ERR_FAILED_OPERATION; 
    
    /* validate the requested state transition */ 
    if ((haState == SA_AMF_HA_QUIESCED) || (haState == SA_AMF_HA_QUIESCING))
    {
        /* log the invalid state transition */ 
        goto enough; 
    }

    /* decode the attributes to get the Environment ID */ 
    env_id = pss_amf_csi_attrib_decode(csiDescriptor.csiAttr); 
    if (env_id ==0)
    {
        /* log that invalid attribute is rereceived from AMF */ 
        m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR, PSS_ERR_AMF_ENVID_ILLEGAL, env_id); 
        goto enough; 
    }

    /* make sure that there is no CSI for this Env-id created already */ 
    csi = pss_amf_csi_list_find_envid(env_id); 
    if ((env_id != m_PSS_DEFAULT_ENV_ID)&&(csi != NULL)) /* PSS is already installed in this */
    {
        /* log that this environment is already available with PSS */ 
        goto enough; 
    }

    if ((env_id == m_PSS_DEFAULT_ENV_ID)&&(csi != NULL))
    {
        /* default one is created already */ 

        /* is the AMF given state is different from HAPS state? */ 
        if (csi->csi_info.ha_state != haState)
        {
            /* log that AMF asked state is different from HAPS given initial state */ 
            /* ask for a role change, nothing else */
            status = pss_amf_csi_vdest_role_change(&csi->csi_info, haState);
            if (status != NCSCC_RC_SUCCESS)
                goto enough; 
        }
        /* Update the global CB state also */
        csi->pwe_cb->p_pss_cb->ha_state = haState;
        gl_pss_amf_attribs.ha_state = haState;

        /* update the CSI Name */ 
        csi->csi_info.work_desc.length = csiDescriptor.csiName.length; 
        memset(&csi->csi_info.work_desc.value, 0, SA_MAX_NAME_LENGTH);
        memcpy(&csi->csi_info.work_desc.value, 
                        &csiDescriptor.csiName.value, 
                        csi->csi_info.work_desc.length); 
        saf_status = SA_AIS_OK; 
        goto enough; 
    }

    /* call into the state machine */ 
    status =  gl_pss_amf_attribs.amf_attribs.ncs_app_amf_hastate_dispatch
                [NCS_APP_AMF_HA_STATE_NULL][haState](NULL, env_id, csiDescriptor); 
    if (status == NCSCC_RC_SUCCESS)
    {
        csi = pss_amf_csi_list_find_envid(env_id); 

        /* Update the global CB state also */
        csi->pwe_cb->p_pss_cb->ha_state = haState;
        gl_pss_amf_attribs.ha_state = haState;

        /* log the environment ID for which CSI creation success */ 
        m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE, PSS_HDLN_AMF_CSI_ADD_ONE_SUCCESS, env_id);

        /* log the state change */ 
        m_LOG_PSS_STATE_CHG(NCSFL_SEV_NOTICE, 
                            PSS_HDLN_AMF_CSI_STATE_CHG_SUCCESS, 
                            NCS_APP_AMF_HA_STATE_NULL, haState); 
        saf_status = SA_AIS_OK; 
    }
    else
    {
        saf_status = SA_AIS_ERR_FAILED_OPERATION; 

        /* log the environment ID for which CSI creation failed */ 
        m_LOG_PSS_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_CRITICAL, 
                           PSS_ERR_CSI_ADD_ONE_FAILED, 
                           status, env_id); 

        /* log the state change failure */ 
        m_LOG_PSS_STATE_CHG(NCSFL_SEV_ERROR, 
                            PSS_HDLN_AMF_CSI_STATE_CHG_FAILED, 
                            NCS_APP_AMF_HA_STATE_NULL, haState); 
    }

enough: 
    saAmfResponse(gl_pss_amf_attribs.amf_attribs.amfHandle, invocation, saf_status); 

    return; 
}

 /****************************************************************************\
 *  Name:          pss_amf_csi_create_and_install_env                         * 
 *                                                                            *
 *  Description:   Creates a new CSI either in ACTIVE or STANDBY state        *
 *                 and updates the CSI name into PSS data structures.         *
 *                                                                            *
 *  Arguments:     PW_ENV_ID - New Environment to be created/installed in PSS.*
 *                 SaAmfHAStateT - State of this CSI                          *
 *                 SaAmfCSIDescriptorT - CSI Name and other details           * 
 *                                                                            * 
 *  Returns:       NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                          *
 *  NOTE:                                                                     * 
 \****************************************************************************/
uns32
pss_amf_csi_create_and_install_env(PW_ENV_ID            envid, 
                                   SaAmfHAStateT        haState,
                                   SaAmfCSIDescriptorT  csiDescriptor)
{
    PSS_CSI_NODE    *csi = NULL; 

    /* install the new CSI */ 
    csi = pss_amf_csi_install(envid, haState); 
    if (csi == NULL)
    {
        /* log that unable to install the environment */ 
        return NCSCC_RC_FAILURE; 
    }

    /* add to the list of CSIs */ 
    pss_amf_csi_list_add_front(csi); 

    /* update the CSI Name filed */ 
    csi->csi_info.work_desc.length = csiDescriptor.csiName.length; 
    memset(&csi->csi_info.work_desc.value, 0, SA_MAX_NAME_LENGTH);
    memcpy(&csi->csi_info.work_desc.value, 
                    &csiDescriptor.csiName.value, 
                    csi->csi_info.work_desc.length); 

    /* everything went well */ 
    return NCSCC_RC_SUCCESS;
}

 /****************************************************************************\
 *  Name:          pss_amf_csi_set_all                                        * 
 *                                                                            *
 *  Description:   to set the assined state to all the CSIs in PSS            *
 *                                                                            *
 *  Arguments:     SaInvocationT invocation - for AMF to correlate resp       * 
 *                 const SaNameT *compName  - PSS component                   *
 *                 SaAmfHAStateT haState    - New HA state to be assigned     *   
 *                 SaAmfCSIDescriptorT csiDescriptor - Details about the      * 
 *                                                     workload being assigned*
 *                                                                            * 
 *  Returns:       SA_AIS_OK to AMF                                           *
 *                 Nothing - to the caller of this function                   *
 *  NOTE:                                                                     * 
 \****************************************************************************/
void 
pss_amf_csi_set_all(SaInvocationT invocation,
                const SaNameT *compName,
                SaAmfHAStateT haState,  
                SaAmfCSIDescriptorT csiDescriptor)
{
    uns32           status = NCSCC_RC_FAILURE; 
    SaAisErrorT     saf_status = SA_AIS_OK;
    PSS_CSI_NODE    *csi_node = NULL; 
    SaAmfHAStateT   prevState = SA_AMF_HA_STANDBY; /* Setting default value to remove warnings. */
    
    csi_node = gl_pss_amf_attribs.csi_list; 

    while (csi_node) /* for all the CSIs in PSS */ 
    {
        prevState = csi_node->csi_info.ha_state; 
        csi_node->pwe_cb->amf_invocation_id = invocation;
        /* perform the asked state change */ 
        status =  gl_pss_amf_attribs.amf_attribs.ncs_app_amf_hastate_dispatch[csi_node->csi_info.ha_state][haState]((void*)csi_node, 
                           csi_node->csi_info.env_id, csiDescriptor); 
        /* update the state */ 
        if (status == NCSCC_RC_SUCCESS)
        {
            csi_node->csi_info.ha_state = haState; 
            /* Update the global CB state also */
            csi_node->pwe_cb->p_pss_cb->ha_state = haState;
            gl_pss_amf_attribs.ha_state = haState;
            if((haState == SA_AMF_HA_QUIESCING) || (haState == SA_AMF_HA_QUIESCED))
            {
               csi_node->pwe_cb->amf_invocation_id = invocation;
            }
        }
        else
        {
            saf_status = SA_AIS_ERR_FAILED_OPERATION; 
            break; 
        }
        /* go to the next CSI */
        csi_node = csi_node->next; 
    }

    if (saf_status == SA_AIS_OK)
    {
        /* log the state change event */ 
        m_LOG_PSS_STATE_CHG(NCSFL_SEV_NOTICE, 
                            PSS_HDLN_AMF_CSI_STATE_CHG_SUCCESS, 
                            prevState, haState); 
    }
    else
    {
        /* log the state change failure */
        m_LOG_PSS_STATE_CHG(NCSFL_SEV_ERROR, 
                            PSS_HDLN_AMF_CSI_STATE_CHG_FAILED, 
                            prevState, haState); 
    }

    /* if the state transition is to QUISCING */ 
    if (haState == SA_AMF_HA_QUIESCING)
    {
        /* log the state change */
        /* saAmfCSIQuiescingComplete(gl_pss_amf_attribs.amf_attribs.amfHandle, invocation, saf_status); 
           This is invoked when PSS thread handles the VDEST-role-change callback event. */
        return; 
    }

    /* send your response to the Big Boss AMF */     
    if(haState != SA_AMF_HA_QUIESCED)
    {
       saAmfResponse(gl_pss_amf_attribs.amf_attribs.amfHandle, 
                  invocation, saf_status); 
    }

 /* 3.0.a addition: Reformat if haState changed from STANDBY to ACTIVE */
    if(saf_status == SA_AIS_OK && prevState == SA_AMF_HA_STANDBY && haState == SA_AMF_HA_ACTIVE)
    {
       int            ps_cnt=0;
       struct dirent  **ps_list;
       int            ps_format_version = 0, tmp_ps0 = 0, tmp_ps1 = 0;
       NCS_OS_FILE file;
       uns32      retval;
       uns8       buf[NCS_PSSTS_MAX_PATH_LEN];

    /* Get the existing persistent-store's format versions.
       -> There could two formats of persistent stores. One PSS_PS_FORMAT_VERSION and
          the other from where to reformat.
       -> If there is only one format version that is PSS_PS_FORMAT_VERSION, then
          formatting need to be done on this version. [This is required
             as MIB reformatting need to be done for every switch-over].
        */

       memset(&buf, '\0', sizeof(buf));
       sprintf(buf, "%s%d", NCS_PSS_DEF_PSSV_ROOT_PATH, PSS_PS_FORMAT_VERSION);

       /* Now create the directory */
       memset(&file, '\0', sizeof(NCS_OS_FILE));
       file.info.create_dir.i_dir_name = buf;

       retval = m_NCS_FILE_OP (&file, NCS_OS_FILE_CREATE_DIR);
       if (retval != NCSCC_RC_SUCCESS)
           return;

       printf ("Persistent store with format %d created successfully", PSS_PS_FORMAT_VERSION);

       /* Using snprintf directly, because the leap routine does not accept variable arguments,
                                                      which leads to multiple function calls */
       ps_cnt = scandir(NCS_PSS_DEF_PSSV_ROOT_PATH, &ps_list, &persistent_file_filter, NULL);
       if (ps_cnt < 0)
       {
           perror("scandir");
           return;
       }
       if(ps_cnt > 2)
       {
          /* Log that there are multiple formats of persistent store */
          m_LOG_PSS_STR(NCSFL_SEV_CRITICAL,
                        "pss_check_n_reformat(): Too many formats of persistent store found");
       }
       else if(ps_cnt != 0)
       {
          if(ps_cnt >= 1)
             tmp_ps0 = atoi(ps_list[0]->d_name);
          if(ps_cnt == 2)
             tmp_ps1 = atoi(ps_list[1]->d_name);
          if(tmp_ps0 == PSS_PS_FORMAT_VERSION)
          {
             if(ps_cnt == 1)
                ps_format_version = tmp_ps0;
             else
                ps_format_version = tmp_ps1;
          }
          else if(tmp_ps1 == PSS_PS_FORMAT_VERSION)
          {
             ps_format_version = tmp_ps0;
          }
          else
          {
             /* if none of the formats returned by scandir is PSS_PS_FORMAT_VERSION, return FAILURE */
             m_LOG_PSS_STR(NCSFL_SEV_CRITICAL, "pss_check_n_reformat(): Invalid formats of persistent store found");
          }
       }
       for(;ps_cnt > 0; ps_cnt--)
          free(ps_list[ps_cnt-1]);
       free(ps_list);

       if(ps_format_version != 0)
  {
     if(NCSCC_RC_SUCCESS != pss_check_n_reformat(gl_pss_amf_attribs.pss_cb, ps_format_version))
     {
        saAmfComponentErrorReport(gl_pss_amf_attribs.amf_attribs.amfHandle,
                                  &gl_pss_amf_attribs.amf_attribs.compName,
                                  0, /* errorDetectionTime; AvNd will provide this value */
                                  SA_AMF_COMPONENT_FAILOVER/* recovery */, 0/*NTF Id*/);
     }
  }
    }
 /* End of 3.0.a addition */
    return; 
}

 /****************************************************************************\
 *  Name:          pss_amf_csi_set_one                                        * 
 *                                                                            *
 *  Description:   To change the CSI state of a particular CSI                *
 *                                                                            *
 *  Arguments:     SaInvocationT invocation - for AMF to correlate resp       * 
 *                 const SaNameT *compName  - PSS component                   *
 *                 SaAmfHAStateT haState    - New HA state to be assigned     *   
 *                 SaAmfCSIDescriptorT csiDescriptor - Details about the      * 
 *                                                     workload being assigned*
 *                                                                            * 
 *  Returns:       SA_AIS_OK to AMF                                           *
 *                 Nothing - to the caller of this function                   *
 *  NOTE:                                                                     * 
 \****************************************************************************/
void 
pss_amf_csi_set_one(SaInvocationT invocation,
                const SaNameT *compName,
                SaAmfHAStateT haState,  
                SaAmfCSIDescriptorT csiDescriptor)
{
    uns32           status = NCSCC_RC_FAILURE; 
    SaAisErrorT     saf_status = SA_AIS_ERR_FAILED_OPERATION;
    PSS_CSI_NODE    *csi;

    /* make sure that there is CSI for the state change */ 
    csi= pss_amf_csi_list_find_csiname(&csiDescriptor.csiName); 
    if (csi == NULL)
    {
        /* by this time env must be initialized for this */ 
        /* log the error */ 
        saAmfResponse(gl_pss_amf_attribs.amf_attribs.amfHandle, invocation, saf_status); 
        return; 
    }

    /* transit to the requested state */ 
    status =  gl_pss_amf_attribs.amf_attribs.ncs_app_amf_hastate_dispatch[csi->csi_info.ha_state][haState]((void*)csi, 
                      csi->csi_info.env_id, csiDescriptor); 
    if (status == NCSCC_RC_SUCCESS)
    {
        /* Update the global CB state also */
        csi->pwe_cb->p_pss_cb->ha_state = haState;
        gl_pss_amf_attribs.ha_state = haState;

        if((haState == SA_AMF_HA_QUIESCING) || (haState == SA_AMF_HA_QUIESCED))
        {
           csi->pwe_cb->amf_invocation_id = invocation;
        }
        /* log the state change */ 
        m_LOG_PSS_STATE_CHG(NCSFL_SEV_NOTICE, PSS_HDLN_AMF_CSI_STATE_CHG_SUCCESS, 
                             csi->csi_info.ha_state, haState); 
        
        /* update the ha state in our data structures */ 
        csi->csi_info.ha_state = haState; 

        /* set what we want to communicate to AMF */ 
        saf_status = SA_AIS_OK; 
    }
    else
    {
        /* log the state transition failure */ 
        m_LOG_PSS_STATE_CHG(NCSFL_SEV_ERROR, PSS_HDLN_AMF_CSI_STATE_CHG_FAILED, 
                           csi->csi_info.ha_state, haState); 
    }

    /* if the state transition is to QUISCING */ 
    if (haState == SA_AMF_HA_QUIESCING)
    {
        /* saAmfCSIQuiescingComplete(gl_pss_amf_attribs.amf_attribs.amfHandle, invocation, saf_status); */
        /* This would be invoked in the MDS callback */
        return; 
    }
    
    /* time to report back */     
    if(haState != SA_AMF_HA_QUIESCED)
    {
       saAmfResponse(gl_pss_amf_attribs.amf_attribs.amfHandle, 
                  invocation, saf_status); 
    }

   return; 
}

 /****************************************************************************\
 *  Name:          pss_amf_csi_remove                                         * 
 *                                                                            *
 *  Description:   Callback to remove a particular CSI                        *
 *                                                                            *
 *  Arguments:     SaInvocationT invocation - For AMF to correlate the resp   * 
 *                 const SaNameT *compName  - PSS component as described BAM  *
 *                 const SaNameT *csiName   - Name of the CSI                 * 
 *                 SaAmfCSIFlagsT csiFlags  - Should all of them to be removed*
 *                                                                            * 
 *  Returns:       SA_AIS_OK - to the AMF                                     *
 *                 Nothing to the caller                                      *
 *  NOTE:                                                                     * 
\*****************************************************************************/
void
pss_amf_csi_remove(SaInvocationT invocation,
                const SaNameT *compName,
                const SaNameT *csiName,
                SaAmfCSIFlagsT csiFlags)
{
    SaAisErrorT     saf_status = SA_AIS_ERR_FAILED_OPERATION; 
    PSS_CSI_NODE    *csi_node = NULL; 
    
    /* based on the flags, see whether all the CSI have to be removed, or 
     * a specific one
     */ 
    switch(csiFlags)
    {
        /* remove a particular CSI */ 
        case SA_AMF_CSI_TARGET_ONE: 
        /* find the CSI details in the global CSI list by CSI Name */ 
        /* log the CSI name to be removed */ 
        csi_node = pss_amf_csi_list_find_csiname(csiName); 
        if (csi_node != NULL)
        {
            saf_status = pss_amf_csi_remove_one(csi_node); 
        }
        break; 
        
        /* remove all the CSIs in PSS */ 
        case SA_AMF_CSI_TARGET_ALL: 
        /* log the event */ 
        saf_status = pss_amf_csi_remove_all(); 
        break; 

        /* all the other cases are junk */ 
        case SA_AMF_CSI_ADD_ONE:
        default:
        /* log the event */ 
        break; 
    }/* end of switch() */ 
    
    /* log the response being sent to AMF */ 

    /* send the response */ 
    saAmfResponse(gl_pss_amf_attribs.amf_attribs.amfHandle, 
                  invocation, saf_status); 

    return; 
}

/*****************************************************************************\
 *  Name:          pss_amf_INIT_ACTIVE_proces                                 * 
 *                                                                            *
 *  Description:   To create a new CSI instance in PSS                        *
 *                                                                            *
 *  Arguments:     void            *data         - PSS, list of CBs           *
 *                 NCS_APP_AMF_ATTRIBS *attribs  - AMF attributes             *
 *                 SaAmfCSIDescriptorT csiDescriptor - Details of the CSI     * 
 *                 SaInvocationT       invocation    - For AMF reposnse       *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* create a new CSI Instance, and put it in ACTIVE State */ 
uns32
pss_amf_INIT_ACTIVE_process(void                *data, 
                            PW_ENV_ID           envid,
                            SaAmfCSIDescriptorT csiDescriptor)
{
    return pss_amf_csi_create_and_install_env(envid, SA_AMF_HA_ACTIVE, 
                                              csiDescriptor);
}

/*****************************************************************************\
 *  Name:          pss_amf_ACTIVE_proces                                      * 
 *                                                                            *
 *  Description:   To put a CSI in ACTIVE state                               *
 *                                                                            *
 *  Arguments:     void            *data         - PSS, list of CBs           *
 *                 NCS_APP_AMF_ATTRIBS *attribs  - AMF attributes             *
 *                 SaAmfCSIDescriptorT csiDescriptor - Details of the CSI     * 
 *                 SaInvocationT       invocation    - For AMF reposnse       *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* Go to ACTIVE State */ 
uns32 
pss_amf_ACTIVE_process(void                *data, 
                       PW_ENV_ID           envid,
                       SaAmfCSIDescriptorT csiDescriptor)
{
   uns32 retval = NCSCC_RC_SUCCESS;
   PSS_CSI_NODE    *csi_node = (PSS_CSI_NODE*)data; 
   MAB_MSG    *mm = NULL;

   /* if the state transition is from Active to Standby,
    * make sure that, Standby is in complete sync with Active
    */
   if (csi_node->csi_info.ha_state == NCS_APP_AMF_HA_STATE_STANDBY)
   {
      /* cold sync is not yet complete */
   }
   
   /* (clean up stale pointers before becoming from standby to active)   */
   if(csi_node->pwe_cb->pss_stdby_oaa_down_buffer != NULL)
   {
       PSS_STDBY_OAA_DOWN_BUFFER_NODE *buffer_node = csi_node->pwe_cb->pss_stdby_oaa_down_buffer;
       PSS_STDBY_OAA_DOWN_BUFFER_NODE *free_node = NULL;
       m_LOG_PSS_STR(NCSFL_SEV_NOTICE, "pss_amf_ACTIVE_process:Cleaning up the stale pointers before change role to active");
       while(buffer_node)
       {
           char addr_str[255] = {0};
           if(pss_handle_oaa_down_event(csi_node->pwe_cb, &buffer_node->oaa_addr) != NCSCC_RC_SUCCESS)
           {
               if (m_NCS_NODE_ID_FROM_MDS_DEST(buffer_node->oaa_addr) == 0)
                   sprintf(addr_str, "VDEST:%d",m_MDS_GET_VDEST_ID_FROM_MDS_DEST(buffer_node->oaa_addr));
               else
                   sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%d",
                                m_NCS_NODE_ID_FROM_MDS_DEST(buffer_node->oaa_addr), 0, (uns32)(buffer_node->oaa_addr));
               ncs_logmsg(NCS_SERVICE_ID_PSS,  PSS_LID_HDLN_C, PSS_FC_HDLN,
                       NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE,
                       NCSFL_TYPE_TIC, PSS_HDLN_OAA_DOWN_EVENT_FAILED, addr_str);
           }
           else
           {
               if (m_NCS_NODE_ID_FROM_MDS_DEST(buffer_node->oaa_addr) == 0)
                   sprintf(addr_str, "VDEST:%d",m_MDS_GET_VDEST_ID_FROM_MDS_DEST(buffer_node->oaa_addr));
               else
                   sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%d",
                                m_NCS_NODE_ID_FROM_MDS_DEST(buffer_node->oaa_addr), 0, (uns32)(buffer_node->oaa_addr));
               ncs_logmsg(NCS_SERVICE_ID_PSS,  PSS_LID_HDLN_C, PSS_FC_HDLN,
                       NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE,
                       NCSFL_TYPE_TIC, PSS_HDLN_OAA_DOWN_EVENT_SUCCESS, addr_str);
           }
           free_node = buffer_node;
           buffer_node = buffer_node->next;
           m_MMGR_FREE_STDBY_PSS_BUFFER_NODE(free_node);
       }
       csi_node->pwe_cb->pss_stdby_oaa_down_buffer = NULL;
   }

   /* do the MDS change role */ 
   retval = pss_amf_csi_vdest_role_change(&csi_node->csi_info, SA_AMF_HA_ACTIVE); 
   if(retval != NCSCC_RC_SUCCESS)
      return retval;

   retval = pss_set_ckpt_role(csi_node->pwe_cb, SA_AMF_HA_ACTIVE);
   if(retval != NCSCC_RC_SUCCESS)
      return retval;

   /* This is a preventive step */
   csi_node->csi_info.ha_state = NCS_APP_AMF_HA_STATE_ACTIVE;
   gl_pss_amf_attribs.ha_state = NCS_APP_AMF_HA_STATE_ACTIVE;
   gl_pss_amf_attribs.csi_list->pwe_cb->p_pss_cb->ha_state = NCS_APP_AMF_HA_STATE_ACTIVE;

#if (NCS_PSS_RED == 1)
   /* Post a message to PSS mailbox */
   mm = m_MMGR_ALLOC_MAB_MSG;
   if(mm == NULL)
   {
      return NCSCC_RC_FAILURE;
   }

   memset(mm, '\0', sizeof(MAB_MSG));
   /*  Now we will be passing PWE handle rather than its pointer */
   mm->yr_hdl = NCS_INT32_TO_PTR_CAST(csi_node->pwe_cb->hm_hdl);
   mm->op = MAB_PSS_RE_RESUME_ACTIVE_ROLE_FUNCTIONALITY;

   if(m_PSS_SND_MSG(csi_node->pwe_cb->mbx, mm) == NCSCC_RC_FAILURE)
   {
      m_MMGR_FREE_MAB_MSG(mm);
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }
#endif

   return retval;
}

/*****************************************************************************\
 *  Name:          pss_amf_INIT_STANDBY_proces                                * 
 *                                                                            *
 *  Description:   To create a CSI in STANDBY state                           *
 *                                                                            *
 *  Arguments:     void            *data         - PSS, list of CBs           *
 *                 NCS_APP_AMF_ATTRIBS *attribs  - AMF attributes             *
 *                 SaAmfCSIDescriptorT csiDescriptor - Details of the CSI     * 
 *                 SaInvocationT       invocation    - For AMF reposnse       *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
uns32
pss_amf_INIT_STANDBY_process(void               *data, 
                            PW_ENV_ID           envid,
                            SaAmfCSIDescriptorT csiDescriptor)
{
    return pss_amf_csi_create_and_install_env(envid, SA_AMF_HA_STANDBY, 
                                              csiDescriptor);
}

/*****************************************************************************\
 *  Name:          pss_amf_STANDBY_process                                    * 
 *                                                                            *
 *  Description:   To pust a CSI in STANDBY state                             *
 *                                                                            *
 *  Arguments:     void            *data         - PSS, list of CBs           *
 *                 NCS_APP_AMF_ATTRIBS *attribs  - AMF attributes             *
 *                 SaAmfCSIDescriptorT csiDescriptor - Details of the CSI     * 
 *                 SaInvocationT       invocation    - For AMF reposnse       *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* Go to STANDBY State */
uns32 
pss_amf_STANDBY_process(void                *data, 
                        PW_ENV_ID           envid,
                        SaAmfCSIDescriptorT csiDescriptor)
{
    PSS_CSI_NODE    *csi_node = (PSS_CSI_NODE*)data; 
    uns32           retval = NCSCC_RC_SUCCESS;

    /* do the MDS change role */ 
    retval = pss_amf_csi_vdest_role_change(&csi_node->csi_info, SA_AMF_HA_STANDBY); 
    if(retval != NCSCC_RC_SUCCESS)
       return retval;

    retval = pss_set_ckpt_role(csi_node->pwe_cb, SA_AMF_HA_STANDBY);

    return retval;
}

/*****************************************************************************\
 *  Name:          pss_amf_QUIESCED_process                                   * 
 *                                                                            *
 *  Description:   To put a CSI in QUIESCED state                             *
 *                                                                            *
 *  Arguments:     void            *data         - PSS, list of CBs           *
 *                 NCS_APP_AMF_ATTRIBS *attribs  - AMF attributes             *
 *                 SaAmfCSIDescriptorT csiDescriptor - Details of the CSI     * 
 *                 SaInvocationT       invocation    - For AMF reposnse       *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* Go to QUIESCED State */
uns32 
pss_amf_QUIESCED_process(void                *data, 
                         PW_ENV_ID           envid,
                         SaAmfCSIDescriptorT csiDescriptor)
{
    PSS_CSI_NODE    *csi_node = (PSS_CSI_NODE*)data; 
    uns32 retval = NCSCC_RC_SUCCESS;

    /* no data has been identified to do the checkpoint at this point of time */

    /* Invoke MDS API to change role of VDEST to QUIESCED */
    retval = pss_amf_csi_vdest_role_change(&csi_node->csi_info, SA_AMF_HA_QUIESCED); 
    if(retval != NCSCC_RC_SUCCESS)
       return retval;

    /* When the VDEST role is changed, the MDS callback function will post a 
       message to the mailbox. Till that time, keep processing the Mailbox events normally. */

    return retval; 
}

/*****************************************************************************\
 *  Name:          pss_amf_QUIESCING_process                                  * 
 *                                                                            *
 *  Description:   To put a CSI in QUIESCING state                            *
 *                                                                            *
 *  Arguments:     void            *data         - PSS, list of CBs           *
 *                 NCS_APP_AMF_ATTRIBS *attribs  - AMF attributes             *
 *                 SaAmfCSIDescriptorT csiDescriptor - Details of the CSI     * 
 *                 SaInvocationT       invocation    - For AMF reposnse       *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* Go to QUIESCING State */
uns32 
pss_amf_QUIESCING_process(void                *data, 
                          PW_ENV_ID           envid,
                          SaAmfCSIDescriptorT csiDescriptor)
{
    PSS_CSI_NODE    *csi_node = (PSS_CSI_NODE*)data; 
    uns32 retval = NCSCC_RC_SUCCESS;

    /* no data has been identified to do the checkpoint at this point of time */

    /* Invoke MDS API to change role of VDEST to QUIESCED */
    retval = pss_amf_csi_vdest_role_change(&csi_node->csi_info, SA_AMF_HA_QUIESCED); 
    if(retval != NCSCC_RC_SUCCESS)
       return retval;

    /* When the VDEST role is changed, the MDS callback function will post a 
       message to the mailbox. Till that time, keep processing the Mailbox events normally. */

   return retval; 
}

/*****************************************************************************\
 *  Name:          pss_amf_QUIESCING_QUIESCED_process                         * 
 *                                                                            *
 *  Description:   To put a CSI in QUIESCING state                            *
 *                                                                            *
 *  Arguments:     void            *data         - PSS, list of CBs           *
 *                 NCS_APP_AMF_ATTRIBS *attribs  - AMF attributes             *
 *                 SaAmfCSIDescriptorT csiDescriptor - Details of the CSI     * 
 *                 SaInvocationT       invocation    - For AMF reposnse       *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* Go to QUIESCED from QUIESCING State */
uns32 
pss_amf_QUIESCING_QUIESCED_process(void                *data, 
                                   PW_ENV_ID           envid,
                                   SaAmfCSIDescriptorT csiDescriptor)
{
    /* nothing to do */ 
    return NCSCC_RC_SUCCESS; 
}

/*****************************************************************************\
 *  Name:          pss_amf_csi_remove_one                                     * 
 *                                                                            *
 *  Description:   Removes the CSI from the global CSI list                   *
 *                 - Uninstall the Environment                                *
 *                 - Destroy the Environment                                  *
 *                 - Destroy the PSS_CB information of this Environment      * 
 *                 - Delete this CSI from the global CSI list                 * 
 *                 - free the CSI node                                        *  
 *                                                                            *
 *  Arguments:     PSS_CSI_NODE * pointer to the CSI to be deleted            * 
 *                                                                            * 
 *  Returns:       SA_AIS_OK/SA_AIS_ERR_FAILED_OPERATION                      *
 *  NOTE:                                                                     * 
\*****************************************************************************/
SaAisErrorT 
pss_amf_csi_remove_one(const PSS_CSI_NODE *csi_node)
{
    NCSMDS_INFO     info;
    uns32           status;
    PSS_CSI_NODE    *del_me = NULL; 
    SaAisErrorT     saf_status = SA_AIS_ERR_FAILED_OPERATION; 
    PW_ENV_ID       env_id = 0; 
    
    /* uninstall the Env */ 
    memset(&info, 0, sizeof(info));
    info.i_mds_hdl = csi_node->pwe_cb->mds_pwe_handle;
    info.i_svc_id  = NCSMDS_SVC_ID_PSS;
    info.i_op      = MDS_UNINSTALL;
    status = ncsmds_api(&info); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */ 
        m_LOG_PSS_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_CRITICAL, 
                           PSS_ERR_MDS_UNINSTALL_FAILED, 
                           status, csi_node->csi_info.env_id); 
        goto enough; 
    }

    /* destroy the associated PSS data structures */ 
    status = pss_pwe_cb_destroy(csi_node->pwe_cb); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_PSS_ERROR_II(NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_CRITICAL, 
                           PSS_ERR_PSSTBL_DESTROY_FAILED, 
                           status, csi_node->csi_info.env_id); 
        goto enough; 
    }

    /* delete this CSI from the global list */ 
    env_id = csi_node->csi_info.env_id; 
    del_me = pss_amf_csi_list_delink(&csi_node->csi_info.work_desc); 
    if (del_me == NULL)
    {
        m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR, 
                          PSS_ERR_CSI_DELINK_FAILED, env_id); 
        goto enough; 
    }

    /* free this CSI node */ 
    m_MMGR_FREE_PSS_CSI_NODE(del_me); 

    /* everything went well */  
    saf_status = SA_AIS_OK; 

    /* log that CSI remove for this env-id is successful */ 
    m_LOG_PSS_HDLN_I(NCSFL_SEV_NOTICE,  PSS_HDLN_AMF_CSI_REMOVE_SUCCESS, env_id);

enough:   
    return saf_status; 
}

 /****************************************************************************\
 *  Name:          pss_amf_csi_remove_all                                     * 
 *                                                                            *
 *  Description:   Removes all the CSIs of PSS                                *
 *                  - pss_amf_csi_remove_one() for all the CSI in the         * 
 *                    global CSI list                                         * 
 *                                                                            *
 *  Arguments:     Nothingi/Ofcourse, global CSI list                         * 
 *                                                                            * 
 *  Returns:       SA_AIS_OK/SA_AIS_ERR_FAILED_OPERATION                      *
 *  NOTE:                                                                     * 
 \****************************************************************************/
SaAisErrorT 
pss_amf_csi_remove_all(void) 
{
    PSS_CSI_NODE    *each_csi, *next_csi; 
    SaAisErrorT     saf_status = SA_AIS_ERR_FAILED_OPERATION; 

    /* for all the CSIs in global CSI list */ 
    next_csi = NULL; 
    each_csi = gl_pss_amf_attribs.csi_list; 
    while (each_csi)
    {
        /* get the next csi, because 'each_csi' will be a free bird soon */ 
        next_csi = each_csi->next; 

        /* remove this csi from PSS */ 
        saf_status = pss_amf_csi_remove_one(each_csi); 
        if (saf_status == SA_AIS_ERR_FAILED_OPERATION)
            break; 
            
        /* go to the next csi */ 
        each_csi = next_csi; 
    }

    gl_pss_amf_attribs.ha_state = NCS_APP_AMF_HA_STATE_NULL;

    /* It is required to set the PSS Control block HA state to NULL */
    gl_pss_amf_attribs.pss_cb->ha_state = NCS_APP_AMF_HA_STATE_NULL;

    if (saf_status == SA_AIS_OK)
    {
        /* log that all the CSIs are destroyed */ 
        m_LOG_PSS_HEADLINE(NCSFL_SEV_NOTICE, PSS_HDLN_AMF_CSI_REMOVE_ALL_SUCCESS); 
    }
    
    return saf_status; 
}

 /****************************************************************************\
 *  Name:          pss_amf_csi_install                                        * 
 *                                                                            *
 *  Description:   Creates a new CSI either in ACTIVE or STANDBY state        *
 *                 - Initializes PSS_CSI_NODE for this CSI                    * 
 *                 - Initializes PSS_CB for this CSI                          * 
 *                 - Creates and install the new Environment                  * 
 *                 - Do the role change of the VDEST, if it is different      * 
 *                   from whatever AMF asked.                                 * 
 *                 - Subscribe for OAA, MAS and PSS subsystem events          *
 *                                                                            *
 *  Arguments:     PW_ENV_ID     - Id of the environment to be created in PSS * 
 *                 SaAmfHAStateT - HA state to be assigned for this Env       *   
 *                                                                            * 
 *  Returns:       PSS_CSI_NODE* / NULL - Pointer to the newly created Env    * 
 *  NOTE:                                                                     * 
 \****************************************************************************/
PSS_CSI_NODE* 
pss_amf_csi_install(PW_ENV_ID envid, SaAmfHAStateT haState) 
{
    uns32           status; 
    PSS_PWE_CB      *pwe_cb = NULL; 
    PSS_CSI_NODE    *csi = NULL; 
    NCSMDS_INFO     info;
    MDS_SVC_ID      subsvc[3] = {0}; 
    
    /* validate the inputs */ 
    if ((envid==0)||(envid > NCSMDS_MAX_PWES))
    {
        /* log the error */
        m_LOG_PSS_ERROR_NO_DATA(NCSFL_SEV_CRITICAL, 
                              NCSFL_LC_HEADLINE, 
                              PSS_ERR_AMF_INVALID_PWE_ID);
        return NULL; 
    }

    /* validate the input state */
    if ((haState != SA_AMF_HA_ACTIVE) && (haState != SA_AMF_HA_STANDBY))
    {
        /* log the error */ 
        m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR, PSS_ERR_AMF_INVALID_HA_STATE, envid);
        return NULL; 
    }

    if (gl_pss_amf_attribs.handles.pss_cb_hdl == 0)
    {
        /* log the error */ /* First PSS should be initialized from the LM APIs */ 
        m_LOG_PSS_ERROR_NO_DATA(NCSFL_SEV_CRITICAL, 
                              NCSFL_LC_HEADLINE, 
                              PSS_ERR_CREATE_LM_API_NOT_YET_CALLED);
        return NULL; 
    }

    csi = pss_amf_csi_list_find_envid(envid); 
    if((csi != NULL) && (csi->csi_info.ha_state == haState)) /* PSS is already installed in this */
    {
        /* log that this environment is already available with PSS */ 
        return csi; 
    }

    /* allocate memory to store the CSI details */ 
    csi = m_MMGR_ALLOC_PSS_CSI_NODE; 
    if (csi == NULL)
    {
        /* log the memory failure error */ 
        m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_CSI_NODE_ALLOC_FAILED, 
                          "pss_amf_csi_install()"); 
        return csi; 
    }
    memset(csi, 0, sizeof(PSS_CSI_NODE)); 

    /* allocate memory for the table details for this environment */ 
    if ((pwe_cb = m_MMGR_ALLOC_PSS_PWE_CB) == NULL)
    {
        /* log the memory failure */ 
        m_LOG_PSS_MEMFAIL(NCSFL_SEV_CRITICAL, PSS_MF_PWE_CB_ALLOC_FAIL, 
                          "pss_amf_csi_install()"); 
        m_MMGR_FREE_PSS_CSI_NODE(csi); 
        return NULL; 
    }

    /* update into the CSI deatails */ 
    csi->pwe_cb = pwe_cb; 

    /* initialize the PWE specific details */ 
    status = pss_pwe_cb_init(gl_pss_amf_attribs.handles.pss_cb_hdl, pwe_cb, envid); 
    if (status != NCSCC_RC_SUCCESS)
    {
       /* log the failure */ 
       m_LOG_PSS_ERROR_II(NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_ERROR, 
                 PSS_ERR_PWE_CB_INIT_FAILED, status, envid);
        m_MMGR_FREE_PSS_CSI_NODE(csi); 
        m_MMGR_FREE_PSS_PWE_CB(pwe_cb);
        return NULL; 
    }
    /* update the env id */ 
    csi->csi_info.env_id = envid; 
    
    /* get the pwe handle */
    csi->csi_info.env_hdl = pwe_cb->mds_pwe_handle;

    /* do I need to do something differently, if the state received is different
     * from whatever VDEST is created with? 
     */ 
    status = pss_amf_csi_vdest_role_change(&csi->csi_info, haState); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_PSS_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                                PSS_ERR_VDEST_ROLE_CHANGE_FAILED, status); 

        /* free the allocated stuff */ 
        m_MMGR_FREE_PSS_CSI_NODE(csi); 
        m_MMGR_FREE_PSS_PWE_CB(pwe_cb);
        return NULL; 
    }

    /* update the haState */ 
    csi->csi_info.ha_state = haState; 

    /* Add to the global CB also */
    csi->pwe_cb->p_pss_cb->ha_state = haState;
    gl_pss_amf_attribs.ha_state = haState;

    /* install VDEST in this PWE */  
    memset(&info, 0, sizeof(info));
    info.i_mds_hdl = pwe_cb->mds_pwe_handle;
    info.i_op      = MDS_INSTALL;
    info.i_svc_id  = NCSMDS_SVC_ID_PSS;

    info.info.svc_install.i_install_scope   = NCSMDS_SCOPE_NONE;
    info.info.svc_install.i_mds_q_ownership = FALSE;
    info.info.svc_install.i_svc_cb          = pss_mds_cb;
    info.info.svc_install.i_yr_svc_hdl      = (MDS_CLIENT_HDL)pwe_cb->hm_hdl;
    info.info.svc_install.i_mds_svc_pvt_ver = (uns8)PSS_MDS_SUB_PART_VERSION;

    status = ncsmds_api(&info);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_PSS_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                             PSS_ERR_MDS_PWE_INSTALL_FAILED, status); 
        m_MMGR_FREE_PSS_CSI_NODE(csi); 
        m_MMGR_FREE_PSS_PWE_CB(pwe_cb);
        return NULL; 
    }

    /* add to the list of CSIs */ 
    if(envid == m_PSS_DEFAULT_ENV_ID)
       pss_amf_csi_list_add_front(csi); 

    /* subscribe for the VDEST events */ 
    memset(&info, 0, sizeof(info));
    info.i_mds_hdl = pwe_cb->mds_pwe_handle;
    info.i_op      = MDS_SUBSCRIBE;
    info.i_svc_id  = NCSMDS_SVC_ID_PSS;
    subsvc[0] = NCSMDS_SVC_ID_OAC;
    subsvc[1] = NCSMDS_SVC_ID_MAS;
    info.info.svc_subscribe.i_num_svcs = 2;
    info.info.svc_subscribe.i_scope    = NCSMDS_SCOPE_NONE;
    info.info.svc_subscribe.i_svc_ids  = subsvc;
    status = ncsmds_api(&info);
    if (status != NCSCC_RC_SUCCESS)
    {
        m_LOG_PSS_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                            PSS_ERR_MDS_SUBSCRIBE_FAILED, status); 
        m_MMGR_FREE_PSS_CSI_NODE(csi); 
        if(envid == m_PSS_DEFAULT_ENV_ID)
        {
           gl_pss_amf_attribs.csi_list = NULL;
        }
        m_MMGR_FREE_PSS_PWE_CB(pwe_cb);
        return NULL; 
    }

    /* subscribe for the ADEST events.
     * For all ADEST events, we use Intra-node scoping to avoid
     * svc events from other nodes.
     */ 
    memset(&info, 0, sizeof(info));
    info.i_mds_hdl = pwe_cb->mds_pwe_handle;
    info.i_op      = MDS_SUBSCRIBE;
    info.i_svc_id  = NCSMDS_SVC_ID_PSS;
    subsvc[0] = NCSMDS_SVC_ID_BAM;
    info.info.svc_subscribe.i_num_svcs = 1;
    info.info.svc_subscribe.i_scope    = NCSMDS_SCOPE_INTRANODE;
    info.info.svc_subscribe.i_svc_ids  = subsvc;
    status = ncsmds_api(&info);
    if (status != NCSCC_RC_SUCCESS)
    {
        m_LOG_PSS_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                            PSS_ERR_MDS_SUBSCRIBE_FAILED, status); 
        m_MMGR_FREE_PSS_CSI_NODE(csi); 
        if(envid == m_PSS_DEFAULT_ENV_ID)
        {
           gl_pss_amf_attribs.csi_list = NULL;
        }
        m_MMGR_FREE_PSS_PWE_CB(pwe_cb);
        return NULL; 
    }

    if(envid == m_PSS_DEFAULT_ENV_ID)
    {
       /* For the default CSI, register PSS's own MIBs with OAA */
       if(pss_register_with_oac(pwe_cb->p_pss_cb) != NCSCC_RC_SUCCESS)
       {
          m_LOG_PSS_HEADLINE(NCSFL_SEV_ERROR, PSS_HDLN_OAC_REG_FAILURE);
          m_MMGR_FREE_PSS_CSI_NODE(csi);
          m_MMGR_FREE_PSS_PWE_CB(pwe_cb);
          return NULL;
       }
    }

#if (NCS_PSS_RED == 1)
    pss_mbcsv_open_ckpt(pwe_cb);
    pss_set_ckpt_role(pwe_cb, haState);
#endif

    return csi; 
}

/*****************************************************************************\
 *  Name:          pss_amf_csi_vdest_role_change                              * 
 *                                                                            *
 *  Description:   Updates the role of the PSS VDEST to ACTIVE or STANDBY     *
 *                                                                            *
 *  Arguments:     NCS_APP_AMF_CSI_ATTRIBS * - to get to the Env Handle       * 
 *                 SaAmfHAStateT             - destination state              *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                          *
 *  NOTE:                                                                     * 
\*****************************************************************************/
uns32
pss_amf_csi_vdest_role_change(NCS_APP_AMF_CSI_ATTRIBS *csi_info, 
                              SaAmfHAStateT haState)
{
    uns32               status; 
    NCSMDS_INFO         info;
    NCSVDA_INFO         vda_info;

    /* check in what state VDEST is created */ 
    memset(&info, 0, sizeof(NCSMDS_INFO)); 
    info.i_op = MDS_QUERY_PWE; 
    info.i_svc_id = PSR_VDEST_ID;  
    info.i_mds_hdl = csi_info->env_hdl; 
    status = ncsmds_api(&info); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_PSS_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                             PSS_ERR_MDS_PWE_QUERY_FAILED, status); 
        return status; 
    }

    /* do we need to do the role change of the VDEST */ 
    if (info.info.query_pwe.info.virt_info.o_role != haState)
    {
        /* log that there is a difference in the state that VDEST is created and
         * the AMF driven state
         */ 
        /* update the role of the VDEST as per AMF given state */ 
        memset(&vda_info, 0, sizeof(vda_info));

        vda_info.req = NCSVDA_VDEST_CHG_ROLE;
        vda_info.info.vdest_chg_role.i_vdest = PSR_VDEST_ID;

        /* change the VDEST role */ 
        vda_info.info.vdest_chg_role.i_new_role = haState;
        status = ncsvda_api(&vda_info);
        if (status != NCSCC_RC_SUCCESS)
        {
            m_LOG_PSS_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                                 PSS_ERR_MDS_VDEST_ROLE_CHG_FAILED, status); 
            return status; 
        }
    }
    return NCSCC_RC_SUCCESS; 
}

 /****************************************************************************\
 *  Name:          pss_amf_csi_list_find_envid                                * 
 *                                                                            *
 *  Description:   Search the global CSI list by the Environment ID           *
 *                                                                            *
 *  Arguments:     PW_ENV_ID - Env id to be found in the list                 * 
 *                                                                            * 
 *  Returns:       PSS_CSI_NODE* / NULL                                       *
 *  NOTE:                                                                     * 
 \****************************************************************************/
PSS_CSI_NODE * 
pss_amf_csi_list_find_envid(PW_ENV_ID env_id)
{
    PSS_CSI_NODE *csi_node = NULL; 

    for (csi_node=gl_pss_amf_attribs.csi_list; csi_node; csi_node=csi_node->next)
    {
        if (csi_node->csi_info.env_id == env_id)
            break; 
    }
    
    return csi_node; 
}

 /****************************************************************************\
 *  Name:          pss_amf_csi_list_find_csiname                              * 
 *                                                                            *
 *  Description:   Search the global CSI list by the CSI Name                 *
 *                                                                            *
 *  Arguments:     SaNameT - CSI name to be found in the list                 * 
 *                                                                            * 
 *  Returns:       PSS_CSI_NODE* / NULL                                       *
 *  NOTE:                                                                     * 
 \****************************************************************************/
PSS_CSI_NODE * 
pss_amf_csi_list_find_csiname(const SaNameT *find_me) 
{
    SaNameT         t_find_me; 
    PSS_CSI_NODE    *me = NULL; 

    memset(&t_find_me, 0, sizeof(SaNameT)); 
    memcpy(&t_find_me, find_me, sizeof(SaNameT)); 

    me = gl_pss_amf_attribs.csi_list; 
    while (me)
    {
        /* do a memcompare for the CSI */ 
        if (m_CMP_HORDER_SANAMET(t_find_me, me->csi_info.work_desc) == 0)
            break; 

        /* better luck next time */ 
        me = me->next; 
    }
    return me; 
}
    
/****************************************************************************\
*  Name:          pss_amf_csi_list_delink                                    * 
*                                                                            *
*  Description:   Delink this CSI details from global CSI list               *
*                                                                            *
*  Arguments:     SaNameT - CSI name to be found in the list                 * 
*                                                                            * 
*  Returns:       PSS_CSI_NODE* / NULL                                       *
*  NOTE:                                                                     * 
\****************************************************************************/
PSS_CSI_NODE* 
pss_amf_csi_list_delink(const SaNameT  *csiName) 
{
    SaNameT         t_del_me; 
    PSS_CSI_NODE     *del_me = NULL; 
    PSS_CSI_NODE     *prev_csi = NULL; 

    memset(&t_del_me, 0, sizeof(SaNameT)); 
    memcpy(&t_del_me, csiName, sizeof(SaNameT)); 

    del_me = gl_pss_amf_attribs.csi_list;
    while (del_me)
    {
        /* do a memcompare for the CSI */
        if (m_CMP_HORDER_SANAMET(t_del_me, del_me->csi_info.work_desc) == 0)
            break;
        
        /* better luck next time */
        prev_csi = del_me; 
        del_me = del_me->next;
    }

    if (del_me == NULL)
    {
        /* csi not found */ 
        return del_me; 
    }

    if (prev_csi == NULL)
    {
        /* del_me is the first node in the list to be deleted */ 
        gl_pss_amf_attribs.csi_list = del_me->next; 
    }
    else
    {
        prev_csi->next = del_me->next; 
    }

    return del_me; 
}

/****************************************************************************\
*  Name:          pss_amf_csi_list_add_front                                 * 
*                                                                            *
*  Description:   Add this CSI information to the global CSI list            *
*                                                                            *
*  Arguments:     PSS_CSI_NODE - CSI details to be added                     * 
*                                                                            * 
*  Returns:       Nothing                                                    *
*  NOTE:                                                                     * 
\****************************************************************************/
void 
pss_amf_csi_list_add_front(PSS_CSI_NODE *new_csi) 
{
    /* add in the front */ 
    if(new_csi->csi_info.env_id != m_PSS_DEFAULT_ENV_ID)
    {
       /* Since the default CSI is already at the head of the list, the first time. */
       new_csi->next = gl_pss_amf_attribs.csi_list;
    }
    gl_pss_amf_attribs.csi_list = new_csi; 

    return; 
}

 /****************************************************************************\
 *  Name:          pss_amf_csi_attrib_decode                                  * 
 *                                                                            *
 *  Description:   Decodes the env-id from CSI attributes                     *
 *                                                                            *
 *  Arguments:     SaAmfCSIDescriptorT csiDescriptor - Details about the      * 
 *                                                     workload being assigned*
 *                                                                            * 
 *  Returns:       Env Id - Successful decoding                               *
 *                 zero   - in case of failure                                *
 *  NOTE:                                                                     * 
 \****************************************************************************/
PW_ENV_ID
pss_amf_csi_attrib_decode(SaAmfCSIAttributeListT csiAttr) 
{
    uns32      decoded_env = 0; 

    /* following varibles aids in Logging */ 
    int8       attrName[255] = {0}; 
    int8       attrValue[255] = {0}; 
    int32      num_args = 0; 
    
    /* make sure that attribute list is not empty */
    if (csiAttr.attr == NULL)
    {
        /* log that CSI Attribute list is empty */ 
        m_LOG_PSS_ERROR_NO_DATA(NCSFL_SEV_ERROR, NCSFL_LC_HEADLINE, 
                                PSS_ERR_AMF_CSI_ATTRIBS_NULL);
        return 0; 
    }

    /* make sure that attribute list is not empty */
    if ((csiAttr.attr->attrName == NULL)|| (csiAttr.attr->attrValue == NULL))
    {
        /* log that CSI Attribute Name/Value is empty */ 
        m_LOG_PSS_ERROR_NO_DATA(NCSFL_SEV_ERROR, NCSFL_LC_HEADLINE, 
                                PSS_ERR_AMF_CSI_ATTRIBS_NAME_OR_ATTRIB_VAL_NULL);
        return 0; 
    }

    if (strcmp(csiAttr.attr->attrName, "Env-Id") != 0)
    {
        /* log that CSI Attribute Name/Value is empty */ 
        m_LOG_PSS_ERROR_NO_DATA(NCSFL_SEV_ERROR, NCSFL_LC_HEADLINE, 
                                PSS_ERR_AMF_CSI_ATTRIBS_NAME_INVALID);
        return 0; 
    }

    /* good amount of input validation, now get to the work */ 
    /* log the received attribute and value pair */ 
    sprintf(attrName, "%s:%s\n", "pss_amf_csi_attrib_decode():attribute name", 
            csiAttr.attr->attrName); 
    sprintf(attrValue, "%s:%s\n", "pss_amf_csi_attrib_decode():attribute value", 
            csiAttr.attr->attrValue); 
    m_LOG_PSS_STR(NCSFL_SEV_NOTICE, attrName); 
    m_LOG_PSS_STR(NCSFL_SEV_NOTICE, attrValue); 

    num_args = sscanf(csiAttr.attr->attrValue, "%d", &decoded_env);
    if (num_args == 0)
    {
        m_LOG_PSS_ERROR_NO_DATA(NCSFL_SEV_ERROR, NCSFL_LC_HEADLINE, 
                                PSS_ERR_AMF_CSI_SSCANF_FAILED);
        return 0; 
    }

    /* validate the given value */ 
    if ((decoded_env <= 0) || (decoded_env > NCSMDS_MAX_PWES))
    { 
        /* log the error */ 
        m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR, PSS_ERR_AMF_CSI_DECODED_ENV_ID_OUTOF_REACH, 
                          decoded_env);
        return 0; 
    }

    /* log the decoded env-id */ 
    m_LOG_PSS_HDLN_I(NCSFL_SEV_INFO, PSS_HDLN_AMF_CSI_ATTRIB_DECODE_SUCCESS, decoded_env);
   
    /* got a valid environment id */  
    return ((PW_ENV_ID)decoded_env); 
} /* pss_amf_csi_attrib_decode() */

/*****************************************************************************\
 *  Name:          pss_amf_sigusr1_handler                                    * 
 *                                                                            *
 *  Description:   to post a message to PSS mailbox to register with AMF      *
 *                 (Handles signal from AMF)                                  *
 *                                                                            *
 *  Arguments:     int i_sug_num -  Signal received                           *
 *                                                                            * 
 *  Returns:       Nothing                                                    *  
 *  NOTE:                                                                     * 
\*****************************************************************************/
void  
pss_amf_sigusr1_handler(int i_sig_num)
{
    /* uninstall the signal handler */ 
    m_NCS_SIGNAL(SIGUSR1, SIG_IGN); 
    m_NCS_SEL_OBJ_IND(gl_pss_amf_attribs.amf_attribs.sighdlr_sel_obj);

    return; 
}

/****************************************************************************\
*  Name:          pss_amf_component_name_set                                 * 
*                                                                            *
*  Description:   To read the file and sets the PSS Component name           *
*                                                                            *
*  Arguments:     void                                                       * 
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
*  NOTE:          m_PSS_COMP_NAME_FILE is used                               * 
\****************************************************************************/
uns32
pss_amf_component_name_set(void)
{
    FILE    *fp = NULL;
    char    comp_name[256] = {0}; 

    /* open the file to read the component name */
    fp = fopen(m_PSS_COMP_NAME_FILE, "r");/* OSAF_LOCALSTATEDIR/ncs_pss_comp_name */
    if (fp == NULL)
    {
        /* log that, there is no component name file */
        return NCSCC_RC_FAILURE;
    }

    if(fscanf(fp, "%s", (char*)&comp_name) != 1)
    {
        fclose(fp);
        return NCSCC_RC_FAILURE;
    }
    fclose(fp);

    if (setenv("SA_AMF_COMPONENT_NAME", (char*)&comp_name, 1) == -1)
    {
        m_LOG_PSS_ERROR_NO_DATA(NCSFL_SEV_CRITICAL, 
                              NCSFL_LC_HEADLINE, 
                              PSS_ERR_COMP_NAME_SETENV_FAILED);
        return NCSCC_RC_FAILURE; 
    }

    return NCSCC_RC_SUCCESS; 
}

/****************************************************************************\
*  Name:          pss_amf_initialize                                         * 
*                                                                            *
*  Description:   Initialises the interface with AMF                         *
*                                                                            *
*  Arguments:     NCS_APP_AMF_ATTRIBS* amf_attribs                           * 
*                 SaAirErrorT        * o_saf_status                          *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
*  NOTE:                                                                     * 
\****************************************************************************/
uns32 pss_amf_initialize(NCS_APP_AMF_ATTRIBS* amf_attribs, SaAisErrorT *o_saf_status)
{
    uns32          status = NCSCC_RC_SUCCESS;
    SaAisErrorT    saf_status = SA_AIS_OK;

    /* initialize the interface with AMF */ 
   saf_status = ncs_app_amf_initialize(amf_attribs); 

   if(!o_saf_status)
      *o_saf_status = saf_status;

   if (saf_status != SA_AIS_OK)
   {
       /* log the failure */ 
       m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR, 
                         PSS_ERR_AMF_INITIALIZE_FAILED, saf_status);

       if(saf_status == SA_AIS_ERR_TRY_AGAIN)
       {
          gl_pss_amf_attribs.amf_attribs.amfHandle = 0;

          /* Set the message type */
          amf_attribs->amf_init_timer.msg_type = MAB_PSS_AMF_INIT_RETRY;

          /* start the timer. */
          status = ncs_app_amf_init_start_timer(&amf_attribs->amf_init_timer);
          if(status == NCSCC_RC_FAILURE)
          {
             /* Timer couldn't be started. Log the error. */
             m_LOG_PSS_ERROR_I(NCSFL_SEV_ERROR,
                         PSS_ERR_AMF_TIMER_FAILED, status);
          }
          return status;
       }

       return NCSCC_RC_FAILURE; 
   }

   /* log that AMF componentization is done */ 
   m_LOG_PSS_HEADLINE(NCSFL_SEV_INFO, PSS_HDLN_AMF_INITIALIZE_SUCCESS); 

   /* Destroy the retry amf initialisation timer if it is created. */
   if(amf_attribs->amf_init_timer.timer_id != TMR_T_NULL)
   {
      m_NCS_TMR_DESTROY(amf_attribs->amf_init_timer.timer_id);
      amf_attribs->amf_init_timer.timer_id = TMR_T_NULL;
   }

   return NCSCC_RC_SUCCESS;

}

/****************************************************************************\
*  Name:          pss_confirm_ha_role                                        * 
*                                                                            *
*  Description:   To set the HA role for all CSIs to "ha_role"               *
*                                                                            *
*  Arguments:     void                                                       * 
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
uns32 pss_confirm_ha_role(PW_ENV_ID pwe_id, SaAmfHAStateT  ha_role)
{
   PSS_CSI_NODE    *csi_node = NULL; 

   csi_node = gl_pss_amf_attribs.csi_list; 

   while (csi_node) /* for all the CSIs */ 
   {
      if(csi_node->csi_info.env_id == pwe_id)
      {
         /* update the state */ 
         csi_node->csi_info.ha_state = ha_role; 
         break;
      }
      /* go to the next CSI */
      csi_node = csi_node->next; 
   }
   if(csi_node == NULL)
      return NCSCC_RC_FAILURE;

   gl_pss_amf_attribs.ha_state = ha_role;
   gl_pss_amf_attribs.csi_list->pwe_cb->p_pss_cb->ha_state = ha_role;
   if(ha_role == SA_AMF_HA_QUIESCING)
   {
      saAmfCSIQuiescingComplete(gl_pss_amf_attribs.amf_attribs.amfHandle, 
         csi_node->pwe_cb->amf_invocation_id, SA_AIS_OK);
   }

   /* send the response to AMF */     
   saAmfResponse(gl_pss_amf_attribs.amf_attribs.amfHandle, 
                 csi_node->pwe_cb->amf_invocation_id, SA_AIS_OK);

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
*  Name:          pss_amf_vdest_role_chg_api_callback                        * 
*                                                                            *
*  Description:   Changes the VDEST role, via a message post to PSS          *
*                                                                            *
*  Arguments:     role - New HA role                                         * 
*                 func - Function to be invoked to set the role              *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
\****************************************************************************/
uns32 pss_amf_vdest_role_chg_api_callback(MDS_CLIENT_HDL handle)
{
   PSS_PWE_CB  *pwe_cb = (PSS_PWE_CB*)m_PSS_VALIDATE_HDL((uns32)handle);
   MAB_MSG     *mm = NULL;

   /* Post a message to PSS mailbox */
   mm = m_MMGR_ALLOC_MAB_MSG;
   if((mm == NULL) || (pwe_cb == NULL))
   {
      if(pwe_cb != NULL)
      {
         ncshm_give_hdl((uns32)handle);
      }
      return NCSCC_RC_FAILURE;
   }

   memset(mm, '\0', sizeof(MAB_MSG));
   /*  Now we will be passing PWE handle rather than its pointer */
   mm->yr_hdl = NCS_INT64_TO_PTR_CAST(handle);
   mm->op = MAB_PSS_VDEST_ROLE_QUIESCED;

   ncshm_give_hdl((uns32)handle);
   if(m_PSS_SND_MSG(pwe_cb->mbx, mm) == NCSCC_RC_FAILURE)
   {
      m_MMGR_FREE_MAB_MSG(mm);
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
*  Name:          pss_amf_sigusr2_handler                                    *
*                                                                            *
*  Description:   to print the MAS data structures into a file in /tmp       *
*                                                                            *
*  Arguments:     int i_sug_num -  Signal received                           *
*                                                                            *
*  Returns:       Nothing                                                    *
*  NOTE:                                                                     *
\****************************************************************************/
static void
pss_amf_sigusr2_handler(int i_sig_num)
{
    /* dump the data structure contents */
    pss_cb_data_dump();

    return;
}

