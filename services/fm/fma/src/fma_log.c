/*****************************************************************************
 *                                                                           *
 * NOTICE TO PROGRAMMERS:  RESTRICTED USE.                                   *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED TO YOU AND YOUR COMPANY UNDER A LICENSE         *
 * AGREEMENT FROM EMERSON, INC.                                              *
 *                                                                           *
 * THE TERMS OF THE LICENSE AGREEMENT RESTRICT THE USE OF THIS SOFTWARE TO   *
 * SPECIFIC PROJECTS ("LICENSEE APPLICATIONS") IDENTIFIED IN THE AGREEMENT.  *
 *                                                                           *
 * IF YOU ARE UNSURE WHETHER YOU ARE AUTHORIZED TO USE THIS SOFTWARE ON YOUR *
 * PROJECT,  PLEASE CHECK WITH YOUR MANAGEMENT.                              *
 *                                                                           *
 *****************************************************************************
 
 
..............................................................................

DESCRIPTION:

******************************************************************************
*/

#include "fma.h"

#if ( NCS_FMA_LOG == 1 )

/****************************************************************************
  Name          : fma_log_seapi

  Description   : This routine logs the SE-API info.

  Arguments     : op     - se-api operation
                  status - status of se-api operation execution
                  sev    - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void fma_log_seapi (FMA_LOG_SEAPI op, FMA_LOG_SEAPI status, uns8 sev)
{
   ncs_logmsg(NCS_SERVICE_ID_FMA, FMA_LID_SEAPI, FMA_FC_SEAPI,
              NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TII, op, status);

   return;
}
/****************************************************************************
  Name          : fma_log_mds

  Description   : This routine logs the MDS info.

  Arguments     : op     - mds operation
                  status - status of mds operation execution
                  sev    - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void fma_log_mds (FMA_LOG_MDS op, FMA_LOG_MDS status, uns8 sev)
{
   ncs_logmsg(NCS_SERVICE_ID_FMA, FMA_LID_MDS, FMA_FC_MDS,
              NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TII, op, status);

   return;
}

/****************************************************************************
  Name          : fma_log_lock

  Description   : This routine logs the LOCK info.

  Arguments     : op     - lock operation
                  status - status of lock operation execution
                  sev    - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void fma_log_lock (FMA_LOG_LOCK op, FMA_LOG_LOCK status, uns8 sev)
{
   ncs_logmsg(NCS_SERVICE_ID_FMA, FMA_LID_LOCK, FMA_FC_LOCK,
              NCSFL_LC_LOCKS, sev, NCSFL_TYPE_TII, op, status);

   return;
}

/****************************************************************************
  Name          : fma_log_cb

  Description   : This routine logs the control block info.

  Arguments     : op     - cb operation
                  status - status of cb operation execution
                  sev    - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void fma_log_cb (FMA_LOG_CB op, FMA_LOG_CB status, uns8 sev)
{
   ncs_logmsg(NCS_SERVICE_ID_FMA, FMA_LID_CB, FMA_FC_CB,
              NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TII, op, status);

   return;
}

/****************************************************************************
  Name          : fma_log_sel_obj

  Description   : This routine logs the selection object info.

  Arguments     : op     - sel-obj operation
                  status - status of sel-obj operation execution
                  sev    - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void fma_log_sel_obj (FMA_LOG_SEL_OBJ op, FMA_LOG_SEL_OBJ status, uns8 sev)
{
   ncs_logmsg(NCS_SERVICE_ID_FMA, FMA_LID_SEL_OBJ, FMA_FC_SEL_OBJ,
              NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TII, op, status);

   return;
}

/****************************************************************************
  Name          : fma_log_api

  Description   : This routine logs the AMF API info.

  Arguments     : type      - API type
                  status    - status of API execution
                  comp_name - ptr to the comp-name
                  sev       - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void fma_log_api (FMA_LOG_API   type,
                  FMA_LOG_API   status,
                  const SaNameT *comp_name,
                  uns8          sev)
{
   uns8 name[SA_MAX_NAME_LENGTH];

   m_NCS_OS_MEMSET(name, '\0', SA_MAX_NAME_LENGTH);

   ncs_logmsg(NCS_SERVICE_ID_FMA, FMA_LID_API, FMA_FC_API,
              NCSFL_LC_API, sev, NCSFL_TYPE_TII, type, status);

   return;
}
/****************************************************************************
  Name          : fma_log_hdl_db

  Description   : This routine logs the handle database info.

  Arguments     : op     - hdl-db operation
                  status - status of hdl-db operation execution
                  hdl    - amf-hdl
                  sev    - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void fma_log_hdl_db (FMA_LOG_HDL_DB op,
                     FMA_LOG_HDL_DB status,
                     uns32          hdl,
                     uns8           sev)
{
   ncs_logmsg(NCS_SERVICE_ID_FMA, FMA_LID_HDL_DB, FMA_FC_HDL_DB,
              NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TIIL, op, status, hdl);

   return;
}


/****************************************************************************
  Name          : fma_log_misc

  Description   : This routine logs miscellaneous info.

  Arguments     : op  - misc operation
                  status - status of misc operation.
                  sev - severity

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void fma_log_misc (FMA_LOG_MISC op, uns8 sev,void *func_name)
{
  
   ncs_logmsg(NCS_SERVICE_ID_FMA, FMA_LID_MISC, FMA_FC_MISC,
              NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TIC, op,func_name);

   return;
}

/****************************************************************************
  Name          : fma_log_mem

  Description   : This routine logs miscellaneous info.

  Arguments     : op  - misc operation
                  sev - severity
                  status - status of memory operation
  
  Return Values : None

  Notes         : None.
 *****************************************************************************/
