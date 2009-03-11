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
  
  DESCRIPTION: This file describes AMF Interface implementation for MAS
*****************************************************************************/ 
#if (NCS_MAB == 1)

#include "mab.h"

/* following are the globals to be relooked at */ 
extern MAS_ATTRIBS gl_mas_amf_attribs; 

/* Function which componentise the MAS. */
static uns32
mas_amf_componentize(NCS_APP_AMF_ATTRIBS *amf_attribs); 

/* Stethascope of AMF */ 
static void 
mas_amf_health_check(SaInvocationT,const SaNameT *, SaAmfHealthcheckKeyT*); 

/* pest for Suicide */ 
static void 
mas_amf_comp_terminate(SaInvocationT,  const SaNameT *); 

static void
mas_amf_comp_terminate_complete(void);

/* to create a new CSI or to change the state of an existing CSI */ 
static void
mas_amf_csi_set(SaInvocationT,  const SaNameT *,
                SaAmfHAStateT, SaAmfCSIDescriptorT); 

/* to remove one or more CSIs from MAS */ 
static void
mas_amf_csi_remove(SaInvocationT, const SaNameT*, 
                   const SaNameT*, SaAmfCSIFlagsT); 

/* to create a CSI in ACTIVE state */ 
static uns32 
mas_amf_INIT_ACTIVE_process(void*, PW_ENV_ID, SaAmfCSIDescriptorT);

/* to move to ACTIVE state */ 
static uns32 
mas_amf_ACTIVE_process (void*, PW_ENV_ID, SaAmfCSIDescriptorT);

/* to create a CSI in STANDBY state */ 
static uns32 
mas_amf_INIT_STANDBY_process(void*, PW_ENV_ID, SaAmfCSIDescriptorT);

/* to STANDBY state */ 
static uns32 
mas_amf_STANDBY_process(void*, PW_ENV_ID, SaAmfCSIDescriptorT);

/* to QUIESCED state */ 
static uns32 
mas_amf_QUIESCED_process(void*, PW_ENV_ID, SaAmfCSIDescriptorT);

/* to QUIESCING state */ 
static uns32 
mas_amf_QUIESCING_process(void*, PW_ENV_ID, SaAmfCSIDescriptorT); 

/* from QUIESCING to QUIESCED */ 
static uns32 
mas_amf_QUIESCING_QUIESCED_process(void*, PW_ENV_ID, SaAmfCSIDescriptorT);

/* to create a new CSI */ 
static void 
mas_amf_csi_new(SaInvocationT,  const SaNameT *,
                SaAmfHAStateT, SaAmfCSIDescriptorT);  

/* to do the role change for all the CSIs */ 
static void 
mas_amf_csi_set_all(SaInvocationT,  const SaNameT *,
                SaAmfHAStateT, SaAmfCSIDescriptorT);  

/* do a role change for a specific CSI */ 
static void 
mas_amf_csi_set_one(SaInvocationT,  const SaNameT *,
                SaAmfHAStateT, SaAmfCSIDescriptorT);  

/* to remove a single CSI */ 
static SaAisErrorT 
mas_amf_csi_remove_one(const MAS_CSI_NODE*); 

/* to remove all the CSIs in MAS */ 
static SaAisErrorT
mas_amf_csi_remove_all(void); 

/* to create a new Environment */ 
static uns32
mas_amf_csi_create_and_install_env(PW_ENV_ID, SaAmfHAStateT, SaAmfCSIDescriptorT); 

/* to change the role of the VDEST */ 
static uns32
mas_amf_csi_vdest_role_change(MAS_CSI_NODE *csi, SaAmfHAStateT); 

/* to destroy the table information of an environment */ 
static uns32
mas_amf_csi_mas_tbl_destroy(MAS_TBL *inst); 

/* serach by the env-id in the global CSI list */ 
static MAS_CSI_NODE* 
mas_amf_csi_list_find_envid(PW_ENV_ID); 

/* search by CSI name in the global CSI list */ 
static MAS_CSI_NODE* 
mas_amf_csi_list_find_csiname(const SaNameT *); 

/* delink this node from the global CSI list */ 
static MAS_CSI_NODE*  
mas_amf_csi_list_delink(const SaNameT*); 

/* decodes the CSI attributes and gives the Environment ID */ 
static PW_ENV_ID
mas_amf_csi_attrib_decode(SaAmfCSIAttributeListT csiAttr); 

static void mas_process_sig_usr1_signal(void);

/* signal handler to dump the MAS data structures */ 
static void  
mas_amf_sigusr2_handler(int i_sig_num); 

extern NCS_BOOL  gl_inited;

/* to change the state */
static uns32 
mas_amf_csi_state_change_process(MAS_CSI_NODE *csi_node, 
                                 SaAmfHAStateT haState);

/*****************************************************************************\
 *  Name:          mas_mbx_amf_process                                        * 
 *                                                                            *
 *  Description:   This function processes the messages from AMF              *
 *                 framework as well as native MAS events on its mail box.    *
 *                                                                            *
 *  Arguments:     SYSF_MBX * - MAS mail box                                  *
 *                                                                            * 
 *  Returns:       Nothing.                                                   *
 *                                                                            *
 *  NOTE:         This is the startup function for MAS thread.                * 
 *                Following global varibles are accessed                      *
 *                   - gl_mas_amf_attribs                                     *
\*****************************************************************************/
void
mas_mbx_amf_process(SYSF_MBX   *mas_mbx)
{
    NCS_SEL_OBJ             numfds, ncs_amf_sel_obj; 
    int             count;
    NCS_SEL_OBJ_SET          readfds;
    
    SaAisErrorT     saf_status = SA_AIS_OK; 
    NCS_SEL_OBJ     mbx_fd;

    uns32           status = NCSCC_RC_FAILURE; 

#if (NCS_MAS_RED == 1)
    NCS_MBCSV_ARG   mbcsv_arg; 
    NCS_SEL_OBJ     ncs_mbcsv_sel_obj; 
#endif
    
    m_MAB_DBG_TRACE("\nmas_mbx_amf_process():entered.");

    if (mas_mbx == NULL)
    {
        m_LOG_MAB_NO_CB("mas_mbx_amf_process(): NULL mailbox"); 
        m_MAB_DBG_TRACE("\nmas_mbx_amf_process():left.");
        return;
    }
    m_NCS_MEMSET(&ncs_amf_sel_obj, 0, sizeof(NCS_SEL_OBJ)); 

    /* install the signal handler for SIGUSR2, to dump the data structure contents */ 
    if ((ncs_app_signal_install(SIGUSR2,mas_amf_sigusr2_handler)) == -1) 
    {
        m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_CRITICAL, 
                                NCSFL_LC_FUNC_RET_FAIL,
                                MAB_MAS_ERR_SIGUSR2_INSTALL_FAILED); 
         return;
    }

#if (NCS_MAS_RED == 1)
    m_NCS_MEMSET(&mbcsv_arg, 0, sizeof(NCS_MBCSV_ARG)); 
    mbcsv_arg.i_op = NCS_MBCSV_OP_DISPATCH; 
    mbcsv_arg.i_mbcsv_hdl = gl_mas_amf_attribs.mbcsv_attribs.mbcsv_hdl; 
    mbcsv_arg.info.dispatch.i_disp_flags = SA_DISPATCH_ONE; 
#endif

    /* wait for AMF and native MAS requests */     
    while (1)
    {
        numfds.raise_obj = 0;
        numfds.rmv_obj = 0;

        /* re-intialize the FDs and count */ 
        m_NCS_SEL_OBJ_ZERO(&readfds);
        
        /* add the AMF selection object to FD watche list */ 
        if (gl_mas_amf_attribs.amf_attribs.amfHandle != 0)
        {
            /* Use the following macro to set the saAmfSelectionObject in NCS_SEL_OBJ. */
            m_SET_FD_IN_SEL_OBJ((uns32)gl_mas_amf_attribs.amf_attribs.amfSelectionObject, ncs_amf_sel_obj);
            m_NCS_SEL_OBJ_SET(ncs_amf_sel_obj, &readfds);

            numfds = m_GET_HIGHER_SEL_OBJ(ncs_amf_sel_obj, numfds);
        }
        else 
        {
           m_NCS_SEL_OBJ_SET(gl_mas_amf_attribs.amf_attribs.sighdlr_sel_obj, &readfds);
           numfds = m_GET_HIGHER_SEL_OBJ(gl_mas_amf_attribs.amf_attribs.sighdlr_sel_obj, numfds);
        }

#if (NCS_MAS_RED == 1)
        /* add the MBCSv Selection Object */ 
        if (gl_mas_amf_attribs.mbcsv_attribs.mbcsv_hdl != 0)
        {
            m_SET_FD_IN_SEL_OBJ((uns32)gl_mas_amf_attribs.mbcsv_attribs.mbcsv_sel_obj, ncs_mbcsv_sel_obj);
            m_NCS_SEL_OBJ_SET(ncs_mbcsv_sel_obj, &readfds);

            numfds = m_GET_HIGHER_SEL_OBJ(ncs_mbcsv_sel_obj, numfds);
        }
#endif

        /* add the traditional MBX fd to the FD watch list */
        mbx_fd = m_NCS_IPC_GET_SEL_OBJ(mas_mbx); 
        m_NCS_SEL_OBJ_SET(mbx_fd, &readfds);
        numfds = m_GET_HIGHER_SEL_OBJ(mbx_fd, numfds);
  
        /*
        if (gl_mas_amf_attribs.amf_attribs.amfHandle == 0)
        {
           m_NCS_SEL_OBJ_SET(gl_mas_amf_attribs.amf_attribs.sighdlr_sel_obj, &readfds);
           numfds = m_GET_HIGHER_SEL_OBJ(gl_mas_amf_attribs.amf_attribs.sighdlr_sel_obj, numfds);
        }
        */

        /* wait for the requests indefinately */ 
        count = m_NCS_SEL_OBJ_SELECT(numfds, &readfds, NULL, NULL, NULL);
        if (count > 0)
        {
            if (gl_mas_amf_attribs.amf_attribs.amfHandle == 0) 
            {
                if (m_NCS_SEL_OBJ_ISSET(gl_mas_amf_attribs.amf_attribs.sighdlr_sel_obj, &readfds))
                {
                    mas_process_sig_usr1_signal();

                    m_NCS_SEL_OBJ_CLR(gl_mas_amf_attribs.amf_attribs.sighdlr_sel_obj, &readfds);
                    m_NCS_SEL_OBJ_RMV_IND(gl_mas_amf_attribs.amf_attribs.sighdlr_sel_obj, TRUE, TRUE);
                    m_NCS_SEL_OBJ_DESTROY(gl_mas_amf_attribs.amf_attribs.sighdlr_sel_obj);
                    gl_mas_amf_attribs.amf_attribs.sighdlr_sel_obj.raise_obj = 0;
                    gl_mas_amf_attribs.amf_attribs.sighdlr_sel_obj.rmv_obj = 0;
                }
            }
            else 
            {
                /* AMF FD is selected */ 
                if(m_NCS_SEL_OBJ_ISSET(ncs_amf_sel_obj, &readfds))
                {
                    /* process the event from AMF */
                    saf_status = saAmfDispatch(gl_mas_amf_attribs.amf_attribs.amfHandle, 
                                        gl_mas_amf_attribs.amf_attribs.dispatchFlags);
                    if (saf_status != SA_AIS_OK)
                    {
                        /* log the saf error  */
                        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, 
                                      MAB_MAS_ERR_AMF_DISPATCH_FAILED, 
                                      saf_status);
                        m_MAB_DBG_TRACE("\nmas_mbx_amf_process():left.");
                        return; 
                    }

                    if((SaAmfHandleT)(long)NULL == gl_mas_amf_attribs.amf_attribs.amfHandle)
                         mas_amf_comp_terminate_complete();
                
                    /* clear the bit off for this fd */
                    m_NCS_SEL_OBJ_CLR(ncs_amf_sel_obj, &readfds);
                } /* request from AMF is serviced */ 
            }

            /* MBCSv Interface requests */ 
#if (NCS_MAS_RED == 1)
            if (FD_ISSET(gl_mas_amf_attribs.mbcsv_attribs.mbcsv_sel_obj, &readfds)) 
            {
                status = ncs_mbcsv_svc(&mbcsv_arg); 
                if (status != NCSCC_RC_SUCCESS)
                {
                    /* log the error */ 
                    m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_CRITICAL, 
                                      MAB_MAS_ERR_MBCSV_DISPATCH_FAILED, 
                                      gl_mas_amf_attribs.mbcsv_attribs.mbcsv_hdl, status);
                    m_MAB_DBG_TRACE("\nmas_mbx_amf_process(): ncs_mbcsv_svc():left.");
                    return; 
                }
            }
