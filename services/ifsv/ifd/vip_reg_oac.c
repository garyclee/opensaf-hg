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
FILE NAME: VIP_REG_OAC.C
******************************************************************************/
/*
~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
~*~*~*~*~*~*~*~*~*~*~*~*~*     DESCRIPTION     *~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~*   This file Contains the implementation for "IP Address Virtualization"  *~*
*~*   feature's MIB registration with MIBLIB && OAC                          *~*
~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
                                                                                                                             
~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~*   vip_miblib_reg ~~~~~~~~~~~~~~~~~ Function for "miblib" Registration    *~*
*~*   vip_reg_with_oac ~~~~~~~~~~~~~~~ Function for "oac" Registration       *~*
*~*   ncsvip_tbl_req ~~~~~~~~~~~~~~~~~ Function for registering the VIP table*~*
*~*   vip_unreg_with_oac ~~~~~~~~~~~~~ Function to unregister with "oac"     *~*
*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/

#if ( NCS_VIP == 1 )

#include "vip_tbl.h"

/*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~*            *~ Forward Declaration of Extern Functions ~*                 *~*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
EXTERN_C uns32  ncsvipentry_tbl_reg(void);
EXTERN_C uns32 vip_miblib_reg(void);
EXTERN_C uns32 vip_reg_with_oac(IFSV_CB *ifsv_vip_cb);
EXTERN_C uns32 vip_unreg_with_oac(IFSV_CB *ifsv_vip_cb);


/*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~*            *~ Forward Declaration of Static Functions ~*                 *~*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
static uns32 ncsvip_tbl_req(struct ncsmib_arg *args);

/*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~*                  *~ Function Pointer Declarations ~*                     *~*
*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/




typedef uns32 (*VIP_MIBREG_FNC)(void);
/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME: vip_miblib_reg                                           *~*
*~* DESCRIPTION   : MIB LIB Registration                                     *~*
*~* ARGUMENTS     : None                                                     *~*
*~* RETURNS       : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                        *~*
*~* NOTES         :                                                          *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/

uns32 vip_miblib_reg()
{
/*    NCSMIBLIB_REQ_INFO  miblib_init;*/
    uns32               status = NCSCC_RC_SUCCESS;
    VIP_MIBREG_FNC   reg_func = NULL;
                                                                                                      
#if 0
    /* Initalize miblib */
    m_NCS_OS_MEMSET(&miblib_init, 0, sizeof(NCSMIBLIB_REQ_INFO));
                                                                                                      
    /* register with MIBLIB */
    miblib_init.req = NCSMIBLIB_REQ_INIT_OP;
    status = ncsmiblib_process_req(&miblib_init);
    if (status != NCSCC_RC_SUCCESS)
    {
       m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncsmiblib_process_req():vip_miblib_reg() returned failure"," ");
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_MIBLIB_REGISTRATION_FAILURE);
       return status;
    }
    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_MIBLIB_REGISTRATION_SUCCESS);
                                                                                                     
#endif
    /* Register the objects and table data with MIB lib */
    reg_func = ncsvipentry_tbl_reg;/*Generated in mib lib code*/

    if (reg_func() != NCSCC_RC_SUCCESS)
    {
       m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncsvipentry_tbl_reg():vip_miblib_reg() returned failure"," ");
       m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_MIB_TABLE_REGISTRATION_FAILURE);
       return NCSCC_RC_FAILURE;
    }
    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_MIB_TABLE_REGISTRATION_SUCCESS);

    return status;
}

/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME: vip_reg_with_oac                                         *~*
*~* DESCRIPTION   : Registration with OAC                                    *~*
*~* ARGUMENTS     : IFSV_CB *ifsv_vip_cb                                     *~*
*~* RETURNS       : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                        *~*
*~* NOTES         :                                                          *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
uns32 vip_reg_with_oac(IFSV_CB *ifsv_vip_cb)
{
   NCSOAC_SS_ARG   oac_arg;
   NCSMIB_TBL_ID   tbl_id = NCSMIB_TBL_IFSV_VIPTBL;   
   
   m_NCS_OS_MEMSET(&oac_arg, 0, sizeof(NCSOAC_SS_ARG));

   oac_arg.i_oac_hdl = ifsv_vip_cb->oac_hdl;


   /* The VIP MIB table is being resgistered with MAB */
    oac_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
    oac_arg.i_tbl_id = tbl_id;
    oac_arg.info.tbl_owned.is_persistent = FALSE;
    oac_arg.info.tbl_owned.i_mib_req = ncsvip_tbl_req;
   /*Function Pointer to be implemented*/

    oac_arg.info.tbl_owned.i_mib_key = ifsv_vip_cb->cb_hdl;   
    oac_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_IFD;
      
    if (ncsoac_ss(&oac_arg) != NCSCC_RC_SUCCESS)
    {
        m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncsoac_ss():vip_reg_with_oac returned failure"," ");
        m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_OAC_REGISTRATION_FAILURE);
        return NCSCC_RC_FAILURE;       
    }
    m_IFSV_VIP_LOG_MESG(NCS_SERVICE_ID_IFD, IFSV_VIP_OAC_REGISTRATION_SUCCESS);
      
    /* Regiser the Filter for table */
    oac_arg.i_op = NCSOAC_SS_OP_ROW_OWNED; 
    /*For ROW own Operation*/
    oac_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_ANY;
    oac_arg.info.row_owned.i_fltr.fltr.any.meaningless = 0x123;
       
    if (ncsoac_ss(&oac_arg) != NCSCC_RC_SUCCESS)
      {
        m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ncsoac_ss():vip_reg_with_oac returned failure"," ");
        return NCSCC_RC_FAILURE;     
      }
   return NCSCC_RC_SUCCESS;
}

