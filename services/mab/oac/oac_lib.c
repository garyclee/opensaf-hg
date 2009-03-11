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
..............................................................................

  DESCRIPTION: This file contains the Single Entry APIs ro create, destroy and
               instantiate OAA in a particular environment. 
******************************************************************************
*/

#include "mab.h"

#if (NCS_MAB == 1)

MABOAC_API uns32       gl_oac_handle;

static  NCS_SEL_OBJ oac_sync_sel; 


typedef struct oacmbx_struct
{
    SYSF_MBX   oac_mbx;
    void *     oac_mbx_hdl;
} OACMBX_STRUCT;

static OACMBX_STRUCT gl_oacmbx;
static NCS_BOOL      gl_oac_inited = FALSE;

/* global list of all OAA instances */ 
static MAB_INST_NODE    *gl_oac_inst_list; 

/* to register with SPLR */ 
static uns32
oaclib_oac_create(NCS_LIB_REQ_INFO * req_info); 

/* to create the mailbox and thread for OAA */ 
static uns32
oaclib_oac_create_ipc_task(); 

/* instantiate OAA in a particular PWE */ 
static uns32
oaclib_oac_instantiate(NCS_LIB_REQ_INFO * req_info);

/* un-instantiate OAA in a particular PWE */ 
static uns32    
oaclib_oac_uninstantiate(PW_ENV_ID      i_env_id, 
                         SaNameT        i_inst_name, 
                         uns32          i_oac_hdl); 

/* to add to the global list of instances */
static uns32 
oac_inst_list_add(PW_ENV_ID env_id, uns32 i_oac_hdl, SaNameT i_inst_name);

/* to delete and release a particular OAA instance from the list */ 
static void 
oac_inst_list_release(PW_ENV_ID env_id); 

static uns32 sysf_ake_lm_cbfn(MAB_LM_EVT* evt)
{
       return NCSCC_RC_SUCCESS; 
}

/*****************************************************************************\
 *  Name:          oaclib_request                                             * 
 *                                                                            *
 *  Description:   Single Entry point for OAA component of MASv               *
 *                                                                            *
 *  Arguments:     NCS_LIB_REQ_INFO* - input arguments                        * 
 *                                                                            * 
 *  Returns:       NCSCC_RC_SUCCESS - Everything went well                    * 
 *                 NCSCC_RC_FAILURE = Something went wrong                    *
 *  NOTE:                                                                     * 
\*****************************************************************************/
uns32 oaclib_request(NCS_LIB_REQ_INFO * req_info)
{
    uns32       status = NCSCC_RC_FAILURE; 
    NCS_SEL_OBJ_SET     set; 
    uns32 timeout = 1000;   

    /* make sure there is somedata */ 
    if (req_info == NULL)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

    /* see what can we do for the caller */ 
    switch (req_info->i_op)
    {
        /* create OAA */ 
        case NCS_LIB_REQ_CREATE:
            status = oaclib_oac_create(req_info);
            if (status != NCSCC_RC_SUCCESS)
                return m_MAB_DBG_SINK(status);
        break;

        /* Instantiate OAA in a paerticulr PWE/Environment */
        case NCS_LIB_REQ_INSTANTIATE:
            status = oaclib_oac_instantiate(req_info); 
        break;

        /* uninstantiate a particular instance of OAA */ 
        case NCS_LIB_REQ_UNINSTANTIATE:
            status = oaclib_oac_uninstantiate(req_info->info.uninst.i_env_id, 
                                              req_info->info.uninst.i_inst_name, 
                                              req_info->info.uninst.i_inst_hdl); 
        break; 

        /* destroy OAA (destroys all the instances of OAA) */ 
        case NCS_LIB_REQ_DESTROY:
            if (gl_oacmbx.oac_mbx_hdl == NULL)
            {
               NCS_SPLR_REQ_INFO splr_info;

               /* deregister with SPLR */ 
               m_NCS_OS_MEMSET(&splr_info, 0, sizeof(NCS_SPLR_REQ_INFO)); 
               splr_info.type = NCS_SPLR_REQ_DEREG; 
               splr_info.i_sp_abstract_name = m_OAA_SP_ABST_NAME;
               status = ncs_splr_api(&splr_info); 
               if (status != NCSCC_RC_SUCCESS)
               {
                 /* log the failure */ 
                 m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR, 
                                 MAB_OAC_ERR_SPLR_DEREG_FAILED, status, 0);
                 return status; 
               }
            }
            else
            {
                MAB_MSG     *post_me; 

                post_me = m_MMGR_ALLOC_MAB_MSG;
                if(post_me == NULL)
                {
                    return m_MAB_DBG_SINK(NCSCC_RC_OUT_OF_MEM);
                }
                m_NCS_OS_MEMSET(post_me, 0, sizeof(MAB_MSG));
                post_me->op = MAB_OAC_DESTROY;

                m_NCS_SEL_OBJ_CREATE(&oac_sync_sel);
                m_NCS_SEL_OBJ_ZERO(&set);
                m_NCS_SEL_OBJ_SET(oac_sync_sel, &set);

                /* post a message to MAC's thread */
                status = m_NCS_IPC_SEND(&gl_oacmbx.oac_mbx, (NCS_IPC_MSG *)post_me, NCS_IPC_PRIORITY_HIGH);
                if (status != NCSCC_RC_SUCCESS)
                {
                    m_MMGR_FREE_MAB_MSG(post_me); 
                    return m_MAB_DBG_SINK(status);
                }
                m_NCS_SEL_OBJ_SELECT(oac_sync_sel, &set, 0, 0, &timeout);
                m_NCS_SEL_OBJ_DESTROY(oac_sync_sel);
                m_NCS_OS_MEMSET(&oac_sync_sel, 0, sizeof(oac_sync_sel));
            }
        break;
       
        /* OAA can not help with this type of request */  
        default:
            return m_MAB_DBG_SINK(status);
        break; 

    }

    /* time to report the status to the caller */ 
    return status;
}

