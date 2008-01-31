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

#include "mqd.h"

static uns32 mqd_mib_tbl_req(struct ncsmib_arg *args);

/******************************************************************************
  Function :  mqd_mib_tbl_req
  
  Purpose  :  High Level MIB Access Routine for Interface MIB table
 
  Input    :  struct ncsmib_arg*
    
  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/

static uns32 mqd_mib_tbl_req(struct ncsmib_arg *args)
{
   MQD_CB             *pMqd = NULL;
   uns32              rc = NCSCC_RC_SUCCESS;
   NCSMIBLIB_REQ_INFO  miblib_req;

   pMqd = ncshm_take_hdl(NCS_SERVICE_ID_MQD, args->i_mib_key);

   if (pMqd == NULL)
       return NCSCC_RC_FAILURE;

   m_NCS_OS_MEMSET(&miblib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

   miblib_req.req = NCSMIBLIB_REQ_MIB_OP;
   miblib_req.info.i_mib_op_info.args = args;
   miblib_req.info.i_mib_op_info.cb = pMqd;

   /*  call the mib routine handler  */
   rc = ncsmiblib_process_req(&miblib_req);
   if(rc == NCSCC_RC_SUCCESS)
       m_LOG_MQSV_D(MQD_MIB_TBL_REQ_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_INFO,rc,__FILE__,__LINE__);
   else
       m_LOG_MQSV_D(MQD_MIB_TBL_REQ_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
        
   ncshm_give_hdl((uns32)args->i_mib_key);

   return rc;
}



