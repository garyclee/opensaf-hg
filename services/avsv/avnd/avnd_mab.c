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

  DESCRIPTION: This module does the initialisation of MAB and provides 
               callback functions at Availability Node Directors. It contains
               the receive callback that is called by MAB for the MIB tables
               related to AVND.
..............................................................................

  FUNCTIONS INCLUDED in this module:


  
******************************************************************************
*/

#include "avnd.h"

typedef  uns32 (* AVND_MIBLIB_REG_FUNC)();
/* Array of function pointers for AVND MIBLIB registration routines
 * for the AVND managed MIB tables. 
 * These are the routines generated by MIBLIB tool and are in the files
 * <table_name>_mib.c file.
 */
static AVND_MIBLIB_REG_FUNC 
   avnd_mib_reg_array[NCSMIB_TBL_AVSV_END - NCSMIB_TBL_AVSV_AVND_BASE + 1] = 
{
   ncssndtableentry_tbl_reg,         /* NCSMIB_TBL_AVSV_NCS_NODE_STAT */
   ncsssutableentry_tbl_reg,         /* NCSMIB_TBL_AVSV_NCS_SU_STAT */
   ncsscomptableentry_tbl_reg,       /* NCSMIB_TBL_AVSV_NCS_COMP_STAT */
   saamfscompcsitableentry_tbl_reg   /* NCSMIB_TBL_AVSV_AMF_COMP_CSI */
};


/***************************************************************************
 * Function :  avnd_tbls_reg_with_mab
 *
 * Purpose  :  This function registers all AVND tables with MAB. 
 *
 * Input    :  cb      :AVND Control Block. 
 *
 * Returns  :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 *
 * NOTES    :  None.
 *
 **************************************************************************/
uns32 avnd_tbls_reg_with_mab(AVND_CB *cb)
{
   NCSOAC_SS_ARG  avnd_oac_arg;
   NCSMIB_TBL_ID  tbl_id;
   uns32          rc = NCSCC_RC_SUCCESS;

   m_NCS_MEMSET(&avnd_oac_arg, 0, sizeof(NCSOAC_SS_ARG));

   avnd_oac_arg.i_oac_hdl = cb->mab_hdl;
   avnd_oac_arg.i_op      = NCSOAC_SS_OP_TBL_OWNED;
   avnd_oac_arg.info.tbl_owned.i_mib_key = (uns64)(cb->cb_hdl);
   avnd_oac_arg.info.tbl_owned.i_mib_req =  avnd_req_mib_func;
   avnd_oac_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_AVND;

   /* Register for all AVND tables */
   for (tbl_id  = NCSMIB_TBL_AVSV_AVND_BASE;
        tbl_id <= NCSMIB_TBL_AVSV_END;
        tbl_id ++)
   {
      avnd_oac_arg.i_tbl_id  = tbl_id;

      if(ncsoac_ss(&avnd_oac_arg) != NCSCC_RC_SUCCESS)
      {
         /* Log Error about skipping the table */
         rc = NCSCC_RC_FAILURE;
         continue;
      }
   } /* end for tbl_id loop. Registration of all tables */

   return rc;
}


/***************************************************************************
 * Function :  avnd_tbls_unreg_with_mab
 *
 * Purpose  :  This function unregisters all AVND tables with MAB. 
 *
 * Input    :  AVND Control Block. 
 *
 * Returns  :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES    :  None.
 * 
 **************************************************************************/
uns32 avnd_tbls_unreg_with_mab(AVND_CB *cb)
{

   NCSOAC_SS_ARG  avnd_oac_arg;
   NCSMIB_TBL_ID  tbl_id;
   uns32          rc = NCSCC_RC_SUCCESS ;

   m_NCS_MEMSET(&avnd_oac_arg, 0, sizeof(NCSOAC_SS_ARG));

   avnd_oac_arg.i_oac_hdl = cb->mab_hdl;
   avnd_oac_arg.i_op      = NCSOAC_SS_OP_TBL_GONE;
 
   /* Unregister for all AVND tables */
   for (tbl_id  = NCSMIB_TBL_AVSV_AVND_BASE;
        tbl_id <= NCSMIB_TBL_AVSV_END;
        tbl_id ++)
   {
      avnd_oac_arg.i_tbl_id  = tbl_id;

      if(ncsoac_ss(&avnd_oac_arg) != NCSCC_RC_SUCCESS)
      {
         /* Log Error about skipping the tables unregistration */
         rc = NCSCC_RC_FAILURE;
         continue;
      }
   } /* end for tbl_id loop. Un registration of all tables */

   return rc;
}