/****************************************************************************\
*  Name:          oaclib_oac_create                                          * 
*                                                                            *
*  Description:   to create OAA                                              *
*                  - Register with SPLR                                      *
*                                                                            *
*  Arguments:     NCS_LIB_REQ_INFO  - input parameters                       * 
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - something went wrong                    * 
\****************************************************************************/
static uns32
oaclib_oac_create(NCS_LIB_REQ_INFO * req_info)
{
    uns32             status; 
    NCS_SPLR_REQ_INFO splr_info;

    /* OAA is installed already */         
    if (gl_oac_inited == TRUE)
        return NCSCC_RC_SUCCESS;

    /* register with DTS */ 
    status = mab_log_bind(); 
    if (status != NCSCC_RC_SUCCESS)
        return status; 

    /* register with SPLR data base */ 
    /* SPLR: Service Provider Library Registry */ 
    m_NCS_OS_MEMSET(&splr_info, 0, sizeof(NCS_SPLR_REQ_INFO)); 
    splr_info.type = NCS_SPLR_REQ_REG; 
    splr_info.i_sp_abstract_name = m_OAA_SP_ABST_NAME;
    splr_info.info.reg.instantiation_flags = 
        NCS_SPLR_INSTANTIATION_PER_INST_NAME | 
        NCS_SPLR_INSTANTIATION_PER_ENV_ID; 
    splr_info.info.reg.instantiation_api = oaclib_request; 
    status = ncs_splr_api(&splr_info); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR, 
                        MAB_OAC_ERR_SPLR_REG_FAILED, status, 0);
        /* unbind with DTS */
        mab_log_unbind(); 
        return status; 
    }

    gl_oac_inited = TRUE; 

    /* log the OAC version */
    m_LOG_MAB_HDLN_I(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_CREATE_SUCCESS, OAC_MDS_SUB_PART_VERSION);

    return status; 
}