/****************************************************************************
  PROCEDURE NAME:   mqd_reg_with_mab

  DESCRIPTION:      The function registers all the SNMP MIB tables supported
                    by MQD with MAB
  ARGUMENTS:

  RETURNS:
        NCSCC_RC_SUCCESS
        NCSCC_RC_FAILURE

  NOTES:
*****************************************************************************/
uns32 mqd_reg_with_mab(MQD_CB *pMqd)
{

   NCSOAC_SS_ARG      mab_arg;
   uns32              rc= NCSCC_RC_SUCCESS;

   m_NCS_OS_MEMSET(&mab_arg, 0, sizeof(NCSOAC_SS_ARG));
   mab_arg.i_oac_hdl = pMqd->oac_hdl;

   mab_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
   mab_arg.i_tbl_id = NCSMIB_TBL_MQSV_SCLROBJECTSTBL;
   mab_arg.info.tbl_owned.i_mib_req = mqd_mib_tbl_req;
   mab_arg.info.tbl_owned.i_mib_key = pMqd->hdl;
   mab_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_MQD;

   rc= ncsoac_ss(&mab_arg);
   if(NCSCC_RC_SUCCESS != rc)
   {
       m_LOG_MQSV_D(MQD_MIB_SCALAR_TBL_REG_WITH_MAB_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
       return rc;
   }
   else
       m_LOG_MQSV_D(MQD_MIB_TBL_REQ_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
       
   mab_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;
   mab_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_ANY;
   mab_arg.info.row_owned.i_fltr.fltr.any.meaningless = 0x123;

   rc = ncsoac_ss(&mab_arg);

   if(NCSCC_RC_SUCCESS != rc)
   {
      m_LOG_MQSV_D(MQD_MIB_SCALAR_TBL_ROW_OWNED_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      return NCSCC_RC_FAILURE;
   }
   else 
      m_LOG_MQSV_D(MQD_MIB_SCALAR_TBL_ROW_OWNED_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
      


   mab_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
   mab_arg.i_tbl_id = NCSMIB_TBL_MQSV_MQGRPTBL;
   mab_arg.info.tbl_owned.i_mib_req = mqd_mib_tbl_req;
   mab_arg.info.tbl_owned.i_mib_key = pMqd->hdl;
   mab_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_MQD;

   rc = ncsoac_ss(&mab_arg);
   if(NCSCC_RC_SUCCESS != rc)
   {
      m_LOG_MQSV_D(MQD_MIB_QGROUP_TBL_OWNED_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      return NCSCC_RC_FAILURE;
   }
   else
      m_LOG_MQSV_D(MQD_MIB_QGROUP_TBL_OWNED_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
      
   mab_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;
   mab_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_ANY;
   mab_arg.info.row_owned.i_fltr.fltr.any.meaningless = 0x123;

   rc =  ncsoac_ss(&mab_arg);
   if(NCSCC_RC_SUCCESS != rc)
   {
      m_LOG_MQSV_D(MQD_MIB_QGROUP_TBL_ROW_OWNED_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      return NCSCC_RC_FAILURE;
   }
   else
      m_LOG_MQSV_D(MQD_MIB_QGROUP_TBL_ROW_OWNED_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
      
   mab_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
   mab_arg.i_tbl_id = NCSMIB_TBL_MQSV_MSGQGRPMBRSTBL;
   mab_arg.info.tbl_owned.i_mib_req = mqd_mib_tbl_req;
   mab_arg.info.tbl_owned.i_mib_key = pMqd->hdl;
   mab_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_MQD;

   rc =  ncsoac_ss(&mab_arg);
   if(NCSCC_RC_SUCCESS != rc)
   {
      m_LOG_MQSV_D(MQD_MIB_QGROUP_MEMBERS_TBL_OWNED_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      return NCSCC_RC_FAILURE;
   }
   else 
      m_LOG_MQSV_D(MQD_MIB_QGROUP_MEMBERS_TBL_OWNED_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
      
   mab_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;
   mab_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_ANY;
   mab_arg.info.row_owned.i_fltr.fltr.any.meaningless = 0x123;

   rc =  ncsoac_ss(&mab_arg);
   if(NCSCC_RC_SUCCESS != rc)
   {
      m_LOG_MQSV_D(MQD_MIB_QGROUP_MEMBERS_TBL_ROW_OWNED_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      return NCSCC_RC_FAILURE;
   }
   else 
      m_LOG_MQSV_D(MQD_MIB_QGROUP_MEMBERS_TBL_ROW_OWNED_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
       

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  PROCEDURE NAME:   mqd_unreg_with_mab

  DESCRIPTION:      The function de registers all the SNMP MIB tables supported
                    by MQD with MAB
  ARGUMENTS:

  RETURNS:
        NCSCC_RC_SUCCESS
        NCSCC_RC_FAILURE

  NOTES:
*****************************************************************************/

uns32 mqd_unreg_with_mab(MQD_CB *pMqd)
{
    NCSOAC_SS_ARG      mab_arg;
    uns32              rc= NCSCC_RC_SUCCESS;

    m_NCS_OS_MEMSET(&mab_arg, 0, sizeof(NCSOAC_SS_ARG));
    mab_arg.i_oac_hdl = pMqd->hdl;
    mab_arg.i_op = NCSOAC_SS_OP_TBL_GONE;

    mab_arg.i_tbl_id = NCSMIB_TBL_MQSV_SCLROBJECTSTBL;
    rc = ncsoac_ss(&mab_arg);
    if(NCSCC_RC_SUCCESS != rc)
    {
       m_LOG_MQSV_D(MQD_MIB_SCALARTBL_UNREG_WITH_MAB_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
       return NCSCC_RC_FAILURE;
    }
    else 
       m_LOG_MQSV_D(MQD_MIB_SCALARTBL_UNREG_WITH_MAB_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
        
    mab_arg.i_tbl_id = NCSMIB_TBL_MQSV_MQGRPTBL;
    rc = ncsoac_ss(&mab_arg);

    if(NCSCC_RC_SUCCESS != rc)
    {
       m_LOG_MQSV_D(MQD_MIB_GRPENTRYTBL_UNREG_WITH_MAB_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
       return NCSCC_RC_FAILURE;
    }
    else 
       m_LOG_MQSV_D(MQD_MIB_GRPENTRYTBL_UNREG_WITH_MAB_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
        
    mab_arg.i_tbl_id = NCSMIB_TBL_MQSV_MSGQGRPMBRSTBL;
    rc = ncsoac_ss(&mab_arg);
    if(NCSCC_RC_SUCCESS != rc)
    {
       m_LOG_MQSV_D(MQD_MIB_GRPMEMBRSENTRYTBL_UNREG_WITH_MAB_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
       return NCSCC_RC_FAILURE;
    }
    else
       m_LOG_MQSV_D(MQD_MIB_GRPMEMBRSENTRYTBL_UNREG_WITH_MAB_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
       
    return NCSCC_RC_SUCCESS;
}



/****************************************************************************
 * Name        : mqd_reg_with_miblib
 *
 * Description : To register with miblib
 *
 * Arguments   :
 *
****************************************************************************/
uns32 mqd_reg_with_miblib()
{
   uns32 rc;

   rc = safmsgscalarobject_tbl_reg();
   if (rc!= NCSCC_RC_SUCCESS)
   {
       m_LOG_MQSV_D(MQD_MIB_SCALARTBL_REG_WITH_MIBLIB_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      return rc;
   }
   else
       m_LOG_MQSV_D(MQD_MIB_SCALARTBL_REG_WITH_MIBLIB_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
      
   rc = samsgqueuegroupmembersentry_tbl_reg();
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_MQSV_D(MQD_MIB_GRPENTRYTBL_REG_WITH_MIBLIB_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      return rc;
   }
   else
       m_LOG_MQSV_D(MQD_MIB_GRPENTRYTBL_REG_WITH_MIBLIB_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
      
   rc = samsgqueuegroupentry_tbl_reg();
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_MQSV_D(MQD_MIB_GRPMEMBERSENTRYTBL_REG_WITH_MIBLIB_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      return rc;
   }
   else 
       m_LOG_MQSV_D(MQD_MIB_GRPMEMBERSENTRYTBL_REG_WITH_MIBLIB_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
      

   return NCSCC_RC_SUCCESS;
}

 
