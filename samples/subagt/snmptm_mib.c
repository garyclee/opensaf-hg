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
 */

/*****************************************************************************
..............................................................................
  MODULE NAME: SNMPTM_MIB.C

..............................................................................
  
  Description:  This module contains processing logic SNMP requests to SNMPTM
                via MIB API

..............................................................................

  snmptm_reg_with_oac ........ Register all the tables with the OAC sybsystem
  snmptm_unreg_with_oac ...... Unreg all the tables from the OAC subsystem
  snmptm_miblib_reg .......... Register all the tables with the OpenSAF MibLib

******************************************************************************
*/

#include "snmptm.h"

#if(NCS_SNMPTM == 1)

/* Used for MOVEROW un-reg with OAC's */
EXTERN_C NCS_BOOL snmptm_finish_unreg;
EXTERN_C NCS_BOOL snmptm_idx_deleted;

/* Array of function pointers for SNMPTM MIB access routines. */
static NCSMIB_REQ_FNC 
   snmptm_mib_request_array[NCSMIB_TBL_SNMPTM_COMMAND - NCSMIB_TBL_SNMPTM_SCALAR + 1] = 
{
   snmptm_scalar_tbl_req,    /* Request func for SCALAR table   */ 
   snmptm_tblone_tbl_req,    /* Request func for TBLONE table   */
   NULL,                     /* Not impletented for TBLTWO      */
   snmptm_tblthree_tbl_req,  /* Request func for TBLTHREE table */
   snmptm_tblfour_tbl_req,   /* Request func for TBLFOUR table  */
   snmptm_tblfive_tbl_req,   /* Request func for TBLFIVE table  */
   snmptm_tblsix_tbl_req,    /* Request func for TBLSIX table   */
   snmptm_tblseven_tbl_req,  /* Request func for TBLSEVEN table */
   snmptm_tbleight_tbl_req,  /* Request func for TBLEIGHT table */
   snmptm_tblnine_tbl_req,   /* Request func for TBLNINE table */
   snmptm_tblten_tbl_req,    /* Request func for TBLTEN table */
   NULL,                     /* Not required for NOTIF table    */
   snmptm_tblcmd_tbl_req,    /* Request func for COMMAND table  */
};

/* Array of function pointers for SNMPTM MIB registration routines, used for 
 * easing switches. The following registration routines will be generated by 
 * SMIDUMP tool automatically upon submitting the corresponding MIB file. 
 * So here the function pointer should match exactly to the generated 
 * registration routine. Any changes to the MIB table name forces to change
 * the below given registration routine names accordingly.
 */
static SNMPTM_MIBREG_FNC 
   snmptm_mib_register_array[NCSMIB_TBL_SNMPTM_TBLTEN - NCSMIB_TBL_SNMPTM_SCALAR + 1] = 
{
   ncstestscalars_tbl_reg,          /* OpenSAF MIBLIB register function for SCALAR table    */
   ncstesttableoneentry_tbl_reg,    /* OpenSAF MibLib register function for TBLONE table    */
   NULL,                            /* Not implemented for TBLTWO                       */
   ncstesttablethreeentry_tbl_reg,  /* OpenSAF MibLib register function for TBLTHREE table  */
   ncstesttablefourentry_tbl_reg,   /* OpenSAF MibLib register function for TBLFOUR table   */
   ncstesttablefiveentry_tbl_reg,   /* OpenSAF MibLib register function for TBLFIVE table   */
   ncstesttablesixentry_tbl_reg,    /* OpenSAF MibLib register function for TBLSIX table    */
   ncstesttablesevenentry_tbl_reg,  /* OpenSAF MibLib register function for TBLSEVEN table  */
   ncstesttableeightentry_tbl_reg,  /* OpenSAF MibLib register function for TBLEIGHT table  */
   ncstesttablenineentry_tbl_reg,   /* OpenSAF MibLib register function for TBLNINE table  */
   ncstesttabletenentry_tbl_reg     /* OpenSAF MibLib register function for TBLTEN table  */
};