#endif
            /* Process the messages on the Mail box */ 
            if (m_NCS_SEL_OBJ_ISSET(mbx_fd, &readfds)) 
            {
                /* process native requests of MASv */ 
                status = mas_do_evts(mas_mbx); 
                if (status != NCSCC_RC_SUCCESS)
                {
                    /* log the error */ 
                    m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, 
                                      MAB_MAS_ERR_DO_EVTS_FAILED, 
                                      status);
                    m_MAB_DBG_TRACE("\nmas_mbx_amf_process():left.");
                    return; 
                }
                m_NCS_SEL_OBJ_CLR(mbx_fd, &readfds);

            } /* request from Mail Box is serviced */ 

        } /* if (count > 0) */ 
        else
        {
            /* log the error */ 
        }
    } /* while(1) */

    m_MAB_DBG_TRACE("\nmas_mbx_amf_process():left.");
    return;
}

/*****************************************************************************\
 *  Name:          mas_amf_componentize                                       * 
 *                                                                            *
 *  Description:   Registers the following with AMF framework                 *
 *                   - AMF callbacks                                          *
 *                   - CSI HA state machine callbacks                         *
 *                   - Healthcheck key                                        *
 *                   - Recovery Policy                                        *
 *                   - Start the health check                                 *
 *                                                                            *
 *  Arguments:     NCS_APP_AMF_ATTRIBS * Amf attributes of MAS                *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
static uns32
mas_amf_componentize(NCS_APP_AMF_ATTRIBS *amf_attribs)
{
    NCS_APP_AMF_HA_STATE_HANDLERS   ha_hdls; 
    SaAmfCallbacksT                 amf_clbks; 
    uns32                           status = NCSCC_RC_FAILURE; 

    m_MAB_DBG_TRACE("\nmas_amf_componentize():entered.");
    if (amf_attribs == NULL)
    {
        m_LOG_MAB_NO_CB("mas_amf_componentize(): received NULL to operate on"); 
        m_MAB_DBG_TRACE("\nmas_amf_componentize():left.");
        return NCSCC_RC_FAILURE; 
    }

    /* initialize the HA state machine callbacks */ 
    m_NCS_OS_MEMSET(&ha_hdls, 0, 
                    sizeof(NCS_APP_AMF_HA_STATE_HANDLERS)); 
    ha_hdls.invalidTrans = ncs_app_amf_invalid_state_process; 
    ha_hdls.initActive =  mas_amf_INIT_ACTIVE_process; 
    ha_hdls.activeTrans =  mas_amf_ACTIVE_process; 
    ha_hdls.initStandby = mas_amf_INIT_STANDBY_process; 
    ha_hdls.standbyTrans = mas_amf_STANDBY_process; 
    ha_hdls.quiescedTrans = mas_amf_QUIESCED_process; 
    ha_hdls.quiescingTrans = mas_amf_QUIESCING_process; 
    ha_hdls.quiescingToQuiesced = mas_amf_QUIESCING_QUIESCED_process; 

    /* intialize the AMF callbaclks */ 
    m_NCS_MEMSET(&amf_clbks, 0, 
                    sizeof(SaAmfCallbacksT)); 
    amf_clbks.saAmfHealthcheckCallback = mas_amf_health_check; 
    amf_clbks.saAmfComponentTerminateCallback = mas_amf_comp_terminate; 
    amf_clbks.saAmfCSISetCallback = mas_amf_csi_set;
    amf_clbks.saAmfCSIRemoveCallback = mas_amf_csi_remove;

    /* intialize the AMF attributes */ 
    status = ncs_app_amf_attribs_init(amf_attribs, 
                         m_MAS_HELATH_CHECK_KEY, 
                         m_MAS_DISPATCH, 
                         m_MAS_HB_INVOCATION_TYPE, 
                         m_MAS_RECOVERY, 
                         &ha_hdls, 
                         &amf_clbks); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */ 
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, 
                          MAB_MAS_ERR_AMF_ATTRIBS_INIT_FAILED, status);
        /* reset the attribs */ 
        m_NCS_MEMSET(amf_attribs, 0, sizeof(NCS_APP_AMF_ATTRIBS)); 
        m_MAB_DBG_TRACE("\nmas_amf_componentize():left.");
        return status; 
    }
    
    /* initialize the interface with AMF */ 
    status = mas_amf_initialize(amf_attribs);

   m_MAB_DBG_TRACE("\nmas_amf_componentize():left.");
   return status; 
}

/*****************************************************************************\
 *  Name:          mas_amf_health_check                                       * 
 *                                                                            *
 *  Description:   Does a self diagnose on the health of MAS, and reports     *
 *                 healthe to AMF                                             *
 *                 This function is the callback for Health check of MAS      *
 *                 to the AMF agent.                                          *
 *                                                                            *
 *  Arguments:     SaInvocationT  - for AMF to correlate the response         * 
 *                 const SaNameT *compNamei - Name of the component (MAS)     *
 *                 SaAmfHealthcheckKeyT *healthcheckKey - Key of MAS          *                                      
 *                                                                            * 
 *  Returns:       SA_AIS_OK to AMF                                           *
 *                 Nothing - to the caller of this function                   *
 *                                                                            *
 *  NOTE:          For the time being MAS says its health is fine to AMF.     *
 *                 Not yet decided on what could be checked.                  * 
\*****************************************************************************/
static void
mas_amf_health_check(SaInvocationT invocation,
                     const SaNameT *compName,
                     SaAmfHealthcheckKeyT *healthcheckKey)
{
    SaAisErrorT     saf_status = SA_AIS_OK; 
    MAS_CSI_NODE    *check_me = NULL; 
    MAS_CSI_NODE    *check_my_next = NULL; 

    m_MAB_DBG_TRACE("\nmas_amf_health_check():entered.");
    /* not yet decided what to diagnose */ 
    /* TBD
     * 1. all the CSIs shall be in the same state 
     * 2. what else? 
     */ 

    /*  all the CSIs shall be in the same state  */ 
    check_me = gl_mas_amf_attribs.csi_list; 
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
        m_LOG_MAB_HEADLINE(NCSFL_SEV_INFO, MAB_HDLN_MAS_AMF_HLTH_DOING_AWESOME); 
    else
        m_LOG_MAB_HEADLINE(NCSFL_SEV_CRITICAL, MAB_HDLN_MAS_AMF_HLTH_DOING_AWFUL); 
        

    saAmfResponse(gl_mas_amf_attribs.amf_attribs.amfHandle, 
                  invocation, saf_status); 
    m_MAB_DBG_TRACE("\nmas_amf_health_check():left.");
    return; 
}

/*****************************************************************************\
 *  Name:          mas_amf_prepare_will                                       * 
 *                                                                            *
 *  Description:   To cleanup the mess that MAS enjoyed during its life time  *
 *                 Now, time to cleanup this...                               *
 *                                                                            *
 *  Arguments:     Nothing                                                    * 
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                          *
 *  NOTE:                                                                     * 
\*****************************************************************************/
uns32
mas_amf_prepare_will(void)
{
    uns32           status; 
    NCSVDA_INFO     vda_info; 
    SaAisErrorT     saf_status = SA_AIS_ERR_FAILED_OPERATION; 

    m_MAB_DBG_TRACE("\nmas_amf_prepare_will():entered.");

    /* remove all the CSIs */ 
    saf_status = mas_amf_csi_remove_all();  
    if (saf_status == SA_AIS_ERR_FAILED_OPERATION)
    {
        /* log that component terminate failed because of */ 
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAS_ERR_AMF_CSI_REMOVE_ALL_FAILED, saf_status); 
        m_MAB_DBG_TRACE("\nmas_amf_prepare_will():left.");
        return NCSCC_RC_FAILURE;
    }

    /* destroy the VDEST */ 
    m_NCS_MEMSET(&vda_info, 0, sizeof(NCSVDA_INFO)); 
    vda_info.req = NCSVDA_VDEST_DESTROY; 
    vda_info.info.vdest_destroy.i_create_type = NCSVDA_VDEST_CREATE_SPECIFIC; 
    vda_info.info.vdest_destroy.i_vdest = MAS_VDEST_ID; 
    vda_info.info.vdest_destroy.i_make_vdest_non_persistent = FALSE; 
    status = ncsvda_api(&vda_info); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log that VDEST destroy failed */ 
        m_LOG_MAB_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                                MAB_MAS_ERR_VDEST_DESTROY_FAILED, status); 
        m_MAB_DBG_TRACE("\nmas_amf_prepare_will():left.");
        return status; 
    }


    m_MAB_DBG_TRACE("\nmas_amf_prepare_will():left.");
    return status; 
}

/*****************************************************************************\
 *  Name:          mas_amf_comp_terminate                                     * 
 *                                                                            *
 *  Description:   To terminate the MAS component                             *
 *                                                                            *
 *  Arguments:     SaInvocationT invocation - for AMF to correlate the resp   * 
 *                 const SaNameT *compName  - MAS component name              *
 *                                                                            * 
 *  Returns:       SA_AIS_OK to AMF                                           *
 *                 Nothing - to the caller of this function                   *
 *  NOTE:                                                                     * 
\*****************************************************************************/
static void
mas_amf_comp_terminate(SaInvocationT invocation, const SaNameT *compName)
{
    SaAisErrorT     saf_status = SA_AIS_OK; 

    m_MAB_DBG_TRACE("\nmas_amf_comp_terminate():entered.");

    /* time to send our response to AMF */ 
    saAmfResponse(gl_mas_amf_attribs.amf_attribs.amfHandle, invocation, saf_status); 

    saf_status = saAmfFinalize(gl_mas_amf_attribs.amf_attribs.amfHandle);
    if(saf_status != SA_AIS_OK)
    {
       /* Log the error. */
       m_LOG_MAB_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                                MAB_MAS_ERR_AMF_FINALIZE_FAILED, saf_status); 
    }

    gl_mas_amf_attribs.amf_attribs.amfHandle = (SaAmfHandleT)(long)NULL;

}

/*****************************************************************************\
 *  Name:          mas_amf_comp_terminate_complete                                     * 
 *                                                                            *
 *  Description:   To terminate the MAS component                             *
 *                                                                            *
 *  Arguments:     
 *                                                                            * 
 *  Returns:       
 *                 Nothing - to the caller of this function                   *
 *  NOTE:                                                                     * 
\*****************************************************************************/
static void
mas_amf_comp_terminate_complete()
{
    m_MAB_DBG_TRACE("\nmas_amf_comp_terminate_complete():entered.");

    mas_amf_prepare_will();

    /* Detach the IPC. */
    m_NCS_IPC_DETACH(&gl_mas_amf_attribs.mbx_details.mas_mbx, NULL, NULL);

    /* release the IPC */
    m_NCS_IPC_RELEASE(&gl_mas_amf_attribs.mbx_details.mas_mbx, mab_leave_on_queue_cb);


    m_MAB_DBG_TRACE("\nmas_amf_comp_terminate():left.");

    m_MAB_DBG_TRACE("\nmas_amf_comp_terminate_complete():left.");

    /* finalize the interface with DTSv */ 
    mab_log_unbind();
   
    ncs_agents_shutdown(0,0);

    /* what are you waiting for? Jump into the well */ 
    exit(0); /* I am not sure of the effect of the parameter */ 
}