/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME: ncsvip_tbl_req                                           *~*
*~* DESCRIPTION   : VIP TABLE Request function                               *~*
*~* ARGUMENTS     : ncsmib_arg *args                                         *~*
*~* RETURNS       : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                        *~*
*~* NOTES         :                                                          *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/
uns32 ncsvip_tbl_req(struct ncsmib_arg *args)
{
    IFSV_CB           *ifsv_vip_cb = IFSV_CB_NULL;
    uns32               status = NCSCC_RC_SUCCESS;
    NCSMIBLIB_REQ_INFO  miblib_req;
    uns32               cb_hdl = args->i_mib_key;
                                                                                                      
    /* Get the CB from the handle manager */
    if ((ifsv_vip_cb = (IFSV_CB *)ncshm_take_hdl(NCS_SERVICE_ID_IFD,
                                              cb_hdl)) == NULL)
    {
       args->rsp.i_status = NCSCC_RC_NO_INSTANCE;
       args->i_op = m_NCSMIB_REQ_TO_RSP(args->i_op);
       args->i_rsp_fnc(args);
                                                                                                     
       return NCSCC_RC_FAILURE;
    }
                                                                                                     
    m_NCS_OS_MEMSET(&miblib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));
                                                                                                     
    miblib_req.req = NCSMIBLIB_REQ_MIB_OP;
    miblib_req.info.i_mib_op_info.args = args;
    miblib_req.info.i_mib_op_info.cb = ifsv_vip_cb;
                                                                                                     
    m_NCS_LOCK(&ifsv_vip_cb->intf_rec_lock, NCS_LOCK_READ);
    if((args->i_policy & NCSMIB_POLICY_PSS_LAST_PLBCK_TRIGGER) ==
          NCSMIB_POLICY_PSS_LAST_PLBCK_TRIGGER)
    {
       m_NCS_CONS_PRINTF("For TBLONE, last playback update from PSS received...]n");
    }
  /* call the miblib routine handler */
    status = ncsmiblib_process_req(&miblib_req);
                                                                                                     
    m_NCS_UNLOCK(&ifsv_vip_cb->intf_rec_lock, NCS_LOCK_READ);
                                                                                                     
  /* Release IFSV CB handle */
    ncshm_give_hdl(cb_hdl);
                                                                                                     
    return status;
}


/*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*~* PROCEDURE NAME: vip_unreg_with_oac                                       *~*
*~* DESCRIPTION   : Unregistration with OAC                                  *~*
*~* ARGUMENTS     : IFSV_CB *ifsv_vip_cb                                     *~*
*~* RETURNS       : NCSCC_RC_FAILURE/NCSCC_RC_SUCCESS                        *~*
*~* NOTES         :                                                          *~*
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
*/

uns32
vip_unreg_with_oac(IFSV_CB *cb)
{
   NCSOAC_SS_ARG      oac_arg;
   NCSMIB_TBL_ID      tbl_id = NCSMIB_TBL_IFSV_VIPTBL;
                                                                                                                             
   m_NCS_OS_MEMSET(&oac_arg, 0, sizeof(NCSOAC_SS_ARG));
   oac_arg.i_oac_hdl = cb->cb_hdl;
   oac_arg.i_op = NCSOAC_SS_OP_TBL_GONE;
                                                                                                                             
   oac_arg.i_tbl_id = tbl_id;
   if(NCSCC_RC_SUCCESS != ncsoac_ss(&oac_arg))
      return NCSCC_RC_FAILURE;
   return NCSCC_RC_SUCCESS;
}

#endif