/***************************************************************************
 * Function :  avnd_mab_reg_tbl_rows
 *
 * Purpose  :  This function registers AVND table rows with MAB. 
 *
 * Input    :  cb         AVND Control Block.
 *             tbl_id     The MAB table id for which the row needs
 *                        to be made the owner.
 *             name1_net  ptr to the sa-name (1st index) (n/w order)
 *             name2_net  ptr to the sa-name (2nd index) (n/w order)
 *             node_id    ptr to node id (index)
 *             row_hdl    The row handle returned by MAB.
 *
 * Returns  :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES    :  If NCSCC_RC_SUCCESS row_hdl will contain the handle
 *             returned by MAB.
 * 
 **************************************************************************/
uns32 avnd_mab_reg_tbl_rows(AVND_CB       *cb,
                            NCSMIB_TBL_ID tbl_id,
                            SaNameT       *name1_net,
                            SaNameT       *name2_net,
                            SaClmNodeIdT  *node_id,
                            uns32 *row_hdl)
{
   NCSOAC_SS_ARG avnd_oac_arg;
   NCSMAB_EXACT  exact;
   uns32         idx[2*(SA_MAX_NAME_LENGTH + 1)], i=0, j=0;
   uns32         rc = NCSCC_RC_SUCCESS ;
   uns16         len1 = 0, len2 = 0;

   /* Register the component  row with MAB */
   m_NCS_MEMSET((char *)idx, '\0', sizeof(idx));
   m_NCS_MEMSET((char *)&exact, '\0', sizeof(NCSMAB_EXACT));
   m_NCS_MEMSET(&avnd_oac_arg, 0, sizeof(NCSOAC_SS_ARG));
   
   /* fill 1st index */
   if (name1_net)
   {
      len1 = m_NCS_OS_NTOHS(name1_net->length);
      idx[0] = len1;
      for(i = 0; i < len1; i++)
         idx[i+1] = ((uns32)name1_net->value[i]);
   }
   else if (node_id)
   {
      idx[0] = *node_id;
   }
   
   /* fill 2nd index */
   if (name2_net)
   {
      len2= m_NCS_OS_NTOHS(name2_net->length);
      idx[i+1] = len2;
      j = i + 2;
      for(i = 0; i < len2; i++, j++)
         idx[j] = ((uns32)name2_net->value[i]);
   }

   exact.i_bgn_idx = 0;
   if (name1_net) exact.i_idx_len = len1 + 1;
   if (name2_net) exact.i_idx_len += len2 + 1;
   if (node_id) exact.i_idx_len = 1;
   exact.i_exact_idx = idx;

   /* Register for AVND table rows */
   avnd_oac_arg.i_oac_hdl = cb->mab_hdl;
   avnd_oac_arg.i_op      = NCSOAC_SS_OP_ROW_OWNED;
   avnd_oac_arg.i_tbl_id  = tbl_id;

   avnd_oac_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_EXACT;
   avnd_oac_arg.info.row_owned.i_fltr.is_move_row_fltr = FALSE;
   avnd_oac_arg.info.row_owned.i_fltr.fltr.exact = exact;
   avnd_oac_arg.info.row_owned.i_ss_cb = (NCSOAC_SS_CB)NULL;
   avnd_oac_arg.info.row_owned.i_ss_hdl = cb->cb_hdl;

   if (ncsoac_ss(&avnd_oac_arg) != NCSCC_RC_SUCCESS)
   {
      /* Log Error about skipping the table rows */
      rc = NCSCC_RC_FAILURE;
   }

   *row_hdl = avnd_oac_arg.info.row_owned.o_row_hdl;

   return rc;
}


/***************************************************************************
 * Function :  avnd_mab_unreg_rows
 *
 * Purpose  :  This function unregisters AVND table rows with MAB. 
 *
 * Input    :  AVND Control Block. 
 *             AVND Tbl ID
 *             The row handle of the row that needs to be deleted.
 *
 * Returns  :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES    :  None.
 * 
 **************************************************************************/
uns32 avnd_mab_unreg_tbl_rows(AVND_CB *cb, NCSMIB_TBL_ID tbl_id, uns32 row_hdl)
{
   NCSOAC_SS_ARG  avnd_oac_arg;
   uns32          rc = NCSCC_RC_SUCCESS ;


   m_NCS_MEMSET(&avnd_oac_arg, 0, sizeof(NCSOAC_SS_ARG));

   /* Unregister for AVND table rows */
   avnd_oac_arg.i_oac_hdl = cb->mab_hdl;
   avnd_oac_arg.i_op      = NCSOAC_SS_OP_ROW_GONE;
   avnd_oac_arg.i_tbl_id  = tbl_id;
   avnd_oac_arg.info.row_gone.i_row_hdl = row_hdl;


   if (ncsoac_ss(&avnd_oac_arg) != NCSCC_RC_SUCCESS)
   {
      /* Log Error about skipping the table rows*/
      rc = NCSCC_RC_FAILURE;
   }

   return rc;
}