/*****************************************************************************\
 *  Name:          mas_amf_csi_set                                            * 
 *                                                                            *
 *  Description:   Callback for the CSI assignment by AMF                     *
 *                                                                            *
 *  Arguments:     SaInvocationT invocation - for AMF to correlate resp       * 
 *                 const SaNameT *compName  - MAS component                   *
 *                 SaAmfHAStateT haState    - New HA state to be assigned     *   
 *                 SaAmfCSIDescriptorT csiDescriptor - Details about the      * 
 *                                                     workload being assigned*
 *                                                                            * 
 *  Returns:       SA_AIS_OK to AMF                                           *
 *                 Nothing - to the caller of this function                   *
 *                                                                            *
 *  NOTE:                                                                     * 
\*****************************************************************************/
static void 
mas_amf_csi_set(SaInvocationT invocation,
                const SaNameT *compName,
                SaAmfHAStateT haState,  
                SaAmfCSIDescriptorT csiDescriptor)
{
    uns8 tCompName[SA_MAX_NAME_LENGTH+1] = {0}; 
    uns8 tCsiName[SA_MAX_NAME_LENGTH+1] = {0}; 

    m_MAB_DBG_TRACE("\nmas_amf_csi_set():entered.");

    /* log the information about the received CSI assignment */ 
    memcpy(tCompName, compName->value, compName->length); 
    memcpy(tCsiName, csiDescriptor.csiName.value, csiDescriptor.csiName.length);
    m_LOG_MAB_CSI_DETAILS(NCSFL_SEV_NOTICE, 
                          MAB_HDLN_MAS_AMF_CSI_DETAILS, 
                          csiDescriptor.csiFlags, 
                          tCompName, tCsiName, haState); 

    switch (csiDescriptor.csiFlags)
    {
        case SA_AMF_CSI_ADD_ONE: 
        mas_amf_csi_new(invocation, compName, haState, csiDescriptor); 
        m_MAB_DBG_TRACE("\nmas_amf_csi_set():left for SA_AMF_CSI_ADD_ONE.");
        break; 

        case SA_AMF_CSI_TARGET_ONE: 
        mas_amf_csi_set_one(invocation, compName, haState, csiDescriptor); 
        m_MAB_DBG_TRACE("\nmas_amf_csi_set():left for SA_AMF_CSI_TARGET_ONE.");
        break; 

        case SA_AMF_CSI_TARGET_ALL: 
        mas_amf_csi_set_all(invocation, compName, haState, csiDescriptor); 
        m_MAB_DBG_TRACE("\nmas_amf_csi_set():left for SA_AMF_CSI_TARGET_ALL.");
        break; 

        default: 
        /* log the error code */ 
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAS_ERR_AMF_CSI_FLAGS_ILLEGAL, 
                          csiDescriptor.csiFlags); 
        saAmfResponse(gl_mas_amf_attribs.amf_attribs.amfHandle, invocation, 
                      SA_AIS_ERR_FAILED_OPERATION); 
        m_MAB_DBG_TRACE("\nmas_amf_csi_set():left for default.");
        break; 
    }
    return; 
}

/*****************************************************************************\
 *  Name:          mas_amf_csi_new                                            * 
 *                                                                            *
 *  Description:   Creates a new CSI either in ACTIVE or STANDBY state        *
 *                                                                            *
 *  Arguments:     SaInvocationT invocation - for AMF to correlate resp       * 
 *                 const SaNameT *compName  - MAS component                   *
 *                 SaAmfHAStateT haState    - Initial HA state to be assigned *   
 *                 SaAmfCSIDescriptorT csiDescriptor - Details about the      * 
 *                                                     workload being assigned*
 *                                                                            * 
 *  Returns:       SA_AIS_OK to AMF                                           *
 *                 Nothing - to the caller of this function                   *
 *  NOTE:                                                                     * 
\*****************************************************************************/
static void 
mas_amf_csi_new(SaInvocationT invocation,
                const SaNameT *compName,
                SaAmfHAStateT haState,  
                SaAmfCSIDescriptorT csiDescriptor)
{
    uns32           status; 
    PW_ENV_ID       env_id; 
    MAS_CSI_NODE    *csi = NULL;  
    SaAisErrorT     saf_status = SA_AIS_ERR_FAILED_OPERATION; 
    
    m_MAB_DBG_TRACE("\nmas_amf_csi_new():entered.");

    /* validate the requested state transition */ 
    if ((haState == SA_AMF_HA_QUIESCED) || (haState == SA_AMF_HA_QUIESCING))
    {
        /* log the invalid state transition */ 
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAS_ERR_AMF_INVALID_ROLE, haState); 
        saAmfResponse(gl_mas_amf_attribs.amf_attribs.amfHandle, invocation, saf_status); 
        m_MAB_DBG_TRACE("\nmas_amf_csi_new():left invalid state transition.");
        return; 
    }

    /* decode the attributes to get the Environment ID */ 
    env_id = mas_amf_csi_attrib_decode(csiDescriptor.csiAttr); 
    if (env_id ==0)
    {
        /* log that invalid attribute is rereceived from AMF */ 
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAS_ERR_AMF_ENVID_ILLEGAL, env_id); 
        m_MAB_DBG_TRACE("\nmas_amf_csi_new():left invalid environment id.");
        goto enough; 
    }

    /* make sure that there is no CSI for this Env-id created already */ 
    csi = mas_amf_csi_list_find_envid(env_id); 
    if ((env_id != m_MAS_DEFAULT_ENV_ID)&&(csi != NULL)) /* MAS is already installed in this */
    {
        /* log that this environment is already available with MAS */ 
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAS_ERR_AMF_INCONSISTENT_CSI, env_id); 
        m_MAB_DBG_TRACE("\nmas_amf_csi_new():left CSI already existing/no data for default");
        goto enough; 
    }

    if ((env_id == m_MAS_DEFAULT_ENV_ID)&&(csi != NULL))
    {
        /* default one is created already */ 

        /* is the AMF given state is different from HAPS state? */ 
        if (csi->csi_info.ha_state != haState)
        {
            /* log that AMF asked state is different from HAPS given initial state */ 
            /* ask for a role change, nothing else */
            status = mas_amf_csi_state_change_process(csi, haState);
            if (status != NCSCC_RC_SUCCESS)
            {
                m_LOG_MAB_ERROR_II(NCSFL_LC_STATE_MCH, NCSFL_SEV_ERROR,
                                   MAB_MAS_ERR_AMF_STATE_CHG_FAILED, 
                                   haState, csi->csi_info.ha_state); 
                m_MAB_DBG_TRACE("mas_amf_csi_new(): mas_amf_csi_state_change_process() failed\n"); 
                goto enough; 
            }
        }
        /* update the CSI Name */ 
        csi->csi_info.work_desc.length = csiDescriptor.csiName.length; 
        m_NCS_OS_MEMSET(&csi->csi_info.work_desc.value, 0, SA_MAX_NAME_LENGTH);
        memcpy(&csi->csi_info.work_desc.value, 
                        &csiDescriptor.csiName.value, 
                        csi->csi_info.work_desc.length); 
        saf_status = SA_AIS_OK; 

        /* log the state change success */
        m_LOG_MAB_HDLN_I(NCSFL_SEV_NOTICE, MAB_HDLN_MAS_DEF_CSI_SUCCESS, haState);
        goto enough;  
    }

    /* call into the state machine */ 
    status =  gl_mas_amf_attribs.amf_attribs.ncs_app_amf_hastate_dispatch
                [NCS_APP_AMF_HA_STATE_NULL][haState](NULL, env_id, csiDescriptor); 
    if (status == NCSCC_RC_SUCCESS)
    {
        /* log the environment ID for which CSI creation success */ 
        m_LOG_MAB_HDLN_I(NCSFL_SEV_INFO, MAB_HDLN_MAS_CSI_ADD_ONE_SUCCESS, env_id);

        /* log the state change */ 
        m_LOG_MAB_STATE_CHG(NCSFL_SEV_NOTICE, 
                            MAB_HDLN_MAS_AMF_CSI_STATE_CHG_SUCCESS, 
                            NCS_APP_AMF_HA_STATE_NULL, haState); 
        saf_status = SA_AIS_OK; 
    }
    else
    {
        saf_status = SA_AIS_ERR_FAILED_OPERATION; 

        /* log the environment ID for which CSI creation failed */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR,NCSFL_SEV_CRITICAL, 
                           MAB_MAS_ERR_CSI_ADD_ONE_FAILED, 
                           status, env_id); 

        /* log the state change failure */ 
        m_LOG_MAB_STATE_CHG(NCSFL_SEV_ERROR, 
                            MAB_HDLN_MAS_AMF_CSI_STATE_CHG_FAILED, 
                            NCS_APP_AMF_HA_STATE_NULL, haState); 
    }

enough: 
    /* send the response to AMF */ 
    saAmfResponse(gl_mas_amf_attribs.amf_attribs.amfHandle, invocation, saf_status); 

    m_MAB_DBG_TRACE("\nmas_amf_csi_new():left.");
    return; 
}