/****************************************************************************
  Name          :  snmptm_reg_with_oac

  Description   :  The function registers all the SNMP MIB tables supported
                   by SNMPTM with MAB

  Arguments     :  snmptm - pointer to the SNMPTM CB structure. 
        
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         :
****************************************************************************/
uns32 snmptm_reg_with_oac(SNMPTM_CB* snmptm)
{
   NCSOAC_SS_ARG   oac_arg;
   SNMPTM_TBL_ID   tbl_id;   
   uns32           min_mask[4], max_mask[4];

   /*  Ranges to apply for filters */
   min_mask[0] = 5;
   min_mask[1] = 5;
   min_mask[2] = 5;
   min_mask[3] = 5;

   max_mask[0] = 10;
   max_mask[1] = 10;
   max_mask[2] = 10;
   max_mask[3] = 10;

   /* Traverse through all the SNMPTM MIB tables, resgister them with MAB */
   for(tbl_id = NCSMIB_TBL_SNMPTM_SCALAR; 
       tbl_id <= NCSMIB_TBL_SNMPTM_COMMAND;
       tbl_id ++)
       { 
          if (tbl_id == NCSMIB_TBL_SNMPTM_TBLFOUR)
          {
             snmptm_oac_tblfour_register(snmptm);
             continue;
          }
          else if (tbl_id == NCSMIB_TBL_SNMPTM_TBLSEVEN)
          {
             snmptm_oac_tblseven_register(snmptm);
             continue;
          }
          else if (tbl_id == NCSMIB_TBL_SNMPTM_COMMAND) 
          {
             snmptm_oac_tblcmd_register(snmptm);
             continue;
          }

          if(m_MDS_GET_VDEST_ID_FROM_MDS_DEST(snmptm->vcard) == SNMPTM_VCARD_ID1) {
          m_NCS_OS_MEMSET(&oac_arg, 0, sizeof(NCSOAC_SS_ARG));
          oac_arg.i_oac_hdl = snmptm->oac_hdl;
          oac_arg.i_op = NCSOAC_SS_OP_TBL_OWNED;
          oac_arg.i_tbl_id = tbl_id;
          oac_arg.info.tbl_owned.is_persistent = TRUE;
          oac_arg.info.tbl_owned.i_pcn = (char*)&snmptm->pcn_name;
          oac_arg.info.tbl_owned.i_mib_req = 
                   snmptm_mib_request_array[tbl_id - NCSMIB_TBL_SNMPTM_SCALAR];

          /* This function  ptr will be NULL only when this table is not 
             supposed to be registered by a particular component */

          if (oac_arg.info.tbl_owned.i_mib_req == NULL)
             continue;
      
          oac_arg.info.tbl_owned.i_mib_key = snmptm->hmcb_hdl;   
          oac_arg.info.tbl_owned.i_ss_id = NCS_SERVICE_ID_SNMPTM;
      
          if (ncsoac_ss(&oac_arg) != NCSCC_RC_SUCCESS)
             return NCSCC_RC_FAILURE;       
      
          /* regiser the Filter for each table */
          m_NCS_OS_MEMSET(&oac_arg, 0, sizeof(NCSOAC_SS_ARG));
          oac_arg.i_oac_hdl = snmptm->oac_hdl;
          oac_arg.i_op = NCSOAC_SS_OP_ROW_OWNED; 
          oac_arg.i_tbl_id = tbl_id;
        
          if (tbl_id == NCSMIB_TBL_SNMPTM_TBLTHREE) 
          { 
              /* Apply RANGE filters for TBLTHREE */
              oac_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_RANGE;
              oac_arg.info.row_owned.i_fltr.fltr.range.i_bgn_idx = 0;
              oac_arg.info.row_owned.i_fltr.fltr.range.i_idx_len = 4;
              oac_arg.info.row_owned.i_fltr.fltr.range.i_max_idx_fltr = max_mask;
              oac_arg.info.row_owned.i_fltr.fltr.range.i_min_idx_fltr = min_mask;
          }
          else
          if (tbl_id == NCSMIB_TBL_SNMPTM_TBLFIVE) 
          {
             /* For TBLFIVE, filter properties will be same as of TBLTHREE */ 
             oac_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_SAME_AS;
             oac_arg.info.row_owned.i_fltr.fltr.same_as.i_table_id =
                                             NCSMIB_TBL_SNMPTM_TBLTHREE;
          }
          else
          {
             oac_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_ANY;
             oac_arg.info.row_owned.i_fltr.fltr.any.meaningless = 0x123;
          }
         
          if (ncsoac_ss(&oac_arg) != NCSCC_RC_SUCCESS)
             return NCSCC_RC_FAILURE;     
          }

          /* Construct the warmboot playback request to PSS */ 
          if(m_MDS_GET_VDEST_ID_FROM_MDS_DEST(snmptm->vcard) == SNMPTM_VCARD_ID1)
          {
            NCSOAC_PSS_TBL_LIST lcl_tbl_list;

            m_NCS_OS_MEMSET(&oac_arg, 0, sizeof(NCSOAC_SS_ARG));
            oac_arg.i_op = NCSOAC_SS_OP_WARMBOOT_REQ_TO_PSSV;
            oac_arg.i_oac_hdl = snmptm->oac_hdl;
            oac_arg.i_tbl_id = tbl_id; /* This is not used by PSSv for playback */
            oac_arg.info.warmboot_req.i_pcn = (char*)&snmptm->pcn_name;
            oac_arg.info.warmboot_req.is_system_client = FALSE;

            /* Fill the list of tables to be played-back */
            m_NCS_MEMSET(&lcl_tbl_list, '\0', sizeof(lcl_tbl_list));
            lcl_tbl_list.tbl_id = tbl_id;
            oac_arg.info.warmboot_req.i_tbl_list= &lcl_tbl_list;

            if (ncsoac_ss(&oac_arg) != NCSCC_RC_SUCCESS)
               return NCSCC_RC_FAILURE;       
          }
      
              
       } /* End of for loop to register the tables */

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :   snmptm_unreg_with_oac

  Description   :   The function de registers all the SNMP MIB tables
                    supported by SNMPTM with MAB
  Arguments     :     
        
  Return Values :   NCSCC_RC_SUCCESS /  NCSCC_RC_FAILURE

  Notes:
****************************************************************************/
uns32 snmptm_unreg_with_oac(SNMPTM_CB* snmptm)
{
   NCSOAC_SS_ARG   oac_arg;
   SNMPTM_TBL_ID   tbl_id;   
   
   m_NCS_OS_MEMSET(&oac_arg, 0, sizeof(NCSOAC_SS_ARG));
   
   oac_arg.i_oac_hdl = snmptm->oac_hdl;   
   oac_arg.i_op = NCSOAC_SS_OP_TBL_GONE;     
  
   /* For SNMPTM_VCARD_ID2, only TBLFOUR was registered with OAC */
   if (m_MDS_GET_VDEST_ID_FROM_MDS_DEST(snmptm->vcard) == SNMPTM_VCARD_ID2) 
   {
      snmptm_finish_unreg = TRUE;
      if (snmptm_idx_deleted == TRUE)
      {
         oac_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLFOUR; 
         if (ncsoac_ss(&oac_arg) != NCSCC_RC_SUCCESS)
            return NCSCC_RC_FAILURE;      
      }
   }
   else
   {

      /* Traverse through all the SNMPTM MIB tables, de-register them from MAB */
      for(tbl_id = NCSMIB_TBL_SNMPTM_SCALAR; 
          tbl_id <= NCSMIB_TBL_SNMPTM_COMMAND;
          tbl_id ++)
      {
         if (tbl_id == NCSMIB_TBL_SNMPTM_TBLTWO)
             continue; 

         printf("Unregistering the table-id: %d\n", tbl_id); 
         fflush(stdout); 
         oac_arg.i_tbl_id = tbl_id; 

         if (ncsoac_ss(&oac_arg) != NCSCC_RC_SUCCESS)
            return NCSCC_RC_FAILURE;       
      }
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          :  snmptm_miblib_reg

  Description   :  The function registers SNMPTM with MIBLIB and registers the 
                   SNMPTM tables with MIBLIB.
        
  Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE

  Notes         : 
****************************************************************************/
uns32 snmptm_miblib_reg()
{
   NCSMIBLIB_REQ_INFO  miblib_init; 
   uns32               status = NCSCC_RC_SUCCESS; 
   SNMPTM_TBL_ID       tbl_id;  
   SNMPTM_MIBREG_FNC   reg_func = NULL;
   
   /* Initalize miblib */
   m_NCS_OS_MEMSET(&miblib_init, 0, sizeof(NCSMIBLIB_REQ_INFO));

   /* register with MIBLIB */
   miblib_init.req = NCSMIBLIB_REQ_INIT_OP;
   status = ncsmiblib_process_req(&miblib_init);
   if (status != NCSCC_RC_SUCCESS)
      return status;

   /* Traverse through all the SNMPTM MIB tables, resgister them with 
      OpenSAF MibLib */
   for (tbl_id = NCSMIB_TBL_SNMPTM_SCALAR; 
        tbl_id <= NCSMIB_TBL_SNMPTM_TBLTEN;
        tbl_id ++)
       {     
          /* Register the objects and table data with MIB lib */
          reg_func = snmptm_mib_register_array[tbl_id - NCSMIB_TBL_SNMPTM_SCALAR];
          if (reg_func == NULL)
             continue;

          if (reg_func() != NCSCC_RC_SUCCESS)
             return NCSCC_RC_FAILURE;
          
       } /* End of for loop to register the tables */

   return status; 
}

#else /* NCS_SNMPTM */
extern int dummy;
#endif /* NCS_SNMPTM */