/****************************************************************************\
*  Name:          oaclib_oac_create_ipc_task                                 * 
*                                                                            *
*  Description:   Creates mailbox and thread for OAA                         *
*                                                                            *
*  Arguments:     
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
*  NOTE:          'gl_oacmbx' has been accessed                              * 
\****************************************************************************/
static uns32
oaclib_oac_create_ipc_task()
{
    uns32   status; 

    /* create mail-box for OAA */ 
    if(gl_oacmbx.oac_mbx == (SYSF_MBX) ((long)NULL)) /* (To avoid the sysf_ipc.c mem leak) */
    {
        status = m_NCS_IPC_CREATE(&gl_oacmbx.oac_mbx);
        if (status != NCSCC_RC_SUCCESS)
        {
            /* log the failure */ 
            m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, 
                            MAB_OAC_ERR_IPC_CREATE_FAILED, 
                            status);
           /* unbind with DTS */ 
           mab_log_unbind(); 
           return status;
        }
    }

    /* Attach the IPC */
    status = m_NCS_IPC_ATTACH(&gl_oacmbx.oac_mbx);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, 
                        MAB_OAC_ERR_IPC_ATTACH_FAILED, 
                        status);
        
        /* unbind with DTS */ 
        mab_log_unbind(); 

        /* relase the IPC */ 
        m_NCS_IPC_RELEASE(&gl_oacmbx.oac_mbx, mab_leave_on_queue_cb);
        return status;
    }

    /* create the OAA thread */ 
    status = m_NCS_TASK_CREATE ((NCS_OS_CB)oac_do_evts,
        &gl_oacmbx.oac_mbx,
        NCS_MAB_OAC_TASKNAME,
        NCS_MAB_OAC_PRIORITY,
        NCS_MAB_OAC_STACKSIZE,
        &gl_oacmbx.oac_mbx_hdl); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, 
                        MAB_OAC_ERR_TASK_CREATE_FAILED, 
                        status);

        /* unbind with DTS */ 
        mab_log_unbind(); 

        /* release the IPC */ 
        (void)m_NCS_IPC_DETACH(&gl_oacmbx.oac_mbx, NULL, NULL);
        m_NCS_IPC_RELEASE(&gl_oacmbx.oac_mbx, mab_leave_on_queue_cb);
        return status;
    }
    
    /* start the OAA thread */ 
    status = m_NCS_TASK_START (gl_oacmbx.oac_mbx_hdl);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, 
                        MAB_OAC_ERR_TASK_START_FAILED, 
                        status);

        /* unbind with DTS */ 
        mab_log_unbind(); 

        /* release the task and IPC */ 
        (void)m_NCS_IPC_DETACH(&gl_oacmbx.oac_mbx, NULL, NULL);
        m_NCS_IPC_RELEASE(&gl_oacmbx.oac_mbx, mab_leave_on_queue_cb);
        m_NCS_TASK_RELEASE(gl_oacmbx.oac_mbx_hdl);
        return status;
    }

    return status;
}

/****************************************************************************\
*  Name:          oaclib_oac_instantiate                                     * 
*                                                                            *
*  Description:   To instantiate OAA in a specific environment               *
*                                                                            *
*  Arguments:     NCS_LIB_REQ_INFO - instantiate inputs                      * 
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Everything went well                    *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
*  NOTE:          'gl_oacmbx' has been accessed                              * 
\****************************************************************************/
static uns32 
oaclib_oac_instantiate(NCS_LIB_REQ_INFO  *req_info) 
{
    uns32               status; 
    NCSOAC_LM_ARG       oac_lmarg;

    /* Initaialize OAA if it is not */ 
    if (gl_oac_inited == FALSE)
    {
        /* SPLR registration is not done */ 
        return NCSCC_RC_FAILURE; 
    }

    if (gl_oac_inst_list == NULL)
    {
        /* create thread and mailbox */ 
        status = oaclib_oac_create_ipc_task(); 
        if (status != NCSCC_RC_SUCCESS)
        {
            /* Error code is logged in oaclib_oac_create_ipc_task() */
            return status; 
        }
    }

    /* instantiate OAA in the asked PWE */
    m_NCS_OS_MEMSET(&oac_lmarg, 0, sizeof(NCSOAC_LM_ARG)); 
    oac_lmarg.i_op = NCSOAC_LM_OP_CREATE;
    /* take the env-id */ 
    oac_lmarg.info.create.i_vrid = req_info->info.inst.i_env_id; 
    /* get the instance name of the application */ 
    memcpy(&oac_lmarg.info.create.i_inst_name, 
                    &req_info->info.inst.i_inst_name, 
                    sizeof(SaNameT)); 
    oac_lmarg.info.create.i_mbx = &gl_oacmbx.oac_mbx;
    oac_lmarg.info.create.i_lm_cbfnc = sysf_ake_lm_cbfn;
    oac_lmarg.info.create.i_hm_poolid = 0;
    status = ncsoac_lm(&oac_lmarg); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_ERROR, 
                        MAB_OAC_ERR_CREATE_FAILED, 
                        status, req_info->info.inst.i_env_id);
        return status;
    }
    /* give the oac handle to the caller  */
    req_info->info.inst.o_inst_hdl = oac_lmarg.info.create.o_oac_hdl;

    /* add the Environment to the list of all the envs */ 
    status = oac_inst_list_add(req_info->info.inst.i_env_id, 
                               req_info->info.inst.o_inst_hdl, 
                               req_info->info.inst.i_inst_name);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
                        MAB_OAC_ERR_INST_LIST_ADD_FAILED, 
                        status, req_info->info.inst.i_env_id);
        return status; 
    }

    /* log that OAA has been installed successfully */ 
    m_LOG_MAB_HDLN_II(NCSFL_SEV_INFO, MAB_HDLN_OAC_INSTANTIATE_SUCCESS, 
                      req_info->info.inst.i_env_id, 
                      req_info->info.inst.o_inst_hdl); 

    /* time to report the status */ 
    return status;
}