/*****************************************************************************\
 *  Name:          mas_amf_csi_create_and_install_env                         * 
 *                                                                            *
 *  Description:   Creates a new CSI either in ACTIVE or STANDBY state        *
 *                 and updates the CSI name into MAS data structures.         *
 *                                                                            *
 *  Arguments:     PW_ENV_ID - New Environment to be created/installed in MAS.*
 *                 SaAmfHAStateT - State of this CSI                          *
 *                 SaAmfCSIDescriptorT - CSI Name and other details           * 
 *                                                                            * 
 *  Returns:       NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                          *
 *  NOTE:                                                                     * 
\*****************************************************************************/
static uns32
mas_amf_csi_create_and_install_env(PW_ENV_ID            envid, 
                                   SaAmfHAStateT        haState,
                                   SaAmfCSIDescriptorT  csiDescriptor)
{
    MAS_CSI_NODE    *csi = NULL; 

    m_MAB_DBG_TRACE("\nmas_amf_csi_create_and_install_env():entered.");
    
    /* install the new CSI */ 
    csi = mas_amf_csi_install(envid, haState); 
    if (csi == NULL)
    {
        /* log that unable to install the environment */ 
        return NCSCC_RC_FAILURE; 
    }

    /* add to the list of CSIs */ 
    mas_amf_csi_list_add_front(csi); 

    /* update the CSI Name filed */ 
    csi->csi_info.work_desc.length = csiDescriptor.csiName.length; 
    m_NCS_MEMSET(&csi->csi_info.work_desc.value, 0, SA_MAX_NAME_LENGTH);
    memcpy(&csi->csi_info.work_desc.value, 
                    &csiDescriptor.csiName.value, 
                    csi->csi_info.work_desc.length); 

    /* everything went well */ 
    m_MAB_DBG_TRACE("\nmas_amf_csi_create_and_install_env():left.");
    return NCSCC_RC_SUCCESS;
}
/*****************************************************************************\
 *  Name:          mas_amf_csi_set_all                                        * 
 *                                                                            *
 *  Description:   to set the assined state to all the CSIs in MAS            *
 *                                                                            *
 *  Arguments:     SaInvocationT invocation - for AMF to correlate resp       * 
 *                 const SaNameT *compName  - MAS component                   *
 *                 SaAmfHAStateT haState    - New HA state to be assigned     *   
 *                 SaAmfCSIDescriptorT csiDescriptor - Details about the      * 
 *                                                     workload being assigned*
 *                                                                            * 
 *  Returns:       SA_AIS_OK to AMF                                           *
 *                 Nothing - to the caller of this function                   *
 *  NOTE:                                                                     * 
\*****************************************************************************/
static void 
mas_amf_csi_set_all(SaInvocationT invocation,
                const SaNameT *compName,
                SaAmfHAStateT haState,  
                SaAmfCSIDescriptorT csiDescriptor)
{
    uns32           status = NCSCC_RC_FAILURE; 
    SaAisErrorT     saf_status = SA_AIS_OK;
    MAS_CSI_NODE    *csi_node = NULL; 
    SaAmfHAStateT   prevState = 0;  
    
    m_MAB_DBG_TRACE("\nmas_amf_csi_set_all():entered.");

    csi_node = gl_mas_amf_attribs.csi_list; 

    while (csi_node) /* for all the CSIs in MAS */ 
    {
        /* perform the asked state change */ 
        status =  gl_mas_amf_attribs.amf_attribs.ncs_app_amf_hastate_dispatch[csi_node->csi_info.ha_state][haState]((void*)csi_node, 
                                                                            csi_node->csi_info.env_id, csiDescriptor); 
        /* update the state */ 
        prevState = csi_node->csi_info.ha_state; 
        if (status == NCSCC_RC_SUCCESS)
        {
            csi_node->csi_info.ha_state = haState; 
#if (NCS_MAS_RED == 1)
            if (haState == SA_AMF_HA_QUIESCED)
                csi_node->inst->amf_invocation_id = invocation; 
#endif
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
        m_LOG_MAB_STATE_CHG(NCSFL_SEV_NOTICE, 
                            MAB_HDLN_MAS_AMF_CSI_STATE_CHG_SUCCESS, 
                            prevState, haState); 
    }
    else
    {
        /* log the state change failure */
        m_LOG_MAB_STATE_CHG(NCSFL_SEV_ERROR, 
                            MAB_HDLN_MAS_AMF_CSI_STATE_CHG_FAILED, 
                            prevState, haState); 
    }

    /* if the state transition is to QUIESCING */ 
    if (haState == SA_AMF_HA_QUIESCING)
    {
        /* log the state change */
        saAmfCSIQuiescingComplete(gl_mas_amf_attribs.amf_attribs.amfHandle, invocation, saf_status); 
        m_MAB_DBG_TRACE("\nmas_amf_csi_set_all():left.");
        return; 
    }

    /* Defer the response to AMF till we get an indication from MDS for QUIESCED */ 
    if (haState != SA_AMF_HA_QUIESCED)
    {
        /* send your response to the Big Boss AMF */     
        saAmfResponse(gl_mas_amf_attribs.amf_attribs.amfHandle, 
                      invocation, saf_status); 
    }
    else if (saf_status != SA_AIS_OK)
    {
        saAmfResponse(gl_mas_amf_attribs.amf_attribs.amfHandle, 
                      invocation, saf_status); 
    }
    m_MAB_DBG_TRACE("\nmas_amf_csi_set_all():left.");
    return; 
}

/**************************************************************************\
 *  Name:          mas_amf_csi_set_one                                        * 
 *                                                                            *
 *  Description:   To change the CSI state of a particular CSI                *
 *                                                                            *
 *  Arguments:     SaInvocationT invocation - for AMF to correlate resp       * 
 *                 const SaNameT *compName  - MAS component                   *
 *                 SaAmfHAStateT haState    - New HA state to be assigned     *   
 *                 SaAmfCSIDescriptorT csiDescriptor - Details about the      * 
 *                                                     workload being assigned*
 *                                                                            * 
 *  Returns:       SA_AIS_OK to AMF                                           *
 *                 Nothing - to the caller of this function                   *
 *  NOTE:                                                                     * 
\*****************************************************************************/
static void 
mas_amf_csi_set_one(SaInvocationT invocation,
                const SaNameT *compName,
                SaAmfHAStateT haState,  
                SaAmfCSIDescriptorT csiDescriptor)
{
    uns32           status = NCSCC_RC_FAILURE; 
    SaAisErrorT     saf_status = SA_AIS_ERR_FAILED_OPERATION;
    MAS_CSI_NODE    *csi;

    m_MAB_DBG_TRACE("\nmas_amf_csi_set_one():entered.");
    
    /* make sure that there is CSI for the state change */ 
    csi= mas_amf_csi_list_find_csiname(&csiDescriptor.csiName); 
    if (csi == NULL)
    {
        /* by this time env must be initialized for this */ 
        /* log the error */ 
        saAmfResponse(gl_mas_amf_attribs.amf_attribs.amfHandle, invocation, saf_status); 
        m_MAB_DBG_TRACE("\nmas_amf_csi_set_one():left could not find the CSI in the list.");
        return; 
    }

    /* transit to the requested state */ 
    status =  gl_mas_amf_attribs.amf_attribs.ncs_app_amf_hastate_dispatch[csi->csi_info.ha_state][haState]((void*)csi, 
                                                                        csi->csi_info.env_id, csiDescriptor); 
    if (status == NCSCC_RC_SUCCESS)
    {
        /* log the state change */ 
        m_LOG_MAB_STATE_CHG(NCSFL_SEV_NOTICE, MAB_HDLN_MAS_AMF_CSI_STATE_CHG_SUCCESS, 
                             csi->csi_info.ha_state, haState); 
        
        /* update the ha state in our data structures */ 
        csi->csi_info.ha_state = haState; 
#if (NCS_MAS_RED == 1)
        if (haState == SA_AMF_HA_QUIESCED)
           csi->inst->amf_invocation_id = invocation; 
#endif

        /* set what we want to communicate to AMF */ 
        saf_status = SA_AIS_OK; 
    }
    else
    {
        /* log the state transition failure */ 
        m_LOG_MAB_STATE_CHG(NCSFL_SEV_ERROR, MAB_HDLN_MAS_AMF_CSI_STATE_CHG_FAILED, 
                           csi->csi_info.ha_state, haState); 
    }

    /* if the state transition is to QUISCING */ 
    if (haState == SA_AMF_HA_QUIESCING)
    {
        saAmfCSIQuiescingComplete(gl_mas_amf_attribs.amf_attribs.amfHandle, invocation, saf_status); 
        return; 
    }

    /* defer sending response to AMF till we get an indication from MDS */
    if (haState != SA_AMF_HA_QUIESCED)
    {
        /* time to report back */     
        saAmfResponse(gl_mas_amf_attribs.amf_attribs.amfHandle, 
                      invocation, saf_status); 
    }
    else if (saf_status != SA_AIS_OK)
    {
        saAmfResponse(gl_mas_amf_attribs.amf_attribs.amfHandle, 
                      invocation, saf_status); 
    }

    m_MAB_DBG_TRACE("\nmas_amf_csi_set_one():left.");
   return; 
}

/*****************************************************************************\
 *  Name:          mas_amf_csi_remove                                         * 
 *                                                                            *
 *  Description:   Callback to remove a particular CSI                        *
 *                                                                            *
 *  Arguments:     SaInvocationT invocation - For AMF to correlate the resp   * 
 *                 const SaNameT *compName  - MAS component as described BAM  *
 *                 const SaNameT *csiName   - Name of the CSI                 * 
 *                 SaAmfCSIFlagsT csiFlags  - Should all of them to be removed*
 *                                                                            * 
 *  Returns:       SA_AIS_OK - to the AMF                                     *
 *                 Nothing to the caller                                      *
 *  NOTE:                                                                     * 
\*****************************************************************************/
static void
mas_amf_csi_remove(SaInvocationT invocation,
                const SaNameT *compName,
                const SaNameT *csiName,
                SaAmfCSIFlagsT csiFlags)
{
    SaAisErrorT     saf_status = SA_AIS_ERR_FAILED_OPERATION; 
    MAS_CSI_NODE    *csi_node = NULL; 
    
    m_MAB_DBG_TRACE("\nmas_amf_csi_remove():entered.");

    /* based on the flags, see whether all the CSI have to be removed, or 
     * a specific one
     */ 
    switch(csiFlags)
    {
        /* remove a particular CSI */ 
        case SA_AMF_CSI_TARGET_ONE: 
        /* find the CSI details in the global CSI list by CSI Name */ 
        /* log the CSI name to be removed */ 
        csi_node = mas_amf_csi_list_find_csiname(csiName); 
        if (csi_node != NULL)
        {
            saf_status = mas_amf_csi_remove_one(csi_node); 
        }
        break; 
        
        /* remove all the CSIs in MAS */ 
        case SA_AMF_CSI_TARGET_ALL: 
        /* log the event */ 
        saf_status = mas_amf_csi_remove_all(); 
        break; 

        /* all the other cases are junk */ 
        case SA_AMF_CSI_ADD_ONE:
        default:
        /* log the event */ 
        break; 
    }/* end of switch() */ 
    
    /* log the response being sent to AMF */ 

    /* send the response */ 
    saAmfResponse(gl_mas_amf_attribs.amf_attribs.amfHandle, 
                  invocation, saf_status); 

    m_MAB_DBG_TRACE("\nmas_amf_csi_remove():left.");
    return; 
}

/*****************************************************************************\
 *  Name:          mas_amf_INIT_ACTIVE_proces                                 * 
 *                                                                            *
 *  Description:   To create a new CSI instance in MAS                        *
 *                                                                            *
 *  Arguments:     void            *data         - MAS, list of CBs           *
 *                 NCS_APP_AMF_ATTRIBS *attribs  - AMF attributes             *
 *                 SaAmfCSIDescriptorT csiDescriptor - Details of the CSI     * 
 *                 SaInvocationT       invocation    - For AMF reposnse       *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* create a new CSI Instance, and put it in ACTIVE State */ 
static uns32
mas_amf_INIT_ACTIVE_process(void                *data, 
                            PW_ENV_ID           envid,
                            SaAmfCSIDescriptorT csiDescriptor)
{
    return mas_amf_csi_create_and_install_env(envid, SA_AMF_HA_ACTIVE, 
                                              csiDescriptor);
}

/*****************************************************************************\
 *  Name:          mas_amf_ACTIVE_proces                                      * 
 *                                                                            *
 *  Description:   To put a CSI in ACTIVE state                               *
 *                                                                            *
 *  Arguments:     void            *data         - MAS, list of CBs           *
 *                 NCS_APP_AMF_ATTRIBS *attribs  - AMF attributes             *
 *                 SaAmfCSIDescriptorT csiDescriptor - Details of the CSI     * 
 *                 SaInvocationT       invocation    - For AMF reposnse       *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* Go to ACTIVE State */ 
static uns32 
mas_amf_ACTIVE_process(void                *data, 
                       PW_ENV_ID           envid,
                       SaAmfCSIDescriptorT csiDescriptor)
{
#if (NCS_MAS_RED==1)  
    MAS_CSI_NODE    *csi_node = (MAS_CSI_NODE*)data; 
  
    /* if the state transition is from Active to Standby, 
     * make sure that, Standby is in complete sync with Active 
     */ 
    if (csi_node->csi_info.ha_state == SA_AMF_HA_STANDBY)
    {
        /* cold sync is not yet complete */ 
        if (csi_node->inst->red.cold_sync_done == FALSE)
        {
            /* log the error condition */ 
            m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAS_ERR_AMF_SBY_TO_ACT_FAILED_COLD_SYNC,
                             csi_node->csi_info.env_id);
            return NCSCC_RC_FAILURE; 
        }

        /* warm-sync is in progress */ 
        if (csi_node->inst->red.warm_sync_in_progress == TRUE)
        {
            /* log the error condition */ 
            m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAS_ERR_AMF_SBY_TO_ACT_FAILED_WARM_SYNC,
                             csi_node->csi_info.env_id);
            return NCSCC_RC_FAILURE; 
        }
    }
