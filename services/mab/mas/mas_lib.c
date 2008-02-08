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

  $Header: /ncs/software/release/UltraLinq/MAB/MAB1.0/base/products/mab/mas/mas_lib.c
  
    
      DESCRIPTION:
      
        
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "mab.h"
#include "ncs_mda_pvt.h"

#if (NCS_MAB == 1)

NCS_BOOL      gl_inited = FALSE;

/* have a global variable to hold the AMF attributes */ 
MAS_ATTRIBS  gl_mas_amf_attribs; 

/****************************************************************************\
*  Name:          mas_evt_cb                                                 * 
*                                                                            *
*  Description:   Callback to report a Notification to the creator?          *
*                                                                            *
*  Arguments:     MAB_LM_EVT *evt - Data to be notified                      *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
*                 NCSCC_RC_FAILURE   -  failure                              *
*  NOTE:                                                                     * 
\****************************************************************************/
uns32 mas_evt_cb(MAB_LM_EVT* evt)
{
    USE(evt);
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
*  Name:          maslib_request                                             * 
*                                                                            *
*  Description:   SE API for MAS                                             *
*                                                                            *
*  Arguments:     NCS_LIB_REQ_INFO * - Input for MAS                         *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
*                 NCSCC_RC_FAILURE   -  failure                              *
*  NOTE:                                                                     * 
\****************************************************************************/
uns32 maslib_request(NCS_LIB_REQ_INFO * req_info)
{
    MAB_MSG *post_me; 
    uns32 status;
    FILE *fp = NULL;

    /* validate the input */ 
    if (req_info == NULL)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        
    /* check on the type of the request */ 
    switch (req_info->i_op)
    {
        case NCS_LIB_REQ_CREATE:
        {
            /* check whether MAS is created already */ 
            if (gl_inited == TRUE)
                return NCSCC_RC_SUCCESS;
                
            /* open the file to read the component name */
            fp = sysf_fopen(m_MAS_PID_FILE, "w");/* /var/run/mas_pid.txt */
            if (fp == NULL)
            {
               return NCSCC_RC_FAILURE;
            }

            status = getpid();

            status = sysf_fprintf(fp, "%d", status);
            if(status < 1)
            {
                fclose(fp);
                return NCSCC_RC_FAILURE;
            }

            sysf_fclose(fp);

            /* reset the AMF attributes */ 
            m_NCS_MEMSET(&gl_mas_amf_attribs, 0,sizeof(MAS_ATTRIBS)); 
             
            /* Bind with DTA */ 
            if (mab_log_bind() != NCSCC_RC_SUCCESS)
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

            /* log the MASv version */
            m_LOG_MAB_HDLN_I(NCSFL_SEV_NOTICE, MAB_HDLN_MAS_VERSION,  MAS_MDS_SUB_PART_VERSION); 

            if (m_NCS_IPC_CREATE(&gl_mas_amf_attribs.mbx_details.mas_mbx) != NCSCC_RC_SUCCESS)
            {
                /* log that there is a failure in IPC creation for MAS */
                m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_CRITICAL, 
                                        NCSFL_LC_SVC_PRVDR,
                                        MAB_MAS_ERR_IPC_CREATE_FAILED); 
                /* unbind with DTS */ 
                mab_log_unbind(); 
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            }

            if (m_NCS_IPC_ATTACH(&gl_mas_amf_attribs.mbx_details.mas_mbx) != NCSCC_RC_SUCCESS)
            {
                m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_CRITICAL, 
                                        NCSFL_LC_SVC_PRVDR,
                                        MAB_MAS_ERR_IPC_ATTACH_FAILED); 
                mab_log_unbind(); 
                m_NCS_IPC_DETACH(&gl_mas_amf_attribs.mbx_details.mas_mbx, NULL, NULL);
                m_NCS_IPC_RELEASE(&gl_mas_amf_attribs.mbx_details.mas_mbx, NULL);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            }

            /* Store the mailbox pointer in Retry amf-initialisation timer structure */
            gl_mas_amf_attribs.amf_attribs.amf_init_timer.mbx = &gl_mas_amf_attribs.mbx_details.mas_mbx;

             /* initialize the signal handler */ 
             if ((ncs_app_signal_install(SIGUSR1,mas_amf_sigusr1_handler)) == -1) 
             {
                m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_CRITICAL, 
                                        NCSFL_LC_FUNC_RET_FAIL,
                                        MAB_MAS_ERR_SIG_INSTALL_FAILED); 
                 mab_log_unbind(); 
                 m_NCS_IPC_DETACH(&gl_mas_amf_attribs.mbx_details.mas_mbx, NULL, NULL);
                 m_NCS_IPC_RELEASE(&gl_mas_amf_attribs.mbx_details.mas_mbx, NULL);
                 return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
             }

#if (NCS_MAS_RED == 1)
             /* initialize the MBCSv Interface */ 
             gl_mas_amf_attribs.mbcsv_attribs.masv_version = (uns16)m_NCS_MAS_MBCSV_VERSION; 
             status = mas_mbcsv_interface_initialize(&gl_mas_amf_attribs.mbcsv_attribs); 
             if (status != NCSCC_RC_SUCCESS)
             {
                 /* log the failure */ 
                 mab_log_unbind(); 
                 m_NCS_IPC_DETACH(&gl_mas_amf_attribs.mbx_details.mas_mbx, NULL, NULL); 
                 m_NCS_IPC_RELEASE(&gl_mas_amf_attribs.mbx_details.mas_mbx, NULL);
                 return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
             }
#if (NCS_MAS_RED_UNIT_TEST == 1)
             verify_mas_fltr_ids_enc_dec();
#endif
#endif

             /* create the selection object associated with SIGUSR1 signal handler */
             m_NCS_SEL_OBJ_CREATE(&gl_mas_amf_attribs.amf_attribs.sighdlr_sel_obj);

            
            /* create the VDEST of MAS */ 
            {
                NCSMAS_LM_ARG arg;
                NCSVDA_INFO vda_info;

                /* prepare the cb with the vaddress */
                m_NCS_MEMSET(&arg, 0, sizeof(arg));
                m_NCS_MEMSET(&vda_info, 0, sizeof(vda_info));

                vda_info.req = NCSVDA_VDEST_CREATE;
                vda_info.info.vdest_create.i_persistent = FALSE;
                vda_info.info.vdest_create.i_policy = NCS_VDEST_TYPE_DEFAULT;
                vda_info.info.vdest_create.i_create_oac = FALSE;
                vda_info.info.vdest_create.i_create_type = NCSVDA_VDEST_CREATE_SPECIFIC;
                vda_info.info.vdest_create.info.specified.i_vdest = MAS_VDEST_ID;
                /* create Vdest address */
                if (ncsvda_api(&vda_info) != NCSCC_RC_SUCCESS)
                {
                    /* log that VDEST creation failed */ 
                    m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_CRITICAL, 
                                        NCSFL_LC_SVC_PRVDR,
                                        MAB_MAS_ERR_VDEST_CREATE_FAILED); 
                    mab_log_unbind(); 
                    m_NCS_IPC_DETACH(&gl_mas_amf_attribs.mbx_details.mas_mbx, NULL, NULL); 
                    m_NCS_IPC_RELEASE(&gl_mas_amf_attribs.mbx_details.mas_mbx, NULL);
                    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                }

                arg.info.create.i_mds_hdl = vda_info.info.vdest_create.o_mds_pwe1_hdl;
                gl_mas_amf_attribs.mas_mds_def_pwe_hdl = vda_info.info.vdest_create.o_mds_pwe1_hdl;
                gl_mas_amf_attribs.mas_mds_hdl = vda_info.info.vdest_create.o_mds_vdest_hdl; 

                /* Create the darn thing */
                arg.i_op = NCSMAS_LM_OP_CREATE;

                arg.info.create.i_vrid = 1;
                arg.info.create.i_mbx = &gl_mas_amf_attribs.mbx_details.mas_mbx;
                arg.info.create.i_lm_cbfnc = (MAB_LM_CB) mas_evt_cb;
                arg.info.create.i_hm_poolid = 0;
                if (ncsmas_lm( &arg ) != NCSCC_RC_SUCCESS)
                {
                    /* log the error */ 
                    m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_CRITICAL, 
                                        NCSFL_LC_FUNC_RET_FAIL,
                                        MAB_MAS_ERR_LM_CREATE_FAILED); 

                    /* cleanup */ 
                    mab_log_unbind(); 
                    m_NCS_IPC_DETACH(&gl_mas_amf_attribs.mbx_details.mas_mbx, NULL, NULL); 
                    m_NCS_IPC_RELEASE(&gl_mas_amf_attribs.mbx_details.mas_mbx, NULL);
                    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                }
            }

            /* create MAS task */  
            if (m_NCS_TASK_CREATE ((NCS_OS_CB)mas_mbx_amf_process,
                &gl_mas_amf_attribs.mbx_details.mas_mbx,
                NCS_MAB_MAS_TASKNAME,
                NCS_MAB_MAS_PRIORITY,
                NCS_MAB_MAS_STACKSIZE,
                &gl_mas_amf_attribs.mbx_details.mas_mbx_hdl) != NCSCC_RC_SUCCESS)
            {
                /* log the error */ 
                m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_CRITICAL, 
                                        NCSFL_LC_SVC_PRVDR,
                                        MAB_MAS_ERR_TASK_CREATE_FAILED); 
                mab_log_unbind(); 
                m_NCS_IPC_DETACH(&gl_mas_amf_attribs.mbx_details.mas_mbx, NULL, NULL);
                m_NCS_IPC_RELEASE(&gl_mas_amf_attribs.mbx_details.mas_mbx, NULL);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            }
            
            /* start the task */ 
            if (m_NCS_TASK_START (gl_mas_amf_attribs.mbx_details.mas_mbx_hdl) != NCSCC_RC_SUCCESS)
            { 
                /* log the error */ 
                m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_CRITICAL, 
                                        NCSFL_LC_SVC_PRVDR,
                                        MAB_MAS_ERR_TASK_START_FAILED); 
                mab_log_unbind(); 
                m_NCS_TASK_RELEASE(gl_mas_amf_attribs.mbx_details.mas_mbx_hdl);
                m_NCS_IPC_DETACH(&gl_mas_amf_attribs.mbx_details.mas_mbx, NULL, NULL);
                m_NCS_IPC_RELEASE(&gl_mas_amf_attribs.mbx_details.mas_mbx, NULL);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            }            
            gl_inited = TRUE;

            /* log that MASv is created successfully */ 
            m_LOG_MAB_HEADLINE(NCSFL_SEV_INFO, MAB_HDLN_MAS_CREATE_SUCCESS); 

            return NCSCC_RC_SUCCESS;
        }

        /* destroy MAS */ 
        case NCS_LIB_REQ_DESTROY:
        {
            if (gl_inited == FALSE)
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

            post_me = m_MMGR_ALLOC_MAB_MSG;
            if(post_me == NULL)
            {
               m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_MABMSG_CREATE,
                                 "maslib_request(): NCS_LIB_REQ_DESTROY"); 
               return m_MAB_DBG_SINK(NCSCC_RC_OUT_OF_MEM);
            }
            m_NCS_MEMSET(post_me, 0, sizeof(MAB_MSG));
            post_me->op = MAB_MAS_DESTROY;

            /* post a message to MAS's thread */
            status = m_NCS_IPC_SEND(&gl_mas_amf_attribs.mbx_details.mas_mbx,
                                    (NCS_IPC_MSG *)post_me, NCS_IPC_PRIORITY_HIGH);
            if (status != NCSCC_RC_SUCCESS)
            {
                m_MMGR_FREE_MAB_MSG(post_me); /* fix for the bug 60582 */
                return m_MAB_DBG_SINK(status);
            }
            
            return NCSCC_RC_SUCCESS;

        }

        /* for any other Junk requests */
        default:
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    } /* end of switch */ 

    return NCSCC_RC_SUCCESS;
}