/***************************************************************************
 * Function :  avnd_mab_unreg_rows
 *
 * Purpose  :  This function unregisters all AVND tables rows with MAB. 
 *
 * Input    :  AVND Control Block. 
 *
 * Returns  :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES    :  None.
 * 
 **************************************************************************/
uns32 avnd_mab_unreg_rows(AVND_CB *cb)
{
   NCSOAC_SS_ARG  avnd_oac_arg;
   NCSMIB_TBL_ID  tbl_id;
   uns32          rc = NCSCC_RC_SUCCESS ;


   m_NCS_MEMSET(&avnd_oac_arg, 0, sizeof(NCSOAC_SS_ARG));

   avnd_oac_arg.i_oac_hdl = cb->mab_hdl;
   avnd_oac_arg.i_op      = NCSOAC_SS_OP_ROW_GONE;

   /* Unregister for all AVND table rows */
   for (tbl_id  = NCSMIB_TBL_AVSV_AVND_BASE;
        tbl_id <= NCSMIB_TBL_AVSV_END;
        tbl_id ++)
   {
      avnd_oac_arg.i_tbl_id  = tbl_id;

      if (ncsoac_ss(&avnd_oac_arg) != NCSCC_RC_SUCCESS)
      {
         /* Log Error about skipping the table rows*/
         rc = NCSCC_RC_FAILURE;
         continue;
      }
   } /* end for tbl_id loop. un registration of all tables rows */

   return rc;
}


/***************************************************************************
 * Function  :  avnd_miblib_init
 *
 * Purpose   :  This routine initializes MIBLIB module. It then
 *              calls all the MIBLIB generated register routines for
 *              all the MIB tables managed by AVND.
 *
 * Input     :  cb - ptr to the AVND control block
 *
 * Returns   :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES     :  None.
 *
 **************************************************************************/
uns32 avnd_miblib_init (AVND_CB *cb)
{
   uns32                status = NCSCC_RC_SUCCESS; 
   NCSMIB_TBL_ID        tbl_id;  
   AVND_MIBLIB_REG_FUNC reg_func = NULL;
   
   /* Register all the AVND tables with MIBLIB */
   for(tbl_id = NCSMIB_TBL_AVSV_AVND_BASE; 
       tbl_id <= NCSMIB_TBL_AVSV_END;
       tbl_id ++)
   {     
       /* Register the objects and table data with MIB lib */
       reg_func = avnd_mib_reg_array[tbl_id - NCSMIB_TBL_AVSV_AVND_BASE];
       if(reg_func == NULL)
          continue;

       if(reg_func() != NCSCC_RC_SUCCESS)
       {
          /* Log error about skipping registering this table */
          status = NCSCC_RC_FAILURE;
          continue;
       }
   } /* End of for loop to register the tables */

   return status; 
}


/*****************************************************************************
 * Function :  avnd_req_mib_func
 *
 * Purpose  :  This function is the handler for MIB request events. This
 *             function calls the MIBLIB API to do the initial validation for
 *             the MIB arg structure received and then calls the appropriate
 *             table handling routine.
 *
 * Input    :  args - pointer the MIB_ARG struct
 *
 * Returns  :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES    :  None.
 * 
 **************************************************************************/
uns32  avnd_req_mib_func(struct ncsmib_arg *args)
{
   NCSMIBLIB_REQ_INFO  miblib_req;
   AVND_CB             *avnd;
   uns32               status = NCSCC_RC_FAILURE;

   /* get the CB from the handle manager */
   if ((avnd = ncshm_take_hdl(NCS_SERVICE_ID_AVND, (uns32)(args->i_mib_key)))
        == AVND_CB_NULL)
   {
      return status;
   }

   m_NCS_MEMSET(&miblib_req, '\0', sizeof(NCSMIBLIB_REQ_INFO)); 

   miblib_req.req = NCSMIBLIB_REQ_MIB_OP; 
   miblib_req.info.i_mib_op_info.args = args;
   miblib_req.info.i_mib_op_info.cb = avnd;
   
   m_NCS_LOCK(&avnd->lock, NCS_LOCK_READ);

   /* call the mib routine handler */ 
   status = ncsmiblib_process_req(&miblib_req);
  
   m_NCS_UNLOCK(&avnd->lock, NCS_LOCK_READ);

   /* give back the handle */
   ncshm_give_hdl((uns32)(args->i_mib_key));
               
   return status;
}