#endif
    return (mas_amf_csi_state_change_process((MAS_CSI_NODE*)data, 
                                             SA_AMF_HA_ACTIVE)); 
}

/*****************************************************************************\
 *  Name:          mas_amf_INIT_STANDBY_proces                                * 
 *                                                                            *
 *  Description:   To create a CSI in STANDBY state                           *
 *                                                                            *
 *  Arguments:     void            *data         - MAS, list of CBs           *
 *                 NCS_APP_AMF_ATTRIBS *attribs  - AMF attributes             *
 *                 SaAmfCSIDescriptorT csiDescriptor - Details of the CSI     * 
 *                 SaInvocationT       invocation    - For AMF reposnse       *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
static uns32
mas_amf_INIT_STANDBY_process(void               *data, 
                            PW_ENV_ID           envid,
                            SaAmfCSIDescriptorT csiDescriptor)
{
    return mas_amf_csi_create_and_install_env(envid, SA_AMF_HA_STANDBY, 
                                              csiDescriptor);
}

/*****************************************************************************\
 *  Name:          mas_amf_STANDBY_process                                    * 
 *                                                                            *
 *  Description:   To pust a CSI in STANDBY state                             *
 *                                                                            *
 *  Arguments:     void            *data         - MAS, list of CBs           *
 *                 NCS_APP_AMF_ATTRIBS *attribs  - AMF attributes             *
 *                 SaAmfCSIDescriptorT csiDescriptor - Details of the CSI     * 
 *                 SaInvocationT       invocation    - For AMF reposnse       *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* Go to STANDBY State */
static uns32 
mas_amf_STANDBY_process(void                *data, 
                        PW_ENV_ID           envid,
                        SaAmfCSIDescriptorT csiDescriptor)
{
    return (mas_amf_csi_state_change_process((MAS_CSI_NODE*)data, 
                                             SA_AMF_HA_STANDBY)); 
}

/*****************************************************************************\
 *  Name:          mas_amf_QUIESCED_process                                   * 
 *                                                                            *
 *  Description:   To put a CSI in QUIESCED state                             *
 *                                                                            *
 *  Arguments:     void            *data         - MAS, list of CBs           *
 *                 NCS_APP_AMF_ATTRIBS *attribs  - AMF attributes             *
 *                 SaAmfCSIDescriptorT csiDescriptor - Details of the CSI     * 
 *                 SaInvocationT       invocation    - For AMF reposnse       *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* Go to QUIESCED State */
static uns32 
mas_amf_QUIESCED_process(void                *data, 
                         PW_ENV_ID           envid,
                         SaAmfCSIDescriptorT csiDescriptor)
{
    /* no data has been identified to do the checkpoint at this point of time */ 
    return (mas_amf_csi_state_change_process((MAS_CSI_NODE*)data, 
                                             SA_AMF_HA_QUIESCED)); 
}

/*****************************************************************************\
 *  Name:          mas_amf_QUIESCING_process                                  * 
 *                                                                            *
 *  Description:   To put a CSI in QUIESCING state                            *
 *                                                                            *
 *  Arguments:     void            *data         - MAS, list of CBs           *
 *                 NCS_APP_AMF_ATTRIBS *attribs  - AMF attributes             *
 *                 SaAmfCSIDescriptorT csiDescriptor - Details of the CSI     * 
 *                 SaInvocationT       invocation    - For AMF reposnse       *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* Go to QUIESCING State */
static uns32 
mas_amf_QUIESCING_process(void                *data, 
                          PW_ENV_ID           envid,
                          SaAmfCSIDescriptorT csiDescriptor)
{
    /* Complete all the pending transactions */ 
    /* PSS process requests one after the other.  It will not have any
     * pending requests to be serviced and getting this callback is not
     * possible 
     */ 

    /* tell AMF framework that QUISCEING is complete */ 
    return NCSCC_RC_SUCCESS; 
}

/*****************************************************************************\
 *  Name:          mas_amf_QUIESCING_QUIESCED_process                         * 
 *                                                                            *
 *  Description:   To put a CSI in QUIESCING state                            *
 *                                                                            *
 *  Arguments:     void            *data         - MAS, list of CBs           *
 *                 NCS_APP_AMF_ATTRIBS *attribs  - AMF attributes             *
 *                 SaAmfCSIDescriptorT csiDescriptor - Details of the CSI     * 
 *                 SaInvocationT       invocation    - For AMF reposnse       *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
 *                 NCSCC_RC_FAILURE   -  failure                              *
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* Go to QUIESCED from QUIESCING State */
static uns32 
mas_amf_QUIESCING_QUIESCED_process(void                *data, 
                                   PW_ENV_ID           envid,
                                   SaAmfCSIDescriptorT csiDescriptor)
{
    /* nothing to do */ 
    return NCSCC_RC_SUCCESS; 
}

/****************************************************************************\
*  Name:          mas_amf_csi_state_change_process                           * 
*                                                                            *
*  Description:   Changes the VDEST role and MBCSv Channel role              * 
*                                                                            *
*  Arguments:     MAS_CSI_NODE * csi_node - CSI Details                      * 
*                  SaAmfHAStateT haState  - New state to be changed to       *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Message was posted successfully         *
*                 Nothing - to the caller of this function                   *
*  NOTE:                                                                     * 
\****************************************************************************/
static uns32 
mas_amf_csi_state_change_process(MAS_CSI_NODE *csi_node, 
                                 SaAmfHAStateT haState)
{
    uns32 status; 

#if (NCS_MAS_RED == 1)
    NCS_MBCSV_ARG     mbcsv_arg;
#endif

    status =  mas_amf_csi_vdest_role_change(csi_node, haState); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* enough logging is there inside mas_amf_csi_vdest_role_change() function */
        return status; 
    }

#if (NCS_MAS_RED == 1)
    /* do the MBCSv role change */ 
    m_NCS_MEMSET(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));
    mbcsv_arg.i_op = NCS_MBCSV_OP_CHG_ROLE;
    mbcsv_arg.i_mbcsv_hdl = csi_node->inst->red.mbcsv_hdl;
    mbcsv_arg.info.chg_role.i_ckpt_hdl = csi_node->inst->red.ckpt_hdl;
    mbcsv_arg.info.chg_role.i_ha_state = haState;
    
    status = ncs_mbcsv_svc(&mbcsv_arg);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR,NCSFL_SEV_CRITICAL, 
                           MAB_MAS_ERR_MBCSV_CKPT_CHG_ROLE_FAILED, 
                           haState, status); 
    }
    /* update the state in MBCS Redundancy related data structure */
    csi_node->inst->red.ha_state = haState; 
#endif /* #if (NCS_MAS_RED == 1) */ 

    return status; 
}

/*****************************************************************************\
 *  Name:          mas_amf_csi_remove_one                                     * 
 *                                                                            *
 *  Description:   Removes the CSI from the global CSI list                   *
 *                 - Uninstall the Environment                                *
 *                 - Destroy the Environment                                  *
 *                 - Destroy the MAS_TBL information of this Environment      * 
 *                 - Delete this CSI from the global CSI list                 * 
 *                 - free the CSI node                                        *  
 *                                                                            *
 *  Arguments:     MAS_CSI_NODE * pointer to the CSI to be deleted            * 
 *                                                                            * 
 *  Returns:       SA_AIS_OK/SA_AIS_ERR_FAILED_OPERATION                      *
 *  NOTE:                                                                     * 
\*****************************************************************************/
static SaAisErrorT 
mas_amf_csi_remove_one(const MAS_CSI_NODE *csi_node)
{
    NCSVDA_INFO     vda_info; 
    NCSMDS_INFO     info;
    uns32           status;
    MAS_CSI_NODE    *del_me = NULL; 
    SaAisErrorT     saf_status = SA_AIS_ERR_FAILED_OPERATION; 
    PW_ENV_ID       env_id = 0; 
    
#if (NCS_MAS_RED == 1)
    NCS_MBCSV_ARG     mbcsv_arg;
#endif

    m_MAB_DBG_TRACE("\nmas_amf_csi_remove_one():entered.");

#if (NCS_MAS_RED == 1)
    /* Close the Checkpoint channel */ 
    m_NCS_MEMSET(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));
    mbcsv_arg.i_op = NCS_MBCSV_OP_CLOSE;
    mbcsv_arg.i_mbcsv_hdl = csi_node->inst->red.mbcsv_hdl;
    mbcsv_arg.info.close.i_ckpt_hdl = csi_node->inst->red.ckpt_hdl;

    status = ncs_mbcsv_svc(&mbcsv_arg);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR,NCSFL_SEV_CRITICAL, 
                           MAB_MAS_ERR_MBCSV_CKPT_CLOSE_FAILED, 
                           csi_node->csi_info.env_id,status); 
        goto enough; 
    }
#endif /* #if (NCS_MAS_RED == 1) */ 

    /* uninstall the Env */ 
    m_NCS_MEMSET(&info, 0, sizeof(info));
    info.i_mds_hdl = csi_node->inst->mds_hdl;
    info.i_svc_id  = NCSMDS_SVC_ID_MAS;
    info.i_op      = MDS_UNINSTALL;
    status = ncsmds_api(&info); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR,NCSFL_SEV_CRITICAL, 
                           MAB_MAS_ERR_MDS_UNINSTALL_FAILED, 
                           status, csi_node->csi_info.env_id); 
        goto enough; 
    }

    if ((gl_inited == TRUE)&&(csi_node->csi_info.env_id != m_MAS_DEFAULT_ENV_ID))
    {
        /* destroy the env only if it is not DEFAULT One */
        m_NCS_MEMSET(&vda_info, 0, sizeof(vda_info));
        vda_info.req = NCSVDA_PWE_DESTROY; 
        vda_info.info.pwe_destroy.i_mds_pwe_hdl = csi_node->csi_info.env_hdl; 
        status = ncsvda_api(&vda_info);
        if (status != NCSCC_RC_SUCCESS)
        {
            /* TBD-- I am not sure whether I should continue or not */ 
            /* log the error */ 
            m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR,NCSFL_SEV_CRITICAL, 
                               MAB_MAS_ERR_MDS_PWE_DESTROY_FAILED, 
                               status, csi_node->csi_info.env_id); 
            goto enough; 
        }
    }

    /* destroy the associated MAS data structures */ 
    status = mas_amf_csi_mas_tbl_destroy(csi_node->inst); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_FUNC_RET_FAIL,NCSFL_SEV_CRITICAL, 
                           MAB_MAS_ERR_MASTBL_DESTROY_FAILED, 
                           status, csi_node->csi_info.env_id); 
        goto enough; 
    }

    /* delete this CSI from the global list */ 
    env_id = csi_node->csi_info.env_id; 
    del_me = mas_amf_csi_list_delink(&csi_node->csi_info.work_desc); 
    if (del_me == NULL)
    {
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, 
                          MAB_MAS_ERR_CSI_DELINK_FAILED, env_id); 
        goto enough; 
    }

    /* free this CSI node */ 
    m_MMGR_FREE_MAS_CSI_NODE(del_me); 

    /* everything went well */  
    saf_status = SA_AIS_OK; 

    /* log that CSI remove for this env-id is successful */ 
    m_LOG_MAB_HDLN_I(NCSFL_SEV_NOTICE,  MAB_HDLN_MAS_CSI_REMOVE_SUCCESS, env_id);