void fma_log_mem (FMA_LOG_MEM op, FMA_LOG_MEM status, uns8 sev)
{
   ncs_logmsg(NCS_SERVICE_ID_FMA, FMA_LID_MEM, FMA_FC_MEM,
              NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TI, op, status);

   return;
}
/****************************************************************************
  Name          : fma_log_mbx

  Description   : This routine logs mailbox info.

  Arguments     : op  - mbx  operation
                  sev - severity
                  status - status of mbx operation   
                 
  Return Values : None

  Notes         : None.
 *****************************************************************************/
void fma_log_mbx (FMA_LOG_MBX op, FMA_LOG_MBX status, uns8 sev)
{
   ncs_logmsg(NCS_SERVICE_ID_FMA, FMA_LID_MBX, FMA_FC_MBX,
              NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TI, op, status);

   return;
}

/****************************************************************************
  Name          : fma_log_task

  Description   : This routine logs task info.

  Arguments     : op  - task  operation
                  sev - severity
                  status - status of task operation

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void fma_log_task (FMA_LOG_TASK op, FMA_LOG_TASK status, uns8 sev)
{
   ncs_logmsg(NCS_SERVICE_ID_FMA, FMA_LID_TASK, FMA_FC_TASK,
              NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TII, op, status);

   return;
}

/****************************************************************************
  Name          : fma_log_cbk

  Description   : This routine logs callback info.

  Arguments     : op  - task  operation
                  sev - severity
                  status - status of task operation

  Return Values : None

  Notes         : None.
 *****************************************************************************/
void fma_log_cbk (FMA_LOG_CBK op, FMA_LOG_CBK status, uns8 sev)
{
   ncs_logmsg(NCS_SERVICE_ID_FMA, FMA_LID_CBK, FMA_FC_CBK,
              NCSFL_LC_HEADLINE, sev, NCSFL_TYPE_TII, op, status);

   return;
}


/****************************************************************************
 * Name          : fma_log_reg
 *
 * Description   : 
 * 
 * Arguments     : 
 * 
 * Return Values : 
 *    
 * Notes         : None.
 *****************************************************************************/
uns32 fma_log_reg(void)
{
   NCS_DTSV_RQ            reg;
   uns32 rc = NCSCC_RC_SUCCESS;   

   m_NCS_MEMSET(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                = NCS_DTSV_OP_BIND;
   reg.info.bind_svc.svc_id = NCS_SERVICE_ID_FMA; 
   reg.info.bind_svc.version = FMA_LOG_VERSION;
   /* fill svc_name */
   m_NCS_STRCPY(reg.info.bind_svc.svc_name, "FMA");
   
   rc = ncs_dtsv_su_req(&reg);
   
   return rc;
} 

/****************************************************************************
 * Name          : fma_log_dereg
 * 
 * Description   :  
 *                 
 * 
 * Arguments     : 
 *
 * Return Values : None
 *
 * Notes         : None.
 *****************************************************************************/
void fma_log_dereg(void)
{
   NCS_DTSV_RQ        reg;
 
   m_NCS_MEMSET(&reg,0,sizeof(NCS_DTSV_RQ));
   reg.i_op                   = NCS_DTSV_OP_UNBIND;
   reg.info.unbind_svc.svc_id = NCS_SERVICE_ID_FMA;
   ncs_dtsv_su_req(&reg);
   return;
}

#endif 