/****************************************************************************\
*  Name:          oaclib_oac_uninstantiate                                   * 
*                                                                            *
*  Description:   To uninstatntiate OAA in a particular environment          *
*                                                                            *
*  Arguments:     PW_ENV_ID  - Environment id from where OAA has to be       *
*                              uninstantiated                                *
*                 SaNameT    - Instance name                                 *
*                 uns32 *i_oac_handle - Handle of the OAA to be uninstalled  * 
*                 NCS_BOOL spir_cleanup - Should the SPIR to be cleanedup?   *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS  - Everything went well                   *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
*  NOTE:                                                                     * 
\****************************************************************************/
static uns32    
oaclib_oac_uninstantiate(PW_ENV_ID      i_env_id, 
                         SaNameT        i_inst_name, 
                         uns32          i_oac_hdl) 
{
    uns32           status; 
    NCSOAC_LM_ARG   arg;

    /* destroy this OAA instance */ 
    m_NCS_OS_MEMSET(&arg, 0, sizeof(NCSOAC_LM_ARG)); 
    arg.i_op      = NCSOAC_LM_OP_DESTROY;
    memcpy(&arg.info.destroy.i_inst_name, 
                    &i_inst_name, 
                    sizeof(SaNameT)); 
    arg.info.destroy.i_env_id = i_env_id;
    arg.info.destroy.i_oac_hdl = i_oac_hdl;
    status = ncsoac_lm(&arg);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_ERROR, 
                        MAB_OAC_ERR_DESTROY_FAILED, 
                        status, i_env_id);
        return status; 
    }

    /* remove the node from the list of OAAs in the system */
    oac_inst_list_release(i_env_id); 

    /* log that uninstall for OAA in this PWE is successful */ 
    m_LOG_MAB_HDLN_I(NCSFL_SEV_INFO, MAB_HDLN_OAC_UNINSTANTIATE_SUCCESS,
                     i_env_id); 

    return status; 
}
/****************************************************************************\
*  Name:          oaclib_oac_destroy                                         * 
*                                                                            *
*  Description:   To Destroy all the OAA instances in this process           *
*                                                                            *
*  Arguments:     NCS_LIB_REQ_INFO * - input details                         * 
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS                                           *
*  NOTE:          Even if there is an error in uninstantiating a OAA, this   * 
*                 routine will continue to uninstall all the instances of    * 
*                 OAA in a prcoess                                           * 
\****************************************************************************/
uns32    
oaclib_oac_destroy(NCS_LIB_REQ_INFO *req_info)
{
    uns32               status = NCSCC_RC_FAILURE; 
    NCS_SPLR_REQ_INFO   splr_info;
    MAB_INST_NODE       *this_oac_inst; 
    MAB_INST_NODE       *next_oac_inst; 
    NCS_SPIR_REQ_INFO   spir_info; 

    if (gl_oac_inited == FALSE)
        return status; 

    gl_oac_inited = FALSE;

    /* start from the beginig of the list */ 
    this_oac_inst = gl_oac_inst_list; 

    /* Destroy all the OAA instances */
    while(this_oac_inst)
    {
        /* store the next OAC address */
        next_oac_inst = this_oac_inst->next; 
        
        /* deregister the OAA handle of this Environment with SPIR */ 
        /* deregistering with SPIR should result in oaclib_oac_uninstantiate() */ 
        m_NCS_OS_MEMSET(&spir_info, 0, sizeof(NCS_SPIR_REQ_INFO)); 
        spir_info.type = NCS_SPIR_REQ_REL_INST; 
        spir_info.i_sp_abstract_name = m_OAA_SP_ABST_NAME; 
        spir_info.i_environment_id = this_oac_inst->i_env_id; 
        memcpy(&spir_info.i_instance_name, &this_oac_inst->i_inst_name, sizeof(SaNameT)); 
        status = ncs_spir_api(&spir_info);
        if (status != NCSCC_RC_SUCCESS)
        {
            /* log the failure */ 
            m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR,
                            MAB_OAC_ERR_SPIR_RMV_INST_FAILED, 
                            status, this_oac_inst->i_env_id);
        }

        /* not bothered about the return code, continue to the next instance */ 

        /* goto the next instance of OAA */ 
        this_oac_inst = next_oac_inst; 
    }

    /* deregister with SPLR */ 
    m_NCS_OS_MEMSET(&splr_info, 0, sizeof(NCS_SPLR_REQ_INFO)); 
    splr_info.type = NCS_SPLR_REQ_DEREG; 
    splr_info.i_sp_abstract_name = m_OAA_SP_ABST_NAME;
    status = ncs_splr_api(&splr_info); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_SVC_PRVDR, NCSFL_SEV_ERROR, 
                        MAB_OAC_ERR_SPLR_DEREG_FAILED, status, 0);
        return status; 
    }

    /* unbind with DTS */ 
    mab_log_unbind(); 


    /* release the IPC */ 
    {
       SYSF_MBX tmp_mbx = gl_oacmbx.oac_mbx;
       /*gl_oacmbx.oac_mbx = 0; */  
       (void)m_NCS_IPC_DETACH(&tmp_mbx, NULL, NULL);
       m_NCS_IPC_RELEASE(&tmp_mbx, mab_leave_on_queue_cb);
       gl_oacmbx.oac_mbx = 0; 
    }

    /* release the task related stuff */ 
    m_NCS_TASK_DETACH(gl_oacmbx.oac_mbx_hdl);
    /*m_NCS_TASK_RELEASE(gl_oacmbx.oac_mbx_hdl); */
    gl_oacmbx.oac_mbx_hdl = NULL;


    if((oac_sync_sel.raise_obj != 0) || (oac_sync_sel.rmv_obj != 0))
           m_NCS_SEL_OBJ_IND(oac_sync_sel); 

    return status;
}       

