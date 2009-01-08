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


#include "cpd.h"

static uns32 cpd_mib_tbl_req(struct ncsmib_arg *args);


/******************************************************************************
  Function :  cpd_mib_tbl_req
  
  Purpose  :  High Level MIB Access Routine for Interface MIB table
 
  Input    :  struct ncsmib_arg*
    
  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/

uns32 cpd_mib_tbl_req(struct ncsmib_arg *args)
{
   CPD_CB             *cb = NULL;
   uns32              rc = NCSCC_RC_SUCCESS;
   NCSMIBLIB_REQ_INFO  miblib_req;

   cb = ncshm_take_hdl(NCS_SERVICE_ID_CPD, (uns32)args->i_mib_key);

   if (cb == NULL)
       return NCSCC_RC_FAILURE;

   m_NCS_OS_MEMSET(&miblib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

   miblib_req.req = NCSMIBLIB_REQ_MIB_OP;
   miblib_req.info.i_mib_op_info.args = args;
   miblib_req.info.i_mib_op_info.cb = cb;
   
   /*  call the mib routine handler  */
   rc = ncsmiblib_process_req(&miblib_req);
   if(rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_CPD_CL(CPD_MIBLIB_PROCESS_REQ_FAILED,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__);
   }
  
   ncshm_give_hdl((uns32)args->i_mib_key);
  
   return rc;
}                     
                                                                                

/****************************************************************************
  PROCEDURE NAME:   cpd_reg_with_mab

  DESCRIPTION:      The function registers all the SNMP MIB tables supported
                    by CPD with MAB
  ARGUMENTS:     
        
  RETURNS:
        NCSCC_RC_SUCCESS
        NCSCC_RC_FAILURE

  NOTES:
*****************************************************************************/
uns32
cpd_reg_with_mab(CPD_CB *cb)
{
   NCSOAC_SS_ARG      mab_arg;
   NCSMIB_TBL_ID      tbl_id;

   m_NCS_OS_MEMSET(&mab_arg, 0, sizeof(NCSOAC_SS_ARG));
   mab_arg.i_oac_hdl = cb->oac_hdl;

   for(tbl_id = NCSMIB_TBL_CPSV_MIB_BASE;
       tbl_id <= NCSMIB_TBL_CPSV_REPLOCTBL;
       tbl_id ++)
   {
       mab_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
       mab_arg.i_tbl_id = tbl_id;
       mab_arg.info.tbl_owned.i_mib_req = cpd_mib_tbl_req;    
       mab_arg.info.tbl_owned.i_mib_key = cb->cpd_hdl;
       
       mab_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_CPD; 
 
       if (NCSCC_RC_SUCCESS != ncsoac_ss(&mab_arg))
       {
          m_LOG_CPD_CL(CPD_REGISTER_WITH_MAB_FAILED,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__);
          return NCSCC_RC_FAILURE;
       }

       mab_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;
       mab_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_ANY;
       mab_arg.info.row_owned.i_fltr.fltr.any.meaningless = 0x123;
          
       if(NCSCC_RC_SUCCESS != ncsoac_ss(&mab_arg))
       {
          m_LOG_CPD_CL(CPD_REGISTER_WITH_MAB_FAILED,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__); 
          return NCSCC_RC_FAILURE;
       } 
   }
       return NCSCC_RC_SUCCESS;
}                   


/****************************************************************************
  PROCEDURE NAME:   cpd_unreg_with_mab

  DESCRIPTION:      The function de registers all the SNMP MIB tables supported
                    by CPSV with MAB
  ARGUMENTS:     
        
  RETURNS:
        NCSCC_RC_SUCCESS
        NCSCC_RC_FAILURE

  NOTES:
*****************************************************************************/
uns32
cpd_unreg_with_mab(CPD_CB *cb)
{
    NCSOAC_SS_ARG      mab_arg;
    NCSMIB_TBL_ID      tbl_id;

    m_NCS_OS_MEMSET(&mab_arg, 0, sizeof(NCSOAC_SS_ARG));
    mab_arg.i_oac_hdl = cb->cpd_hdl;
    mab_arg.i_op = NCSOAC_SS_OP_TBL_GONE;    
 
    for(tbl_id = NCSMIB_TBL_CPSV_MIB_BASE;
        tbl_id <= NCSMIB_TBL_CPSV_REPLOCTBL;
        tbl_id ++)
    {
        mab_arg.i_tbl_id = tbl_id;
        if(NCSCC_RC_SUCCESS != ncsoac_ss(&mab_arg))
        {
           m_LOG_CPD_CL(CPD_UNREGISTER_WITH_MAB_FAILED,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__);
           return NCSCC_RC_FAILURE;
        }
    }
    return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name        : cpd_reg_with_miblib
 *
 * Description : To register with miblib
 *
 * Arguments   :
 *
****************************************************************************/
uns32 cpd_reg_with_miblib(void)
{
   uns32 rc;

   rc = safckptscalarobject_tbl_reg();
   if (rc!= NCSCC_RC_SUCCESS)
   {
      m_LOG_CPD_CL(CPD_SCALARTBL_REG_FAILED,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__);
      return rc;
   }

   rc = sackptcheckpointentry_tbl_reg();
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_CPD_CL(CPD_CKPTTBL_REG_FAILED,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__);
      return rc;
   }

   rc = sackptnodereplicalocentry_tbl_reg();
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_LOG_CPD_CL(CPD_REPLOCTBL_REG_FAILED,CPD_FC_HDLN,NCSFL_SEV_ERROR,__FILE__,__LINE__);
      return rc;
   }

   return NCSCC_RC_SUCCESS;
}          
































                                      
