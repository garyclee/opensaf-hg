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


#include "gld.h"

static uns32 gld_mib_tbl_req(struct ncsmib_arg *args);
/******************************************************************************
  Function :  gld_mib_tbl_req

  Purpose  :  High Level MIB Access Routine for Interface MIB table

  Input    :  struct ncsmib_arg*

  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE

  Notes    :
******************************************************************************/

uns32 gld_mib_tbl_req(struct ncsmib_arg *args)
{
   GLSV_GLD_CB        *cb = NULL;
   uns32              rc = NCSCC_RC_SUCCESS;
   NCSMIBLIB_REQ_INFO  miblib_req;

   cb = ncshm_take_hdl(NCS_SERVICE_ID_GLD, args->i_mib_key);

   if (cb == NULL)
       return NCSCC_RC_FAILURE;

   m_NCS_OS_MEMSET(&miblib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

   miblib_req.req = NCSMIBLIB_REQ_MIB_OP;
   miblib_req.info.i_mib_op_info.args = args;
   miblib_req.info.i_mib_op_info.cb = cb;

   /*  call the mib routine handler  */
   rc = ncsmiblib_process_req(&miblib_req);

   ncshm_give_hdl((uns32)args->i_mib_key);

   return rc;
}

/****************************************************************************
  PROCEDURE NAME:   gld_reg_with_mab

  DESCRIPTION:      The function registers all the SNMP MIB tables supported
                    by GLD with MAB
  ARGUMENTS:

  RETURNS:
        NCSCC_RC_SUCCESS
        NCSCC_RC_FAILURE

  NOTES:
*****************************************************************************/
uns32 gld_reg_with_mab(GLSV_GLD_CB *cb)
{
   NCSOAC_SS_ARG      mab_arg;
   NCSMIB_TBL_ID      tbl_id;

   m_NCS_OS_MEMSET(&mab_arg, 0, sizeof(NCSOAC_SS_ARG));
   mab_arg.i_oac_hdl = cb->oac_hdl;

  for(tbl_id = NCSMIB_TBL_GLSV_MIB_BASE;
       tbl_id <= NCSMIB_TBL_GLSV_RSCTBL;
       tbl_id ++)
   {
       mab_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
       mab_arg.i_tbl_id = tbl_id;
       mab_arg.info.tbl_owned.i_mib_req = gld_mib_tbl_req;
       mab_arg.info.tbl_owned.i_mib_key = cb->my_hdl;

       mab_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_GLD;

       if (NCSCC_RC_SUCCESS != ncsoac_ss(&mab_arg))
           return NCSCC_RC_FAILURE;
         /* register the Filter for each table */
          mab_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;
          mab_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_ANY;
          mab_arg.info.row_owned.i_fltr.fltr.any.meaningless = 0x123;

          if(NCSCC_RC_SUCCESS != ncsoac_ss(&mab_arg))
            return NCSCC_RC_FAILURE;
   }
       return NCSCC_RC_SUCCESS;
}



/****************************************************************************
  PROCEDURE NAME:   gld_unreg_with_mab

  DESCRIPTION:      The function de registers all the SNMP MIB tables supported
                    by CPSV with MAB
  ARGUMENTS:

  RETURNS:
        NCSCC_RC_SUCCESS
        NCSCC_RC_FAILURE

  NOTES:
*****************************************************************************/
uns32 gld_unreg_with_mab(GLSV_GLD_CB *cb)
{
    NCSOAC_SS_ARG      mab_arg;
    NCSMIB_TBL_ID      tbl_id;

    m_NCS_OS_MEMSET(&mab_arg, 0, sizeof(NCSOAC_SS_ARG));
    mab_arg.i_oac_hdl = cb->my_hdl;
    mab_arg.i_op = NCSOAC_SS_OP_TBL_GONE;

    for(tbl_id = NCSMIB_TBL_GLSV_MIB_BASE;
        tbl_id <= NCSMIB_TBL_GLSV_RSCTBL;
        tbl_id ++)
    {
        mab_arg.i_tbl_id = tbl_id;
        if(NCSCC_RC_SUCCESS != ncsoac_ss(&mab_arg))
            return NCSCC_RC_FAILURE;
    }
    return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name        : gld_reg_with_miblib
 *
 * Description : To register with miblib
 *
 * Arguments   :
 *
****************************************************************************/
uns32 gld_reg_with_miblib()
{
   uns32 rc;

   rc = saflckscalarobject_tbl_reg();
   if (rc!= NCSCC_RC_SUCCESS)
      return rc;

   rc = salckresourceentry_tbl_reg();
   if (rc != NCSCC_RC_SUCCESS)
       return rc;

   return NCSCC_RC_SUCCESS;
}
