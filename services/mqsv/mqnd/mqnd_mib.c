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

  MODULE NAME: MQND_MIB.C

..............................................................................

  DESCRIPTION:
     This module contains MQND-MIB Registration routines

...............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/


#include "mqnd.h"

static uns32 mqnd_mib_tbl_req (struct ncsmib_arg *args);
/*static uns32 mqnd_unreg_with_mab(MQND_CB *cb);*/
/******************************************************************************
  Function :  mqnd_mib_tbl_req

  Purpose  :  High Level MIB Access Routine for Interface MIB table

  Input    :  struct ncsmib_arg*

  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

  Notes    :
******************************************************************************/
static uns32 mqnd_mib_tbl_req (struct ncsmib_arg *args)
{
   MQND_CB            *cb = NULL;
   uns32              status = NCSCC_RC_SUCCESS;
   NCSMIBLIB_REQ_INFO  miblib_req;

   cb = ncshm_take_hdl(NCS_SERVICE_ID_MQND, args->i_mib_key);

   if(cb == NULL)
      return NCSCC_RC_FAILURE;

   m_NCS_OS_MEMSET(&miblib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

   miblib_req.req = NCSMIBLIB_REQ_MIB_OP;
   miblib_req.info.i_mib_op_info.args = args;
   miblib_req.info.i_mib_op_info.cb = cb;

   /* call the mib routine handler */
   status = ncsmiblib_process_req(&miblib_req);

   if(status == NCSCC_RC_SUCCESS)
       m_LOG_MQSV_ND(MQND_MIBLIB_TBL_REQ_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_INFO,status,__FILE__,__LINE__);
   else
       m_LOG_MQSV_ND(MQND_MIBLIB_TBL_REQ_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,status,__FILE__,__LINE__);
   ncshm_give_hdl((uns32)args->i_mib_key);

   return status;
}


/****************************************************************************
  PROCEDURE NAME:   mqnd_reg_with_mab

  DESCRIPTION:      The function registers all the SNMP MIB tables supported
                    by MQND with MAB
  ARGUMENTS:

  RETURNS:
        NCSCC_RC_SUCCESS
        NCSCC_RC_FAILURE

  NOTES:
*****************************************************************************/
uns32 mqnd_reg_with_mab(MQND_CB *cb)
{

   NCSOAC_SS_ARG      mab_arg;
   uns32              rc = NCSCC_RC_SUCCESS;

   m_NCS_OS_MEMSET(&mab_arg, 0, sizeof(NCSOAC_SS_ARG));
   mab_arg.i_oac_hdl = cb->oac_hdl;

   mab_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
   mab_arg.i_tbl_id = NCSMIB_TBL_MQSV_MSGQTBL;
   mab_arg.info.tbl_owned.i_mib_req = mqnd_mib_tbl_req;
   mab_arg.info.tbl_owned.i_mib_key = cb->cb_hdl;
   mab_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_MQND;

   rc = ncsoac_ss(&mab_arg);
  
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_MQSV_ND(MQND_MIB_MSGQ_TBL_REG_WITH_MAB_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      return NCSCC_RC_FAILURE;
   }
   else
      m_LOG_MQSV_ND(MQND_MIB_MSGQ_TBL_REG_WITH_MAB_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
      
#if 0
   mab_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;
   mab_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_ANY;
   mab_arg.info.row_owned.i_fltr.fltr.any.meaningless = 0x123;

   if(NCSCC_RC_SUCCESS != ncsoac_ss(&mab_arg))
      return NCSCC_RC_FAILURE;
#endif 

   mab_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
   mab_arg.i_tbl_id = NCSMIB_TBL_MQSV_MSGQPRTBL;
   mab_arg.info.tbl_owned.i_mib_req = mqnd_mib_tbl_req;
   mab_arg.info.tbl_owned.i_mib_key = cb->cb_hdl;
   mab_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_MQND;

   rc = ncsoac_ss(&mab_arg);
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_MQSV_ND(MQND_MIB_MSGQPR_TBL_REG_WITH_MAB_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      return rc;
   }
   else
      m_LOG_MQSV_ND(MQND_MIB_MSGQPR_TBL_REG_WITH_MAB_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
#if 0
   mab_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;
   mab_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_ANY;
   mab_arg.info.row_owned.i_fltr.fltr.any.meaningless = 0x123;

   if(NCSCC_RC_SUCCESS != ncsoac_ss(&mab_arg))
      return NCSCC_RC_FAILURE;
#endif
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  PROCEDURE NAME:   mqnd_unreg_with_mab

  DESCRIPTION:      The function de registers all the SNMP MIB tables supported
                    by MQND with MAB
  ARGUMENTS:

  RETURNS:
        NCSCC_RC_SUCCESS
        NCSCC_RC_FAILURE

  NOTES:
*****************************************************************************/
#if 0
static uns32 mqnd_unreg_with_mab(MQND_CB *cb)
{
    NCSOAC_SS_ARG      mab_arg;
    uns32              rc = NCSCC_RC_SUCCESS;
    m_NCS_OS_MEMSET(&mab_arg, 0, sizeof(NCSOAC_SS_ARG));
    mab_arg.i_oac_hdl = cb->cb_hdl;
    mab_arg.i_op = NCSOAC_SS_OP_TBL_GONE;

    mab_arg.i_tbl_id = NCSMIB_TBL_MQSV_MSGQTBL;
    rc = ncsoac_ss(&mab_arg);
    if(rc != NCSCC_RC_SUCCESS)
    {
      m_LOG_MQSV_ND(MQND_MIB_MSGQ_TBL_DEREG_WITH_MAB_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      return NCSCC_RC_FAILURE;
    }
    else
      m_LOG_MQSV_ND(MQND_MIB_MSGQ_TBL_DEREG_WITH_MAB_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);
    
    mab_arg.i_tbl_id = NCSMIB_TBL_MQSV_MSGQPRTBL;
    rc = ncsoac_ss(&mab_arg);
    if(rc != NCSCC_RC_SUCCESS)
    {
      m_LOG_MQSV_ND(MQND_MIB_MSGQPR_TBL_DEREG_WITH_MAB_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
      return NCSCC_RC_FAILURE;
    }
    else
      m_LOG_MQSV_ND(MQND_MIB_MSGQPR_TBL_DEREG_WITH_MAB_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);

    return NCSCC_RC_SUCCESS;
}

#endif


/****************************************************************************
 * Name        : mqnd_reg_with_miblib
 *
 * Description : To register with miblib
 *
 * Arguments   :
 *
****************************************************************************/
uns32 mqnd_reg_with_miblib(void)
{
   uns32 rc;

   rc = samsgqueueentry_tbl_reg();
   if (rc!= NCSCC_RC_SUCCESS)
   {
       m_LOG_MQSV_ND(MQND_MSGQTBL_REG_WITH_MIBLIB_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
       return rc;
   }
   else
       m_LOG_MQSV_ND(MQND_MSGQTBL_REG_WITH_MIBLIB_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);

   rc = samsgqueuepriorityentry_tbl_reg();
   if (rc!= NCSCC_RC_SUCCESS)
   {
       m_LOG_MQSV_ND(MQND_MSGQPRTBL_REG_WITH_MIBLIB_FAILED,NCSFL_LC_MQSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__);
       return rc;
   }
   else
       m_LOG_MQSV_ND(MQND_MSGQPRTBL_REG_WITH_MIBLIB_SUCCESS,NCSFL_LC_MQSV_INIT,NCSFL_SEV_NOTICE,rc,__FILE__,__LINE__);

   return NCSCC_RC_SUCCESS;
}





