/****************************************************************************\
*  Name:          maslib_mas_destroy                                         * 
*                                                                            *
*  Description:   Destroys MAS, releases IPC and Thread.                     *
*                                                                            *
*  Arguments:     None                                                       *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS   - everything is OK                      *
*                 NCSCC_RC_FAILURE   -  failure                              *
*  NOTE:                                                                     * 
\****************************************************************************/
uns32 maslib_mas_destroy(void)
{
    NCSMAS_LM_ARG arg;

    /* Destroy the instance */
    m_NCS_MEMSET(&arg, 0, sizeof(NCSMAS_LM_ARG)); 
    arg.i_op      = NCSMAS_LM_OP_DESTROY;
    if (ncsmas_lm(&arg) != NCSCC_RC_SUCCESS)
    {
       /* log the error */ 
       m_LOG_MAB_ERROR_NO_DATA(NCSFL_SEV_CRITICAL, 
                                        NCSFL_LC_FUNC_RET_FAIL,
                                        MAB_MAS_ERR_LM_DESTROY_FAILED); 

       /* go ahead and so the other cleanup */ 
    }
                
    /* Detach the IPC. */
    m_NCS_IPC_DETACH(&gl_mas_amf_attribs.mbx_details.mas_mbx, NULL, NULL);

    /* release the IPC */
    m_NCS_IPC_RELEASE(&gl_mas_amf_attribs.mbx_details.mas_mbx, mab_leave_on_queue_cb);

    /* log that MAS destory is success */ 
    m_LOG_MAB_HEADLINE(NCSFL_SEV_INFO, MAB_HDLN_MAS_DESTROY_SUCCESS); 

    /* UnBind with DTA */ 
    mab_log_unbind(); 

    m_NCS_TASK_STOP(gl_mas_amf_attribs.mbx_details.mas_mbx_hdl);
    m_NCS_TASK_RELEASE(gl_mas_amf_attribs.mbx_details.mas_mbx_hdl);

    /* reset the AMF and other attributes */ 
    m_NCS_MEMSET(&gl_mas_amf_attribs, 0,sizeof(MAS_ATTRIBS)); 
    gl_inited = FALSE;
    
    return NCSCC_RC_SUCCESS;
}

#endif /* NCS_MAB == 1 */