/****************************************************************************\
*  Name:          oac_inst_list_add                                          * 
*                                                                            *
*  Description:   To add this instance of OAA in the list                    *
*                                                                            *
*  Arguments:     PW_ENV_ID - Environment id to be added to the list         * 
*                 i_oac_hdl - OAA handle of this environment                 *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS  - Everything went well                   *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
*  NOTE:                                                                     * 
\****************************************************************************/
static uns32 
oac_inst_list_add(PW_ENV_ID env_id, uns32 i_oac_hdl, SaNameT i_inst_name) 
{
    MAB_INST_NODE   *add_me; 

    /* allocate the home */ 
    add_me = m_MMGR_ALLOC_MAB_INST_NODE; 
    if (add_me == NULL)
    {
        /* log the memory failure */ 
        m_LOG_MAB_MEMFAIL(NCSFL_SEV_ERROR, MAB_MF_MAB_INST_NODE_ALLOC_FAILED, 
                          "oac_inst_list_add()"); 
        return NCSCC_RC_FAILURE; 
    }

    /* occupy */
    m_NCS_OS_MEMSET(add_me, 0, sizeof(MAB_INST_NODE)); 
    add_me->i_env_id = env_id; 
    add_me->i_hdl = i_oac_hdl; 
    memcpy(&add_me->i_inst_name, &i_inst_name, sizeof(SaNameT)); 

    /* register with the society */ 
    add_me->next = gl_oac_inst_list; 
    gl_oac_inst_list = add_me; 

    m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_ENV_INST_ADD_SUCCESS, env_id); 
    return NCSCC_RC_SUCCESS; 
}

/****************************************************************************\
*  Name:          oac_inst_list_release                                      * 
*                                                                            *
*  Description:   To delete and free this instance of OAA in the list        *
*                                                                            *
*  Arguments:     PW_ENV_ID - Environment id to be added to the list         * 
*                 i_oac_hdl - OAA handle of this environment                 *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS  - Everything went well                   *
*                 NCSCC_RC_FAILURE - Something went wrong                    *
*  NOTE:                                                                     * 
\****************************************************************************/
static void 
oac_inst_list_release(PW_ENV_ID env_id) 
{
    MAB_INST_NODE   *release_me; 
    MAB_INST_NODE   *prev = NULL; 
    
    /* find the home */
    release_me = gl_oac_inst_list; 
    while(release_me)
    {
        if (release_me->i_env_id == env_id)
            break; 

        /* search in the next */ 
        prev = release_me; 
        release_me = release_me->next; 
    }

    /* node not found */ 
    if (release_me == NULL)
    {
        /* log that reqd env-id is not found in the list */ 
        m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_ENV_INST_NOT_FOUND, env_id); 
        return; 
    }

    if (prev == NULL)
    {
        /* delete the first node */ 
        prev = release_me->next; 
        gl_oac_inst_list = prev; 
    }
    else
    {
        /* delete the intermediate node */ 
        prev->next = release_me->next; 
    }

    /* log that reqd env-id is found in the list */ 
    m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_ENV_INST_REL_SUCCESS, env_id); 

    /* release the home */ 
    m_MMGR_FREE_MAB_INST_NODE(release_me);

    return; 
}

#endif /* NCS_MAB == 1 */

