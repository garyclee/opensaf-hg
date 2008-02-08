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

  MODULE NAME: IFVS_MIB.C 

..............................................................................

  DESCRIPTION:
     This module contains IFSV-MIB Registration routines

...............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/

#include "ifnd.h"

/******************************************************************************
  Function :  ifnd_mib_tbl_req
  
  Purpose  :  High Level MIB Access Routine for Interface MIB table
 
  Input    :  struct ncsmib_arg*
    
  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ifnd_mib_tbl_req (struct ncsmib_arg *args)
{
   IFSV_CB            *cb = NULL;
   uns32              status = NCSCC_RC_SUCCESS;
   NCSMIBLIB_REQ_INFO  miblib_req;
      
   cb = ncshm_take_hdl(NCS_SERVICE_ID_IFND, args->i_mib_key);   

   if(cb == NULL)
      return NCSCC_RC_FAILURE;

   m_NCS_OS_MEMSET(&miblib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

   miblib_req.req = NCSMIBLIB_REQ_MIB_OP;
   miblib_req.info.i_mib_op_info.args = args;
   miblib_req.info.i_mib_op_info.cb = cb;
   
   /* call the mib routine handler */ 
   status = ncsmiblib_process_req(&miblib_req);
   if( NCSCC_RC_SUCCESS != status )
   {
       m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "ncsmiblib_process_req; failed after MIB Registration  ","FUNC::ifnd_mib_tbl_req");
   }

   ncshm_give_hdl(args->i_mib_key);
   
   return status; 
}

/****************************************************************************
  PROCEDURE NAME:   ifnd_reg_with_mab

  DESCRIPTION:      The function registers all the SNMP MIB tables supported
                    by IFD with MAB
  ARGUMENTS:     
        
  RETURNS:
        NCSCC_RC_SUCCESS
        NCSCC_RC_FAILURE

  NOTES:
*****************************************************************************/
uns32
ifnd_reg_with_mab(IFSV_CB *cb)
{
  
   NCSOAC_SS_ARG      mab_arg;

   
   m_NCS_OS_MEMSET(&mab_arg, 0, sizeof(NCSOAC_SS_ARG));
   mab_arg.i_oac_hdl = cb->oac_hdl;
   
   mab_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
   mab_arg.i_tbl_id = NCSMIB_TBL_IFSV_IFTBL;
   mab_arg.info.tbl_owned.i_mib_req = ifnd_mib_tbl_req;
   mab_arg.info.tbl_owned.i_mib_key = cb->cb_hdl;      
   mab_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_IFND;
   
 
   if(NCSCC_RC_SUCCESS != ncsoac_ss(&mab_arg))
   {
       m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "ncsoac_ss; IFND Registration with MAB Failed  ","FUNC::ifnd_reg_with_mab");
      return NCSCC_RC_FAILURE;       
   }
   
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  PROCEDURE NAME:   ifnd_unreg_with_mab

  DESCRIPTION:      The function de registers all the SNMP MIB tables supported
                    by IFSV with MAB
  ARGUMENTS:     
        
  RETURNS:
        NCSCC_RC_SUCCESS
        NCSCC_RC_FAILURE

  NOTES:
*****************************************************************************/
uns32
ifnd_unreg_with_mab(IFSV_CB *cb)
{
   NCSOAC_SS_ARG      mab_arg;

   
   m_NCS_OS_MEMSET(&mab_arg, 0, sizeof(NCSOAC_SS_ARG));   
   mab_arg.i_oac_hdl = cb->cb_hdl;   
   mab_arg.i_op = NCSOAC_SS_OP_TBL_GONE;     
   
   mab_arg.i_tbl_id = NCSMIB_TBL_IFSV_IFTBL;      
   if(NCSCC_RC_SUCCESS != ncsoac_ss(&mab_arg))
   {
       m_IFND_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,
             "ncsoac_ss; IFND UnRegistration with MAB Failed  ","FUNC::ifnd_unreg_with_mab");
       return NCSCC_RC_FAILURE;       
   }

   return NCSCC_RC_SUCCESS;
}


uns32 ifnd_reg_with_miblib( )
{
   return(ifentry_tbl_reg());
}