enough:   
    m_MAB_DBG_TRACE("\nmas_amf_csi_remove_one():left.");
    return saf_status; 
}

 /****************************************************************************\
 *  Name:          mas_amf_csi_remove_all                                     * 
 *                                                                            *
 *  Description:   Removes all the CSIs of MAS                                *
 *                  - mas_amf_csi_remove_one() for all the CSI in the         * 
 *                    global CSI list                                         * 
 *                                                                            *
 *  Arguments:     Nothingi/Ofcourse, global CSI list                         * 
 *                                                                            * 
 *  Returns:       SA_AIS_OK/SA_AIS_ERR_FAILED_OPERATION                      *
 *  NOTE:                                                                     * 
 \****************************************************************************/
static SaAisErrorT 
mas_amf_csi_remove_all(void) 
{
    MAS_CSI_NODE    *each_csi, *next_csi; 
    SaAisErrorT     saf_status = SA_AIS_OK; 

    m_MAB_DBG_TRACE("\nmas_amf_csi_remove_all():entered.");

    /* for all the CSIs in global CSI list */ 
    next_csi = NULL; 
    each_csi = gl_mas_amf_attribs.csi_list; 
    while (each_csi)
    {
        /* get the next csi, because 'each_csi' will be a free bird soon */ 
        next_csi = each_csi->next; 

        /* remove this csi from MAS */ 
        saf_status = mas_amf_csi_remove_one(each_csi); 
        if (saf_status == SA_AIS_ERR_FAILED_OPERATION)
            break; 
            
        /* go to the next csi */ 
        each_csi = next_csi; 
    }

    if (saf_status == SA_AIS_OK)
    {
        /* log that all the CSIs are destroyed */ 
        m_LOG_MAB_HEADLINE(NCSFL_SEV_INFO, MAB_HDLN_MAS_AMF_CSI_REMOVE_ALL_SUCCESS); 

    }
    
    m_MAB_DBG_TRACE("\nmas_amf_csi_remove_all():left.");
    return saf_status; 
}

 /****************************************************************************\
 *  Name:          mas_amf_csi_mas_tbl_destroy                                * 
 *                                                                            *
 *  Description:   Destroys the tables information of a CSI                   *
 *                                                                            *
 *  Arguments:     MAS_TBL * - table information to be destroyed              * 
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                          *  
 *  NOTE:                                                                     * 
 \****************************************************************************/
static uns32
mas_amf_csi_mas_tbl_destroy(MAS_TBL *inst)
{
    uns32           i;

    m_MAB_DBG_TRACE("\nmas_amf_csi_mas_tbl_destroy():entered.");

    /* nothing to delete */ 
    if (inst == NULL)
    {
        m_MAB_DBG_TRACE("\nnmas_amf_csi_mas_tbl_destroy():left.");
        return NCSCC_RC_SUCCESS; 
    }

    m_MAS_LK(&inst->lock);

    /* free the filter details of all the tables of this CSI */ 
    for(i = 0;i < MAB_MIB_ID_HASH_TBL_SIZE;i++)
    {
        /* free the table-records in this bucket */ 
        mas_table_rec_list_free(inst->hash[i]); 
    }/* for all the tables in the hash table */ 

    m_MAS_LK_DLT(&inst->lock);
#if (NCS_MAS_RED == 1)
    if (inst->red.process_msg)
        m_MMGR_FREE_MAB_MSG(inst->red.process_msg); 
#endif
    ncshm_destroy_hdl(NCS_SERVICE_ID_MAB, inst->hm_hdl);     
    m_MMGR_FREE_MAS_TBL(inst);

    m_MAB_DBG_TRACE("\nnmas_amf_csi_mas_tbl_destroy():left.");
    return NCSCC_RC_SUCCESS;
}  

 /****************************************************************************\
 *  Name:          mas_amf_csi_install                                        * 
 *                                                                            *
 *  Description:   Creates a new CSI either in ACTIVE or STANDBY state        *
 *                 - Initializes MAS_CSI_NODE for this CSI                    * 
 *                 - Initializes MAS_TBL for this CSI                         * 
 *                 - Creates and install the new Environment                  * 
 *                 - Do the role change of the VDEST, if it is different      * 
 *                   from whatever AMF asked.                                 * 
 *                 - Subscribe for OAA, MAS and PSS subsystem events          *
 *                                                                            *
 *  Arguments:     PW_ENV_ID     - Id of the environment to be created in MAS * 
 *                 SaAmfHAStateT - HA state to be assigned for this Env       *   
 *                                                                            * 
 *  Returns:       MAS_CSI_NODE* / NULL - Pointer to the newly created Env    * 
 *  NOTE:                                                                     * 
 \****************************************************************************/
MAS_CSI_NODE* 
mas_amf_csi_install(PW_ENV_ID envid, SaAmfHAStateT haState) 
{
    uns32           status; 
    MAS_TBL         *inst = NULL; 
    MAS_CSI_NODE    *csi = NULL; 
    NCSMDS_INFO     info;
    MDS_SVC_ID      subsvc[4] = {0}; 
    NCSVDA_INFO     vda_info; 
    
#if (NCS_MAS_RED == 1)
    NCS_MBCSV_ARG     mbcsv_arg;
#endif

    m_MAB_DBG_TRACE("\nmas_amf_csi_install():entered.");

    /* validate the HA State */ 
    if ((haState == SA_AMF_HA_QUIESCED)||(haState == SA_AMF_HA_QUIESCING))
    {
        /* log the error */ 
        return csi; 
    }

    /* evaluate the Environment ID */ 
    if ((envid<m_MAS_DEFAULT_ENV_ID) || (envid>=m_MAS_MAX_ENV_ID))
    {
        /* log the error */ 
        return csi; 
    }

    /* allocate memory to store the CSI details */ 
    csi = m_MMGR_ALLOC_MAS_CSI_NODE; 
    if (csi == NULL)
    {
        /* log the memory failure error */ 
        m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_MAS_CSI_NODE_ALLOC_FAILED, 
                          "mas_amf_csi_install()"); 
        m_MAB_DBG_TRACE("\nmas_amf_csi_install():left.");
        return csi; 
    }
    m_NCS_MEMSET(csi, 0, sizeof(MAS_CSI_NODE)); 

    /* allocate memory for the table details for this environment */ 
    if ((inst = m_MMGR_ALLOC_MAS_TBL ) == NULL)
    {
        /* log the memory failure */ 
        m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL,MAB_MF_MAS_TBL_ALLOC_FAILED, 
                          "mas_amf_csi_install()"); 
        m_MMGR_FREE_MAS_CSI_NODE(csi); 
        m_MAB_DBG_TRACE("\nmas_amf_csi_install():left.");
        return NULL; 
    }

    /* update into the CSI deatails */ 
    csi->inst = inst; 

    /* initialize the table details */ 
    m_NCS_MEMSET(inst,0,sizeof(MAS_TBL));
    m_MAS_LK_CREATE(&inst->lock);
    m_MAS_LK(&inst->lock);

    inst->hm_poolid = 0; /* create->i_hm_poolid; */ 
    inst->hm_hdl    = ncshm_create_hdl(inst->hm_poolid, NCS_SERVICE_ID_MAB, inst);
    inst->vrid      = envid;
    inst->mbx       = &gl_mas_amf_attribs.mbx_details.mas_mbx;
    inst->lm_cbfnc  = (MAB_LM_CB) mas_evt_cb;
    
    /* initialize the hash table */  
    mas_row_rec_init(inst);

    /* create the new PWE */ 
    if ((gl_inited == TRUE) &&
        ((envid != m_MAS_DEFAULT_ENV_ID) ||       /* For multiple-CSI support */
         ((envid == m_MAS_DEFAULT_ENV_ID) && 
          (gl_mas_amf_attribs.amf_attribs.amfHandle != 0)))) /* For CSI-1 creation after CSI-remove-all */
    {
        /* create the new PWE */ 
        m_NCS_MEMSET(&vda_info, 0, sizeof(NCSVDA_INFO)); 
        vda_info.req = NCSVDA_PWE_CREATE; 
        vda_info.info.pwe_create.i_mds_vdest_hdl = gl_mas_amf_attribs.mas_mds_hdl; 
        vda_info.info.pwe_create.i_pwe_id = envid; 
        status =  ncsvda_api(&vda_info); 
        if (status != NCSCC_RC_SUCCESS)
        {
            /* log the error */ 
            m_LOG_MAB_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                                 MAB_MAS_ERR_MDS_PWE_CREATE_FAILED, status); 
            /* free the inst */ 
            m_MAS_UNLK(&inst->lock);
            mas_amf_csi_mas_tbl_destroy(inst); 
            m_MMGR_FREE_MAS_CSI_NODE(csi); 

            /* log the failure */ 
            m_MAB_DBG_TRACE("\nmas_amf_csi_install():left.");
            return NULL; 
        }
        /* store the PWE handle to be used while destroying this PWE */ 
        inst->mds_hdl = csi->csi_info.env_hdl = vda_info.info.pwe_create.o_mds_pwe_hdl; 

    }/* create a non-default PWE */ 
    else if ((gl_inited == FALSE)&&(envid == m_MAS_DEFAULT_ENV_ID))
    {
        inst->mds_hdl = csi->csi_info.env_hdl = gl_mas_amf_attribs.mas_mds_def_pwe_hdl; 
    }
        
    /* update the env id */ 
    csi->csi_info.env_id = envid; 

    /* do I need to do something differently, if the state received is different
     * from whatever VDEST is created with? 
     */ 
    status = mas_amf_csi_vdest_role_change(csi, haState); 
    if (status != NCSCC_RC_SUCCESS)
    {
        m_LOG_MAB_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                                MAB_MAS_ERR_VDEST_ROLE_CHANGE_FAILED, status); 
        /* log the failure */ 
        m_MAS_UNLK(&inst->lock);
        mas_amf_csi_mas_tbl_destroy(inst);
        m_MMGR_FREE_MAS_CSI_NODE(csi); 
        m_MAB_DBG_TRACE("\nmas_amf_csi_install():left.");
        return NULL; 
    }

    /* update the haState */ 
    csi->csi_info.ha_state = haState; 

#if (NCS_MAS_RED == 1)
    /* copy this stuff into redundancy area of this instance of MAS */ 
    /* This information will be helpful during the MBCSv callbacks */
    inst->red.ha_state  = haState;
    inst->red.mbcsv_hdl =  gl_mas_amf_attribs.mbcsv_attribs.mbcsv_hdl; 
#endif

    /* install VDEST in this PWE */  
    m_NCS_MEMSET(&info, 0, sizeof(info));
    info.i_mds_hdl = inst->mds_hdl;
    info.i_op      = MDS_INSTALL;
    info.i_svc_id  = NCSMDS_SVC_ID_MAS;

    info.info.svc_install.i_install_scope   = NCSMDS_SCOPE_NONE;
    info.info.svc_install.i_mds_q_ownership = FALSE;
    info.info.svc_install.i_svc_cb          = mas_mds_cb;
    info.info.svc_install.i_yr_svc_hdl      = inst->hm_hdl;
    info.info.svc_install.i_mds_svc_pvt_ver = (uns8)MAS_MDS_SUB_PART_VERSION;

    status = ncsmds_api(&info);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                            MAB_MAS_ERR_MDS_PWE_INSTALL_FAILED, status); 
        m_MAS_UNLK(&inst->lock);
        mas_amf_csi_mas_tbl_destroy(inst); 
        m_MMGR_FREE_MAS_CSI_NODE(csi); 

        m_MAB_DBG_TRACE("\nmas_amf_csi_install():left.");
        return NULL; 
    }

