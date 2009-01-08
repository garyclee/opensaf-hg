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

#include "ifd.h"

/******************************************************************************
  Function :  ifd_mib_tbl_req
  
  Purpose  :  High Level MIB Access Routine for Interface MIB table
 
  Input    :  struct ncsmib_arg*
    
  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ifd_mib_tbl_req (struct ncsmib_arg *args)
{
   IFSV_CB            *cb = NULL;
   uns32              status = NCSCC_RC_SUCCESS;
   NCSMIBLIB_REQ_INFO  miblib_req;
      
   cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, args->i_mib_key);   

   if(cb == NULL)
      return NCSCC_RC_FAILURE;

   m_NCS_OS_MEMSET(&miblib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

   miblib_req.req = NCSMIBLIB_REQ_MIB_OP;
   miblib_req.info.i_mib_op_info.args = args;
   miblib_req.info.i_mib_op_info.cb = cb;
   
   /* call the mib routine handler */ 
   status = ncsmiblib_process_req(&miblib_req);

   ncshm_give_hdl(args->i_mib_key);
   
   return status; 
}

/****************************************************************************
  PROCEDURE NAME:   ifd_reg_with_mab

  DESCRIPTION:      The function registers all the SNMP MIB tables supported
                    by IFD with MAB
  ARGUMENTS:     
        
  RETURNS:
        NCSCC_RC_SUCCESS
        NCSCC_RC_FAILURE

  NOTES:
*****************************************************************************/
uns32
ifd_reg_with_mab(IFSV_CB *cb)
{
  
   NCSOAC_SS_ARG      mab_arg;
   NCSMIB_TBL_ID      tbl_id;   
   
   m_NCS_OS_MEMSET(&mab_arg, 0, sizeof(NCSOAC_SS_ARG));
   mab_arg.i_oac_hdl = cb->oac_hdl;
   
   for(tbl_id = NCSMIB_TBL_IFSV_BASE; 
       tbl_id <= NCSMIB_TBL_IFSV_IFMAPTBL;
       tbl_id ++)
   {
    /* IFND will not register some tables */
      mab_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
      mab_arg.i_tbl_id = tbl_id;
      mab_arg.info.tbl_owned.i_mib_req = ifd_mib_tbl_req;
      mab_arg.info.tbl_owned.i_mib_key = cb->cb_hdl;
      
      mab_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_IFD;
            
      if(NCSCC_RC_SUCCESS != ncsoac_ss(&mab_arg))
         return NCSCC_RC_FAILURE;       
      
      if(tbl_id != NCSMIB_TBL_IFSV_IFTBL)
      {
      /* regiser the Filter for each table */
         mab_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;          
         mab_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_ANY;
         mab_arg.info.row_owned.i_fltr.fltr.any.meaningless = 0x123;
      
         if(NCSCC_RC_SUCCESS != ncsoac_ss(&mab_arg))
            return NCSCC_RC_FAILURE;       
      }
      
   }/* End of for loop to register the tables */
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  PROCEDURE NAME:   ifd_unreg_with_mab

  DESCRIPTION:      The function de registers all the SNMP MIB tables supported
                    by IFSV with MAB
  ARGUMENTS:     
        
  RETURNS:
        NCSCC_RC_SUCCESS
        NCSCC_RC_FAILURE

  NOTES:
*****************************************************************************/
uns32
ifd_unreg_with_mab(IFSV_CB *cb)
{
   NCSOAC_SS_ARG      mab_arg;
   NCSMIB_TBL_ID      tbl_id;   
   
   m_NCS_OS_MEMSET(&mab_arg, 0, sizeof(NCSOAC_SS_ARG));   
   mab_arg.i_oac_hdl = cb->oac_hdl;   
   mab_arg.i_op = NCSOAC_SS_OP_TBL_GONE;     
   
   for(tbl_id = NCSMIB_TBL_IFSV_BASE; 
       tbl_id <= NCSMIB_TBL_IFSV_IFMAPTBL;
       tbl_id ++)
   {

      mab_arg.i_tbl_id = tbl_id;      
      if(NCSCC_RC_SUCCESS != ncsoac_ss(&mab_arg))
         return NCSCC_RC_FAILURE;       
   }
   return NCSCC_RC_SUCCESS;
}


uns32 ifd_reg_with_miblib( )
{
   uns32 rc;

   rc = ifentry_tbl_reg();

   if(rc != NCSCC_RC_SUCCESS)
     {
      m_IFA_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifentry_tbl_reg() returned failure, rc:",rc);
      return rc;
     }

   rc = ifmibobjects_tbl_reg();

   if(rc != NCSCC_RC_SUCCESS)
     {
      m_IFA_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifmibobjects_tbl_reg() returned failure, rc:",rc);
      return rc;
     }

   rc = intf_tbl_reg();

   if(rc != NCSCC_RC_SUCCESS)
     {
      m_IFA_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"intf_tbl_reg() returned failure, rc:",rc);
      return rc;
     }

   rc = ncsifsvifmapentry_tbl_reg();

   if(rc != NCSCC_RC_SUCCESS)
     {
      m_IFA_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncsifsvifmapentry_tbl_reg() returned failure, rc:",rc);
      return rc;
     }

   rc = ncsifsvifxentry_tbl_reg();

   if(rc != NCSCC_RC_SUCCESS)
     {
      m_IFA_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncsifsvifxentry_tbl_reg() returned failure, rc:",rc);
      return rc;
     }
   
#if(NCS_IFSV_BOND_INTF == 1)
   rc = ncsifsvbindifentry_tbl_reg();
   if(rc != NCSCC_RC_SUCCESS)
     {
      m_IFA_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncsifsvbindifentry_tbl_reg() returned failure, rc:",rc);
      return rc;
     }
#endif    


   return NCSCC_RC_SUCCESS;
}