#if (NCS_MAS_RED == 1)
    /* Open the Checkpoint channel */ 
    m_NCS_MEMSET(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));
    mbcsv_arg.i_op = NCS_MBCSV_OP_OPEN;
    mbcsv_arg.i_mbcsv_hdl = inst->red.mbcsv_hdl;
    mbcsv_arg.info.open.i_pwe_hdl = (uns32)inst->mds_hdl;
    mbcsv_arg.info.open.i_client_hdl = inst->hm_hdl;

    status = ncs_mbcsv_svc(&mbcsv_arg);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR,NCSFL_SEV_CRITICAL, 
                           MAB_MAS_ERR_MBCSV_CKPT_OPEN_FAILED, 
                           envid,status); 
        m_MAS_UNLK(&inst->lock);
        mas_amf_csi_mas_tbl_destroy(inst); 
        m_MMGR_FREE_MAS_CSI_NODE(csi); 

        m_MAB_DBG_TRACE("\nmas_amf_csi_install():mas_mbcsv_checkpoint_open():left.");
        return NULL; 
    }
    /* update the checkpoint handle for this PWE */ 
    inst->red.ckpt_hdl = mbcsv_arg.info.open.o_ckpt_hdl;

    /* Set the MBCSv Role */ 
    m_NCS_MEMSET(&mbcsv_arg, '\0', sizeof(NCS_MBCSV_ARG));
    mbcsv_arg.i_op = NCS_MBCSV_OP_CHG_ROLE;
    mbcsv_arg.i_mbcsv_hdl = inst->red.mbcsv_hdl;
    mbcsv_arg.info.chg_role.i_ckpt_hdl = inst->red.ckpt_hdl;
    mbcsv_arg.info.chg_role.i_ha_state = haState;
    
    status = ncs_mbcsv_svc(&mbcsv_arg);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR,NCSFL_SEV_CRITICAL, 
                           MAB_MAS_ERR_MBCSV_CKPT_CHG_ROLE_FAILED, 
                           envid,status); 
        m_MAS_UNLK(&inst->lock);
        mas_amf_csi_mas_tbl_destroy(inst); 
        m_MMGR_FREE_MAS_CSI_NODE(csi); 

        m_MAB_DBG_TRACE("\nmas_amf_csi_install():mbcsv_set_role():left.");
        return NULL; 
    }
#endif /* #if (NCS_MAS_RED == 1) */ 

    /* SPLITTED subscriptions for RED subscription from MAS to OAC */
    
    /* subscribe for the events */ 
    m_NCS_MEMSET(&info, 0, sizeof(info));
    info.i_mds_hdl = inst->mds_hdl;
    info.i_op      = MDS_SUBSCRIBE;
    info.i_svc_id  = NCSMDS_SVC_ID_MAS;

    subsvc[0] = NCSMDS_SVC_ID_MAS;

    info.info.svc_subscribe.i_num_svcs = 1;
    info.info.svc_subscribe.i_scope    = NCSMDS_SCOPE_NONE;
    info.info.svc_subscribe.i_svc_ids  = subsvc;

    status = ncsmds_api(&info);
    if (status != NCSCC_RC_SUCCESS)
    {
        m_LOG_MAB_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                            MAB_MAS_ERR_MDS_SUBSCRIBE_FAILED, status); 
        m_MAS_UNLK(&inst->lock);
        m_MMGR_FREE_MAS_TBL(inst);
        m_MMGR_FREE_MAS_CSI_NODE(csi); 
        m_MAB_DBG_TRACE("\nmas_amf_csi_install():lef.");
        return NULL; 
    }
    
    /* subscribe for the events */ 
    m_NCS_MEMSET(&info, 0, sizeof(info));
    info.i_mds_hdl = inst->mds_hdl;
    info.i_op      = MDS_RED_SUBSCRIBE;
    info.i_svc_id  = NCSMDS_SVC_ID_MAS;

    subsvc[0] = NCSMDS_SVC_ID_OAC;

    info.info.svc_subscribe.i_num_svcs = 1;
    info.info.svc_subscribe.i_scope    = NCSMDS_SCOPE_NONE;
    info.info.svc_subscribe.i_svc_ids  = subsvc;

    status = ncsmds_api(&info);
    if (status != NCSCC_RC_SUCCESS)
    {
        m_LOG_MAB_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                            MAB_MAS_ERR_MDS_SUBSCRIBE_FAILED, status); 
        m_MAS_UNLK(&inst->lock);
        mas_amf_csi_mas_tbl_destroy(inst); 
        m_MMGR_FREE_MAS_CSI_NODE(csi); 
        m_MAB_DBG_TRACE("\nmas_amf_csi_install():left.");
        return NULL; 
    }

    m_MAS_UNLK(&inst->lock);

    /* log that CSI remove for this env-id is successful */ 
    m_LOG_MAB_HDLN_I(NCSFL_SEV_NOTICE,  MAB_HDLN_MAS_CSI_INSTALL_SUCCESS, envid);

    m_MAB_DBG_TRACE("\nmas_amf_csi_install():left.");
    return csi; 
}

/*****************************************************************************\
 *  Name:          mas_amf_csi_vdest_role_change                              * 
 *                                                                            *
 *  Description:   Updates the role of the MAS VDEST to ACTIVE or STANDBY     *
 *                                                                            *
 *  Arguments:     MAS_CSI_NODE * - to get to the Env Handle and MAS details  * 
 *                 SaAmfHAStateT  - destination state                         *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                          *
 *  NOTE:                                                                     * 
\*****************************************************************************/
static uns32
mas_amf_csi_vdest_role_change(MAS_CSI_NODE *csi, 
                              SaAmfHAStateT haState)
{
    uns32               status; 
    NCSMDS_INFO         info;
    NCSVDA_INFO         vda_info;

#if (NCS_MAS_RED == 1)
    MAS_TBL             *inst = csi->inst; 
#endif

    NCS_APP_AMF_CSI_ATTRIBS *csi_info = &csi->csi_info; 

    m_MAB_DBG_TRACE("\nmas_amf_csi_vdest_role_change():entered.");

    /* check in what state VDEST is created */ 
    m_NCS_MEMSET(&info, 0, sizeof(NCSMDS_INFO)); 
    info.i_op = MDS_QUERY_PWE; 
    info.i_svc_id = MAS_VDEST_ID;  
    info.i_mds_hdl = csi_info->env_hdl; 
    status = ncsmds_api(&info); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                             MAB_MAS_ERR_MDS_PWE_QUERY_FAILED, status); 
        m_MAB_DBG_TRACE("\nmas_amf_csi_vdest_role_change():left.");
        return status; 
    }

    /* do we need to do the role change of the VDEST */ 
    if (info.info.query_pwe.info.virt_info.o_role != haState)
    {
        /* log that there is a difference in the state that VDEST is created and
         * the AMF driven state
         */ 
        /* update the role of the VDEST as per AMF given state */ 
        m_NCS_MEMSET(&vda_info, 0, sizeof(vda_info));

        vda_info.req = NCSVDA_VDEST_CHG_ROLE;
        vda_info.info.vdest_chg_role.i_vdest = MAS_VDEST_ID;

        /* change the VDEST role */ 
        vda_info.info.vdest_chg_role.i_new_role = haState;
        status = ncsvda_api(&vda_info);
        if (status != NCSCC_RC_SUCCESS)
        {
            m_LOG_MAB_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                                 MAB_MAS_ERR_MDS_VDEST_ROLE_CHG_FAILED, status); 
            m_MAB_DBG_TRACE("\nmas_amf_csi_vdest_role_change():left.");
            return status; 
        }
    }

#if (NCS_MAS_RED == 1)
    /* move the pending message into the mailbox, only when state change is to Active */ 
    if ((haState == NCS_APP_AMF_HA_STATE_ACTIVE) && (inst->red.process_msg != NULL))
    {
        status = m_MAS_SND_MSG(inst->mbx, inst->red.process_msg); 
        if (status == NCSCC_RC_FAILURE)
        {
            m_MMGR_FREE_MAB_MSG(inst->red.process_msg);
            return NCSCC_RC_FAILURE; 
        }
        inst->red.process_msg = NULL; 
    }
#endif

    m_MAB_DBG_TRACE("\nmas_amf_csi_vdest_role_change():left.");
    return NCSCC_RC_SUCCESS; 
}

 /****************************************************************************\
 *  Name:          mas_amf_csi_list_find_envid                                * 
 *                                                                            *
 *  Description:   Search the global CSI list by the Environment ID           *
 *                                                                            *
 *  Arguments:     PW_ENV_ID - Env id to be found in the list                 * 
 *                                                                            * 
 *  Returns:       MAS_CSI_NODE* / NULL                                       *
 *  NOTE:                                                                     * 
 \****************************************************************************/
static MAS_CSI_NODE * 
mas_amf_csi_list_find_envid(PW_ENV_ID env_id)
{
    MAS_CSI_NODE *csi_node = NULL; 

    for (csi_node=gl_mas_amf_attribs.csi_list; csi_node; csi_node=csi_node->next)
    {
        if (csi_node->csi_info.env_id == env_id)
            break; 
    }
    
    return csi_node; 
}

 /****************************************************************************\
 *  Name:          mas_amf_csi_list_find_csiname                              * 
 *                                                                            *
 *  Description:   Search the global CSI list by the CSI Name                 *
 *                                                                            *
 *  Arguments:     SaNameT - CSI name to be found in the list                 * 
 *                                                                            * 
 *  Returns:       MAS_CSI_NODE* / NULL                                       *
 *  NOTE:                                                                     * 
 \****************************************************************************/
static MAS_CSI_NODE * 
mas_amf_csi_list_find_csiname(const SaNameT *find_me) 
{
    SaNameT         t_find_me; 
    MAS_CSI_NODE    *me = NULL; 

    m_NCS_MEMSET(&t_find_me, 0, sizeof(SaNameT)); 
    memcpy(&t_find_me, find_me, sizeof(SaNameT)); 

    me = gl_mas_amf_attribs.csi_list; 
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
*  Name:          mas_amf_csi_list_delink                                    * 
*                                                                            *
*  Description:   Delink this CSI details from global CSI list               *
*                                                                            *
*  Arguments:     SaNameT - CSI name to be found in the list                 * 
*                                                                            * 
*  Returns:       MAS_CSI_NODE* / NULL                                       *
*  NOTE:                                                                     * 
\****************************************************************************/
static MAS_CSI_NODE* 
mas_amf_csi_list_delink(const SaNameT  *csiName) 
{
    SaNameT         t_del_me; 
    MAS_CSI_NODE     *del_me = NULL; 
    MAS_CSI_NODE     *prev_csi = NULL; 

    m_NCS_MEMSET(&t_del_me, 0, sizeof(SaNameT)); 
    memcpy(&t_del_me, csiName, sizeof(SaNameT)); 

    del_me = gl_mas_amf_attribs.csi_list;
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
        gl_mas_amf_attribs.csi_list = del_me->next; 
    }
    else
    {
        prev_csi->next = del_me->next; 
    }

    return del_me; 
}

/****************************************************************************\
*  Name:          mas_amf_csi_list_add_front                                 * 
*                                                                            *
*  Description:   Add this CSI information to the global CSI list            *
*                                                                            *
*  Arguments:     MAS_CSI_NODE - CSI details to be added                     * 
*                                                                            * 
*  Returns:       Nothing                                                    *
*  NOTE:                                                                     * 
\****************************************************************************/
void 
mas_amf_csi_list_add_front(MAS_CSI_NODE *new_csi) 
{
    /* validate the input */ 
    if (new_csi == NULL)
        return; 
        
    /* add in the front */ 
    new_csi->next = gl_mas_amf_attribs.csi_list;
    gl_mas_amf_attribs.csi_list = new_csi; 
    return; 
}

 /****************************************************************************\
 *  Name:          mas_amf_csi_attrib_decode                                  * 
 *                                                                            *
 *  Description:   Decodes the env-id from CSI attributes                     *
 *                                                                            *
 *  Arguments:     SaAmfCSIAttributeListT csiAttr - Details about the         * 
 *                                                  workload being assigned   *
 *                                                                            * 
 *  Returns:       Env Id - Successful decoding                               *
 *                 zero   - in case of failure                                *
 *  NOTE:                                                                     * 
 \****************************************************************************/
static PW_ENV_ID
mas_amf_csi_attrib_decode(SaAmfCSIAttributeListT csiAttr)
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
        m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_ERROR, NCSFL_LC_HEADLINE, 
                                MAB_MAS_ERR_AMF_CSI_ATTRIBS_NULL);
        return 0; 
    }

    /* make sure that attribute list is not empty */
    if ((csiAttr.attr->attrName == NULL)|| (csiAttr.attr->attrValue == NULL))
    {
        /* log that CSI Attribute Name/Value is empty */ 
        m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_ERROR, NCSFL_LC_HEADLINE, 
                                MAB_MAS_ERR_AMF_CSI_ATTS_NAME_OR_ATTRIB_VAL_NULL);
        return 0; 
    }

    if (strcmp(csiAttr.attr->attrName, "Env-Id") != 0)
    {
        /* log that CSI Attribute Name/Value is empty */ 
        m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_ERROR, NCSFL_LC_HEADLINE, 
                                MAB_MAS_ERR_AMF_CSI_ATTRIBS_NAME_INVALID);
        return 0; 
    }

    /* good amount of input validation, now get to the work */ 
    /* log the received attribute and value pair */ 
    sprintf(attrName, "%s:%s\n", "mas_amf_csi_attrib_decode():attribute name", 
            csiAttr.attr->attrName); 
    sprintf(attrValue, "%s:%s\n", "mas_amf_csi_attrib_decode():attribute value", 
            csiAttr.attr->attrValue); 
    m_LOG_MAB_NO_CB(attrName); 
    m_LOG_MAB_NO_CB(attrValue); 

    num_args = sscanf(csiAttr.attr->attrValue, "%d", &decoded_env);
    if (num_args == 0)
    {
        m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_ERROR, NCSFL_LC_HEADLINE, 
                                MAB_MAS_ERR_AMF_CSI_SSCANF_FAILED);
        return 0; 
    }

    /* validate the given value */ 
    if ((decoded_env <= 0) || (decoded_env > NCSMDS_MAX_PWES))
    { 
        /* log the error */ 
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAS_ERR_AMF_CSI_DECODED_ENV_ID_OUTOF_REACH, 
                          decoded_env);
        return 0; 
    }

    /* log the decoded env-id */ 
    m_LOG_MAB_HDLN_I(NCSFL_SEV_INFO, MAB_HDLN_MAS_AMF_CSI_ATTRIB_DECODE_SUCCESS, decoded_env);
   
    /* got a valid environment id */  
    return ((PW_ENV_ID)decoded_env); 
}

/*****************************************************************************\
 *  Name:          mas_amf_sigusr1_handler                                    * 
 *                                                                            *
 *  Description:   to post a message to MAS mailbox to register with AMF      *
 *                                                                            *
 *  Arguments:     int i_sug_num -  Signal received                           *
 *                                                                            * 
 *  Returns:       Nothing                                                    *  
 *  NOTE:                                                                     * 
\*****************************************************************************/
/* signal from AMF, time to register with AMF */ 
void  
mas_amf_sigusr1_handler(int i_sig_num)
{
    /* ignore the signal */ 
    m_NCS_SIGNAL(SIGUSR1, SIG_IGN); 

    m_NCS_SEL_OBJ_IND(gl_mas_amf_attribs.amf_attribs.sighdlr_sel_obj);

    return; 
}

/*****************************************************************************\
 *  Name:          mas_process_sig_usr1_signal                                * 
 *                                                                            *
 *  Description:   Processes USR1 signal. Calls mas_amf_componentize function *
 *                                                                            *
 *  Arguments:     void                                                       *
 *                                                                            * 
 *  Returns:       void                                                       *  
 *  NOTE:                                                                     * 
\*****************************************************************************/
static void mas_process_sig_usr1_signal(void)
{
    char    comp_name[256] = {0}; 
    FILE    *fp = NULL; 

    /* open the file to read the component name */
    fp = fopen(m_MAS_COMP_NAME_FILE, "r");/* OSAF_LOCALSTATEDIR/ncs_mas_comp_name */
    if (fp == NULL)
    {
        /* log that, there is no component name file */
        return;
    }

    if(fscanf(fp, "%s", comp_name) != 1)
    {
        fclose(fp);
        return;
    }

    if (setenv("SA_AMF_COMPONENT_NAME", comp_name, 1) == -1)
    {
        fclose(fp);
        return;
    }

    fclose(fp);

    mas_amf_componentize(&gl_mas_amf_attribs.amf_attribs);
    return; 
}

/*****************************************************************************\
 *  Name:          mas_amf_initialize                                         * 
 *                                                                            *
 *  Description:   Initialises the interface with AMF.                        *
 *                                                                            *
 *  Arguments:     NCS_APP_AMF_ATTRIBS* amf_attribs                           *
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS on success                                *  
 *                 NCSCC_RC_FAILURE on failure                                *
 *  NOTE:                                                                     * 
\*****************************************************************************/
uns32 mas_amf_initialize(NCS_APP_AMF_ATTRIBS* amf_attribs)
{
    uns32          status = NCSCC_RC_SUCCESS; 
    SaAisErrorT    saf_status = SA_AIS_OK; 

    /* validate the inputs */ 
    if (amf_attribs == NULL)
        return NCSCC_RC_FAILURE; 

   /* Initialise the interface with AMF again. Start health check if initialisation is success. */
   saf_status = ncs_app_amf_initialize(amf_attribs);
   if(saf_status != SA_AIS_OK)
   {
       /* log the failure */
       m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR,
                         MAB_MAS_ERR_AMF_INITIALIZE_FAILED, saf_status);
       if(saf_status == SA_AIS_ERR_TRY_AGAIN)
       {
          /* set the message type. */
          amf_attribs->amf_init_timer.msg_type = MAB_MAS_AMF_INIT_RETRY;

          /* start the timer to retry amf interface initialisation */
          status = ncs_app_amf_init_start_timer(&amf_attribs->amf_init_timer);
          if(status == NCSCC_RC_FAILURE)
          {
             /* Timer couldn't be started. Log the error. */
             m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR,
                         MAB_MAS_ERR_AMF_TIMER_FAILED, status);

          }
          return status;
       }
       return NCSCC_RC_FAILURE;
   }

   /* log that AMF componentization is done */
   m_LOG_MAB_HEADLINE(NCSFL_SEV_NOTICE, MAB_HDLN_MAS_AMF_INITIALIZE_SUCCESS);

   /*  If amf interface initialisation is succeeded on retry, 
       timer would have been created. Destroy it. */
   if(amf_attribs->amf_init_timer.timer_id != TMR_T_NULL)
   {
      m_NCS_TMR_DESTROY(amf_attribs->amf_init_timer.timer_id);
      amf_attribs->amf_init_timer.timer_id = TMR_T_NULL;
   }

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
*  Name:          mas_table_rec_list_free                                    * 
*                                                                            *
*  Description:   Frees the list of tables and associated filters.           * 
*                                                                            *
*  Arguments:     MAS_ROW_REC *tbl_list - List of tables                     * 
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Freed the list successfully             *
\****************************************************************************/
uns32
mas_table_rec_list_free(MAS_ROW_REC *tbl_list) 
{
    NCS_BOOL        is_next_tbl_set = FALSE; 
    MAS_FLTR        *fltr, *del_fltr; 
    MAS_ROW_REC     *tbl_rec, *del_tbl_rec; 
    MAB_FLTR_ANCHOR_NODE    *nxt_fltr_id, *current; 
    
    for(tbl_rec = tbl_list; tbl_rec != NULL; is_next_tbl_set = FALSE)
    {
        /* if there is a default filter for this table, free it */ 
        if (tbl_rec->dfltr_regd == TRUE)
        {
            /* free the fltr and anchor list */ 
            if (tbl_rec->dfltr.fltr_ids.active_fltr != NULL)
            {
                m_MMGR_FREE_FLTR_ANCHOR_NODE(tbl_rec->dfltr.fltr_ids.active_fltr); 
                tbl_rec->dfltr.fltr_ids.active_fltr = NULL;
            }

            /* free the fltr-id list */
            current = tbl_rec->dfltr.fltr_ids.fltr_id_list;
            while (current)
            {
                nxt_fltr_id = current->next;
                m_MMGR_FREE_FLTR_ANCHOR_NODE(current);
                current = nxt_fltr_id;
            }
            tbl_rec->dfltr.fltr_ids.fltr_id_list = NULL;
        } /* end for default filter cleanup */ 

        for(fltr = tbl_rec->fltr_list;fltr != NULL;)
        {
            del_fltr = fltr;
            fltr = fltr->next;
            m_LOG_MAB_FLTR_DETAILS(NCSFL_LC_DATA, NCSFL_SEV_DEBUG,
                                     MAB_MAS_FLTR_RMV_SUCCESS, tbl_rec->tbl_id, 
                                     0, del_fltr->fltr.type); 
            /* free this filter */ 
            m_FREE_MAS_FLTR(del_fltr); 

            /* there are no more fitlers in this table */ 
            if(fltr == NULL)
            {
                /* free this tbale */ 
                del_tbl_rec = tbl_rec;
                tbl_rec = tbl_rec->next;

                is_next_tbl_set = TRUE;

                m_MMGR_FREE_MAS_ROW_REC(del_tbl_rec);
            } 

        } /* for(fltr = tbl_rec->fltr_list;fltr != NULL;) */

        if(is_next_tbl_set == FALSE)
            tbl_rec = tbl_rec->next;

    } /* for all the tables in this list */ 
    return NCSCC_RC_SUCCESS; 
}

/****************************************************************************\
*  Name:          mas_amf_mds_quiesced_process                               * 
*                                                                            *
*  Description:   Responds back to the AMF with the successful state         * 
*                 transistion to QUIESCED state.                             *
*                                                                            *
*  Arguments:     NCSMDS_CALLBACK_INFO *cbinfo - Information from MDS        * 
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Message was posted successfully         *
*                 Nothing - to the caller of this function                   *
\****************************************************************************/
uns32  mas_amf_mds_quiesced_process(NCSCONTEXT mas_hdl) 
{
    MAS_TBL *inst; 
    
    inst = (MAS_TBL*)m_MAS_VALIDATE_HDL(NCS_PTR_TO_INT32_CAST(mas_hdl));
    if(inst == NULL)
    {
        m_LOG_MAB_NO_CB("mas_amf_mds_quiesced_process()"); 
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);            
    }

    /* send the AMF response */ 
    saAmfResponse(gl_mas_amf_attribs.amf_attribs.amfHandle, 
                  inst->amf_invocation_id, SA_AIS_OK); 
    inst->amf_invocation_id = 0;
    
    ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(mas_hdl));
    return NCSCC_RC_SUCCESS; 
}
/****************************************************************************\
*  Name:          mas_amf_sigusr2_handler                                    * 
*                                                                            *
*  Description:   to print the MAS data structures into a file in /tmp       *
*                                                                            *
*  Arguments:     int i_sug_num -  Signal received                           *
*                                                                            * 
*  Returns:       Nothing                                                    *  
*  NOTE:                                                                     * 
\****************************************************************************/
static void  
mas_amf_sigusr2_handler(int i_sig_num)
{
    /* dump the data structure contents */ 
    mas_dictionary_dump(); 

    return; 
}

#endif

