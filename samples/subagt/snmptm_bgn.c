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
  MODULE NAME: SNMPTM_BGN.C

..............................................................................

  DESCRIPTION:   SNMPTM demo file, contains the procedures to initialise MDS,
                 OAC to make an SNMPTM TEST_MIB appl as a stand alone system.
******************************************************************************
*/
#include "snmptm.h"

EXTERN_C uns32  g_snmptm_hdl;
NCS_BOOL gl_snmptm_eda_init = FALSE; /* Global */
NCS_BOOL gl_snmptm_pssv_demo = FALSE; /* Global */
uns32 gl_snmptm_role = SA_AMF_HA_ACTIVE; /* Default STATE */ 

/* MDS related global params */
static uns32      gl_snmptm_oac_hdl;
static MDS_DEST   gl_snmptm_vdest;
static NCSCONTEXT gl_mds_vdest;

/* Static function prototypes */
static uns32 snmptm_create_vdest();
static uns32 snmptm_role_change(uns32 role) ;
static void snmptm_state_cahnge(int number); 
static uns32 snmptm_agt_init(int argc, char **argv);
static void snmptm_exec_test1(SNMPTM_CB *cb);
static void snmptm_exec_test2(SNMPTM_CB *cb);
static void snmptm_exec_test3(SNMPTM_CB *cb);
static void snmptm_exec_test5(SNMPTM_CB *cb);
static char snmptm_util_get_char_option(int argc, char *argv[], char *arg_prefix);
static char *snmptm_util_search_argv_list(int argc, char *argv[], char *arg_prefix);

/****************************************************************************
 Function Name :  snmptm_agt_init

 Purpose       :  Initialises Agents, will be used by OAC which process SNMP 
                  operations on SNMPTM (SNMP TEST_MIB) application. Can be 
                  removed from here once the demo init takes care of 
                  initialising MDS.
   
 Arguments     : 

 Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
****************************************************************************/
static uns32 snmptm_agt_init(int argc, char **argv)
{

   if ('y' == snmptm_util_get_char_option(argc, argv, "EDSV=")) 
   {
      gl_snmptm_eda_init = TRUE;
   }
   if ('y' == snmptm_util_get_char_option(argc, argv, "PSSV=")) 
   {
      gl_snmptm_pssv_demo = TRUE;
   }
   
   if ('y' == snmptm_util_get_char_option(argc, argv, "STANDBY=")) 
   {
      gl_snmptm_role = SA_AMF_HA_STANDBY;
   }
   
   if (ncs_agents_startup(argc, argv) != NCSCC_RC_SUCCESS)
      return NCSCC_RC_FAILURE;

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 Function Name :  snmptm_util_get_char_option

 Purpose       :  Lookup for a particular string value in the input arguments
   
 Arguments     : 

 Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
****************************************************************************/
static char snmptm_util_get_char_option(int argc, char *argv[], char *arg_prefix)
{
   char char_option;
   char                 *p_field;

   p_field = snmptm_util_search_argv_list(argc, argv, arg_prefix);
   if (p_field == NULL)
   {
      return 0;
   }
   if (sscanf(p_field + strlen(arg_prefix), "%c", &char_option) != 1)
   {
      return 0;
   }
   if (isupper(char_option))
      char_option = (char)tolower(char_option);

   return char_option;
}

static char *snmptm_util_search_argv_list(int argc, char *argv[], char *arg_prefix)
{     
   char *tmp;
         
   /* Check   argv[argc-1] through argv[1] */
   for ( ; argc > 1; argc -- )
   {  
      tmp = strstr(argv[argc-1], arg_prefix);
      if (tmp != NULL)
         return tmp;
   }  
   return NULL;
}  




/****************************************************************************
 Function Name :  snmptm_create_vdest

 Purpose       :  Invoked as part of VDA API invocation to create Virtual 
                  SNMPTM-DEST from the "unnamed" range. Also it gets an OAC
                  handle, OAC handle is required to pass down to the SNMPTM
                  application such that SNMPTM register/de-registers its MIBs
                  with MAB/OAC.
   
 Arguments     : 

 Return Values :  NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
****************************************************************************/
static uns32 snmptm_create_vdest()
{
   NCSVDA_INFO vda_info;
   uns32        status; 

   memset(&vda_info, 0, sizeof(vda_info));

   vda_info.req = NCSVDA_VDEST_CREATE;
   vda_info.info.vdest_create.i_create_type = NCSVDA_VDEST_CREATE_SPECIFIC;
   vda_info.info.vdest_create.i_create_oac = TRUE;
   vda_info.info.vdest_create.i_persistent = TRUE; /* Up-to-the application */
   vda_info.info.vdest_create.i_policy = NCS_VDEST_TYPE_DEFAULT;
   vda_info.info.vdest_create.info.specified.i_vdest = gl_snmptm_vdest;
  
   if (ncsvda_api(&vda_info) != NCSCC_RC_SUCCESS)
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   gl_snmptm_oac_hdl = vda_info.info.vdest_create.o_pwe1_oac_hdl;
   gl_mds_vdest = NCS_UNS32_TO_PTR_CAST(vda_info.info.vdest_create.o_mds_pwe1_hdl);

   memset(&vda_info,'\0',sizeof(NCSVDA_INFO));

   status = snmptm_role_change(gl_snmptm_role); 
   if (status != NCSCC_RC_SUCCESS)
       printf("SNMPTM_DEMO Role change failed\n"); 

    return status; 
}

static uns32 snmptm_role_change(uns32 role) 
{
   NCSVDA_INFO vda_info;
   char state[30]= {0}; 

   memset(&vda_info, 0, sizeof(vda_info));
    
   /* set the role of the vdest */
   vda_info.req = NCSVDA_VDEST_CHG_ROLE;
   vda_info.info.vdest_chg_role.i_new_role = role;  
   vda_info.info.vdest_chg_role.i_vdest = gl_snmptm_vdest;

   if (ncsvda_api(&vda_info) != NCSCC_RC_SUCCESS)
   {
      memset(&vda_info,'\0',sizeof(NCSVDA_INFO));
      vda_info.req = NCSVDA_VDEST_DESTROY;
      vda_info.info.vdest_destroy.i_vdest = gl_snmptm_vdest;
      ncsvda_api(&vda_info);
      return NCSCC_RC_FAILURE;
   }
   
   if (role == SA_AMF_HA_ACTIVE)
       sprintf(state, "%s", "ACTIVE"); 
   else
       sprintf(state, "%s", "STANDBY"); 
       
   printf ("\n =================================================================="); 
   printf ("\n|                 SNMPTM_DEMO in %s mode                        |", state); 
   printf ("\n =================================================================="); 
   fflush(stdout); 
                                                                             
   return NCSCC_RC_SUCCESS;
}

static void snmptm_state_cahnge(int number) 
{
    if (gl_snmptm_role == SA_AMF_HA_ACTIVE)
    {
        gl_snmptm_role = SA_AMF_HA_STANDBY; 
    }
    else if (gl_snmptm_role == SA_AMF_HA_STANDBY)
    {
        gl_snmptm_role = SA_AMF_HA_ACTIVE; 
    }

    /* do the role change now! */ 
    snmptm_role_change(gl_snmptm_role); 

    /* install the signal handler again */
    signal (SIGUSR1, snmptm_state_cahnge); 
    return; 
}
        

/****************************************************************************
 Function Name:  snmptm_main
 
 Purpose      :  This is the main function for SNMPTM demo, called once the 
                 global initialisation (eg. like MDS init)is done by 
                 ncs_main_demo. It initialises SNMPTM application viz..
                 initialises the required data of its own and registers the
                 MIB tables with OAC (MAB) & with  OpenSAF MibLib. 

 Arguments    : 

 Return Value :  None

 Note         :  This demo only builds and works in INTERNAL TASKING mode.

****************************************************************************/
void  snmptm_main(int argc, char **argv)
{
   SNMPTM_TBLSIX_KEY  tblsix_key;
   SNMPTM_TBLSIX *tblsix = NULL;
   SNMPTM_TBLTEN_KEY  tblten_key;
   SNMPTM_TBLTEN *tblten = NULL;
   NCSSNMPTM_LM_REQUEST_INFO  lm_req;  /* Layer management structure to be
                                    populated with the initialisation data */

   /* Memset lm_req struct to 0 */       
   memset(&lm_req, 0, sizeof(NCSSNMPTM_LM_REQUEST_INFO));

   /* Initialise OpenSAF agents, can be removed from here once MDS initialisation 
      is taken care  globally by demo initialisation just before calling
      snmptm_main() */
   if (snmptm_agt_init(argc, argv) != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("\nSNMPTM ERROR: Not able to initialize Agents\n"); 
      return;
   }

   /* Set the type of LM request */
   lm_req.i_req = NCSSNMPTM_LM_REQ_SNMPTM_CREATE;

   /* Always the first arguements should be vcard_id */
   if (*argv[1] == '1')
   {
      m_NCS_SET_VDEST_ID_IN_MDS_DEST(gl_snmptm_vdest, SNMPTM_VCARD_ID1);
      lm_req.io_create_snmptm.i_node_id = 1;
      strcpy(&lm_req.io_create_snmptm.i_pcn, "SNMPTM_NODE_1");
   }
   else
   {
      m_NCS_SET_VDEST_ID_IN_MDS_DEST(gl_snmptm_vdest, SNMPTM_VCARD_ID2);
      lm_req.io_create_snmptm.i_node_id = (uns8)atoi(argv[1]);
      strcpy(&lm_req.io_create_snmptm.i_pcn, "SNMPTM_NODE_2");
   }

   m_NCS_CONS_PRINTF("\nSNMPTM Node Id: %d \n", lm_req.io_create_snmptm.i_node_id); 

   if (snmptm_create_vdest() != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("\nSNMPTM ERROR: Not able to create VDEST \n"); 
      return;
   }

   /* install the signal handler; for a role change */ 
   signal (SIGUSR1, snmptm_state_cahnge); 

   /* Update the OAC hdl in LM req */
   lm_req.io_create_snmptm.i_oac_hdl = gl_snmptm_oac_hdl;
   lm_req.io_create_snmptm.i_mds_vdest = gl_mds_vdest;
   lm_req.io_create_snmptm.i_vdest = gl_snmptm_vdest;

   if (! lm_req.io_create_snmptm.i_oac_hdl)
   {
      m_NCS_CONS_PRINTF("\nSNMPTM ERROR: Not able to get the OAC handle\n"); 
   }
 
   /* Handle manager pool_id, set to NCS_HM_POOL_ID_EXTERNAL1 , usually the 
      related pool-id is defined/retrieved from the common pool defined in
      the header file ncs_hdl_pub.h. 
   */ 
   lm_req.io_create_snmptm.i_hmpool_id = NCS_HM_POOL_ID_EXTERNAL1;  
   
   /* Call SNMPTM LM request API, this creates the SNMPTM CB and does
      the initialisation/registration process that are required for
      SNMPTM application */ 
   m_NCS_CONS_PRINTF("\nCreating SNMPTM Instance[Node=%d]..\n", lm_req.io_create_snmptm.i_node_id); 
   if (ncssnmptm_lm_request(&lm_req) != NCSCC_RC_SUCCESS)
   {
      /* Failed to create SNMPTM instance */
      m_NCS_CONS_PRINTF("\nFAILED!\n"); 
      return;
   }
 
   m_NCS_TASK_SLEEP(5000); 
   
   /* To demonstrate the integration of this demo with PSSv */ 
   if(gl_snmptm_pssv_demo && (lm_req.io_create_snmptm.i_node_id == 1))
   {
      SNMPTM_CB *lcl_snmptm = SNMPTM_CB_NULL;

      /* Send the dynamic data(i.e., the 10 MIB rows of TBLSIX) to PSS */
      if ((lcl_snmptm = (SNMPTM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SNMPTM,
                                             g_snmptm_hdl)) != NULL)
      {
         m_NCS_CONS_PRINTF("\nStarting TEST-1 on Node-1: To send dynamic data, i.e., 10 rows of TBLSIX to PSSv!\n"); 
         snmptm_exec_test1(lcl_snmptm);
         ncshm_give_hdl(g_snmptm_hdl);
      }

      /* Wait for sometime, and then delete 2 rows from TBLSIX. Send the 
        update to PSS */
      m_NCS_TASK_SLEEP(3000); 
      if ((lcl_snmptm = (SNMPTM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SNMPTM,
                                             g_snmptm_hdl)) != NULL)
      {
         m_NCS_CONS_PRINTF("\nStarting TEST-2 on Node-1: To delete 2 rows of TBLSIX, and send the update to PSSv!\n"); 
         snmptm_exec_test2(lcl_snmptm);
         ncshm_give_hdl(g_snmptm_hdl);
      }

      /* Wait for sometime, and then modify 3 rows of TBLSIX. Send the 
         update to PSS */
      m_NCS_TASK_SLEEP(3000); 
      if ((lcl_snmptm = (SNMPTM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SNMPTM,
                                             g_snmptm_hdl)) != NULL)
      {
         m_NCS_CONS_PRINTF("\nStarting TEST-3 on Node-1: To modify 3 rows of TBLSIX, and send the udpate to PSSv!\n"); 
         memset(&tblsix_key, '\0', sizeof(tblsix_key));
         tblsix_key.count = 2;
         tblsix = (SNMPTM_TBLSIX*)ncs_patricia_tree_get(&lcl_snmptm->tblsix_tree,
                                                  (uns8*)&tblsix_key);
         tblsix->tblsix_data = 103;

         memset(&tblsix_key, '\0', sizeof(tblsix_key));
         tblsix_key.count = 5;
         tblsix = (SNMPTM_TBLSIX*)ncs_patricia_tree_get(&lcl_snmptm->tblsix_tree,
                                                  (uns8*)&tblsix_key);
         tblsix->tblsix_data = 106;

         memset(&tblsix_key, '\0', sizeof(tblsix_key));
         tblsix_key.count = 6;
         tblsix = (SNMPTM_TBLSIX*)ncs_patricia_tree_get(&lcl_snmptm->tblsix_tree,
                                                  (uns8*)&tblsix_key);
         tblsix->tblsix_data = 107;
         snmptm_exec_test3(lcl_snmptm);
         ncshm_give_hdl(g_snmptm_hdl);
      }

      /* Wait for sometime, and then delete all the rows from TBLSIX. Send a 
         warmboot request for this particular table to PSS */
      m_NCS_TASK_SLEEP(3000); 
      if ((lcl_snmptm = (SNMPTM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SNMPTM,
                                             g_snmptm_hdl)) != NULL)
      {
         NCSOAC_PSS_TBL_LIST lcl_tbl_list;
         NCSOAC_SS_ARG oac_arg;

         m_NCS_CONS_PRINTF("\nStarting TEST-4 on Node-1: To delete all rows of TBLSIX, and request for warmboot playback from PSSv!\n"); 
         while(SNMPTM_TBLSIX_NULL != (tblsix = (SNMPTM_TBLSIX*)
                        ncs_patricia_tree_getnext(&lcl_snmptm->tblsix_tree, 0)))
         {
            /* Delete the node from the tblsix tree */
            ncs_patricia_tree_del(&lcl_snmptm->tblsix_tree, &tblsix->tblsix_pat_node);

            /* Free the memory */
            m_MMGR_FREE_SNMPTM_TBLSIX(tblsix);
         }
         /* Compose Warmboot-playback request to PSS */
         memset(&oac_arg, '\0', sizeof(oac_arg));
         oac_arg.i_op = NCSOAC_SS_OP_WARMBOOT_REQ_TO_PSSV;
         oac_arg.i_oac_hdl = lcl_snmptm->oac_hdl;   
         oac_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLSIX; /* This is not used by PSSv for playback */
         oac_arg.info.warmboot_req.i_pcn = (char*)&lcl_snmptm->pcn_name;
         oac_arg.info.warmboot_req.is_system_client = FALSE;

         /* Fill the list of tables to be played-back */
         memset(&lcl_tbl_list, '\0', sizeof(lcl_tbl_list));
         lcl_tbl_list.tbl_id = NCSMIB_TBL_SNMPTM_TBLSIX;
         oac_arg.info.warmboot_req.i_tbl_list= &lcl_tbl_list;

         if (ncsoac_ss(&oac_arg) != NCSCC_RC_SUCCESS)
         {
            ncshm_give_hdl(g_snmptm_hdl);
            return ;       
         }
         ncshm_give_hdl(g_snmptm_hdl);
      }

      m_NCS_TASK_SLEEP(10000); 
      if ((lcl_snmptm = (SNMPTM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SNMPTM,
                                             g_snmptm_hdl)) != NULL)
      {
         m_NCS_CONS_PRINTF("\nVerifying TEST-4 on Node-1: Dump of TBLSIX data, after waiting for 10 secs...\n\n"); 
         memset(&tblsix_key, '\0', sizeof(tblsix_key));
         while(SNMPTM_TBLSIX_NULL != (tblsix = (SNMPTM_TBLSIX*)
                        ncs_patricia_tree_getnext(&lcl_snmptm->tblsix_tree, (uns8*)&tblsix_key)))
         {
            tblsix_key.count = tblsix->tblsix_key.count;
            m_NCS_CONS_PRINTF("\tTBLSIX entry after warmboot playback: Index = %d, data = %d, Name = %s!\n",
               tblsix->tblsix_key.count, tblsix->tblsix_data, (char*)&tblsix->tblsix_name); 
         }
         ncshm_give_hdl(g_snmptm_hdl);
      }

      /* Send the dynamic data(i.e., the 2 MIB rows of TBLTEN) to PSS */
      if ((lcl_snmptm = (SNMPTM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SNMPTM,
                                             g_snmptm_hdl)) != NULL)
      {
         m_NCS_CONS_PRINTF("\nStarting TEST-5 on Node-1: To send dynamic data, i.e., 2 rows of TBLTEN to PSSv!\n"); 
         snmptm_exec_test5(lcl_snmptm);
         ncshm_give_hdl(g_snmptm_hdl);
      }

      /* Wait for sometime, and then delete all the rows from TBLTEN. Send a 
         warmboot request for this particular table to PSS */
      m_NCS_TASK_SLEEP(3000); 
      if ((lcl_snmptm = (SNMPTM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SNMPTM,
                                             g_snmptm_hdl)) != NULL)
      {
         NCSOAC_PSS_TBL_LIST lcl_tbl_list;
         NCSOAC_SS_ARG oac_arg;

         m_NCS_CONS_PRINTF("\nStarting TEST-6 on Node-1: To delete all rows of TBLTEN, and request for warmboot playback from PSSv!\n"); 
         while(SNMPTM_TBLTEN_NULL != (tblten = (SNMPTM_TBLTEN*)
                        ncs_patricia_tree_getnext(&lcl_snmptm->tblten_tree, 0)))
         {
            /* Delete the node from the tblten tree */
            ncs_patricia_tree_del(&lcl_snmptm->tblten_tree, &tblten->tblten_pat_node);

            /* Free the memory */
            m_MMGR_FREE_SNMPTM_TBLTEN(tblten);
         }
         /* Compose Warmboot-playback request to PSS */
         memset(&oac_arg, '\0', sizeof(oac_arg));
         oac_arg.i_op = NCSOAC_SS_OP_WARMBOOT_REQ_TO_PSSV;
         oac_arg.i_oac_hdl = lcl_snmptm->oac_hdl;   
         oac_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLTEN; /* This is not used by PSSv for playback */
         oac_arg.info.warmboot_req.i_pcn = (char*)&lcl_snmptm->pcn_name;
         oac_arg.info.warmboot_req.is_system_client = FALSE;

         /* Fill the list of tables to be played-back */
         memset(&lcl_tbl_list, '\0', sizeof(lcl_tbl_list));
         lcl_tbl_list.tbl_id = NCSMIB_TBL_SNMPTM_TBLTEN;
         oac_arg.info.warmboot_req.i_tbl_list= &lcl_tbl_list;

         if (ncsoac_ss(&oac_arg) != NCSCC_RC_SUCCESS)
         {
            ncshm_give_hdl(g_snmptm_hdl);
            return ;       
         }
         ncshm_give_hdl(g_snmptm_hdl);
      }

      m_NCS_TASK_SLEEP(5000); 
      if ((lcl_snmptm = (SNMPTM_CB *)ncshm_take_hdl(NCS_SERVICE_ID_SNMPTM,
                                             g_snmptm_hdl)) != NULL)
      {
         m_NCS_CONS_PRINTF("\nVerifying TEST-7 on Node-1: Dump of TBLTEN data, after waiting for 5 secs...\n\n"); 
         memset(&tblten_key, '\0', sizeof(tblten_key));
         while(SNMPTM_TBLTEN_NULL != (tblten = (SNMPTM_TBLTEN*)
                        ncs_patricia_tree_getnext(&lcl_snmptm->tblten_tree, (uns8*)&tblten_key)))
         {
            tblten_key.tblten_unsigned32 = tblten->tblten_key.tblten_unsigned32;
            tblten_key.tblten_int = tblten->tblten_key.tblten_int;
            m_NCS_CONS_PRINTF("\tTBLTEN entry after warmboot playback: Index-uns32 = %d, Index-int = %d!\n",
               tblten->tblten_key.tblten_unsigned32, tblten->tblten_key.tblten_int);
         }
         ncshm_give_hdl(g_snmptm_hdl);
      }

      m_NCS_CONS_PRINTF("\nIntegration of the demo node-1 with Persistency service(PSSv) complete...\n\n"); 

  } /* if(snmptm->node_id == 1) */

   /* S l e e p      F o r e v e r */ 
   while(1)
   {
      m_NCS_TASK_SLEEP(5000); 
   }
   
   return;
}

/* Update all rows of TBLSIX to PSS */
static void snmptm_exec_test1(SNMPTM_CB *cb)
{
   SNMPTM_TBLSIX *tblsix = NULL;
   SNMPTM_TBLSIX_KEY lcl_key;
   uns32   inst_ids[1];   /* For TBLSIX, only INDEX parameter */
   uns8   space[2048];
   NCSMEM_AID ma;
   uns32 xch_id = 1;
   NCSROW_AID ra;
   NCSPARM_AID pa;
   NCSMIB_IDX idx;
   NCSMIB_PARAM_VAL pv;
   NCSOAC_SS_ARG mab_arg;
   NCSMIB_ARG arg;
   uns32 read_cnt = 0;
   char name[9];
   uns32 evt_type = 2; /* Default, SETROW for TBLSIX */

   memset(&lcl_key, '\0', sizeof(lcl_key));
   memset(&arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&arg);
   ncsmem_aid_init(&ma, space, sizeof(space));
   arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLSIX;
   arg.i_mib_key = 0; /* Not required */
   arg.i_usr_key = 0; /* Not required */
   arg.i_rsp_fnc = NULL;
   arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;

   while((tblsix = (SNMPTM_TBLSIX*)ncs_patricia_tree_getnext(&cb->tblsix_tree, (uns8*)&lcl_key)) != NULL)
   {
      read_cnt ++;
      lcl_key.count = tblsix->tblsix_key.count;
      memset(&name, '\0', sizeof(name));
      pv.i_fmat_id = NCSMIB_FMAT_INT;
      pv.i_param_id = ncsTestTableSixData_ID;
      pv.info.i_int = tblsix->tblsix_data;

      if(evt_type == 1)
      {
         /* Compose MIB SET event */
         arg.i_op = NCSMIB_OP_RSP_SET;
         arg.i_xch_id = xch_id++;
         inst_ids[0] = lcl_key.count;
         arg.i_idx.i_inst_len = 1;
         arg.rsp.info.set_rsp.i_param_val = pv;

         memset(&mab_arg,0,sizeof(NCSOAC_SS_ARG));
         mab_arg.i_op = NCSOAC_SS_OP_PUSH_MIBARG_DATA_TO_PSSV;
         mab_arg.i_oac_hdl = cb->oac_hdl;
         mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLSIX;
         mab_arg.info.push_mibarg_data.arg = &arg;
         if (ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
         {
            return ;       
         }

         pv.i_fmat_id = NCSMIB_FMAT_OCT;
         pv.i_param_id = ncsTestTableSixName_ID;
         pv.i_length = m_NCS_STRLEN((char*)&name);
         pv.info.i_oct = (uns8*)((char*)&name);
         arg.i_op = NCSMIB_OP_REQ_SET;
         arg.i_xch_id = xch_id++;
         inst_ids[0] = lcl_key.count;
         arg.i_idx.i_inst_len = 1;
         arg.rsp.info.set_rsp.i_param_val = pv;

         memset(&mab_arg,0,sizeof(NCSOAC_SS_ARG));
         mab_arg.i_op = NCSOAC_SS_OP_PUSH_MIBARG_DATA_TO_PSSV;
         mab_arg.i_oac_hdl = cb->oac_hdl;
         mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLSIX;
         mab_arg.info.push_mibarg_data.arg = &arg;
         if (ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
         {
            return ;       
         }
      }
      else if(evt_type == 2)
      {
         /* Compose MIB SETROW event */
         arg.i_op = NCSMIB_OP_RSP_SETROW;
         arg.i_xch_id = xch_id++;
         arg.i_idx.i_inst_len = 1;
         inst_ids[0] = lcl_key.count;

         ncsparm_enc_init(&pa);
         ncsparm_enc_param(&pa, &pv);

         pv.i_fmat_id = NCSMIB_FMAT_OCT;
         pv.i_param_id = ncsTestTableSixName_ID;
         pv.i_length = m_NCS_STRLEN((char*)&tblsix->tblsix_name);
         pv.info.i_oct = (uns8*)((char*)&tblsix->tblsix_name);
         ncsparm_enc_param(&pa, &pv);

         /* Compose USRBUF */
         arg.rsp.info.setrow_rsp.i_usrbuf = ncsparm_enc_done(&pa);
         m_BUFR_STUFF_OWNER(arg.req.info.setrow_rsp.i_usrbuf);

         memset(&mab_arg,0,sizeof(NCSOAC_SS_ARG));
         mab_arg.i_op = NCSOAC_SS_OP_PUSH_MIBARG_DATA_TO_PSSV;
         mab_arg.i_oac_hdl = cb->oac_hdl;
         mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLSIX;
         mab_arg.info.push_mibarg_data.arg = &arg;
         if (ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
         {
            return ;       
         }
         if(arg.rsp.info.setrow_rsp.i_usrbuf != NULL)
            m_MMGR_FREE_BUFR_LIST(\
                            arg.rsp.info.setrow_rsp.i_usrbuf);
      }
      else
      {
         /* Default - Compose MIB SETALLROWS event */
         ncssetallrows_enc_init(&ra);
         ncsrow_enc_init(&ra);

         inst_ids[0] = lcl_key.count;
         idx.i_inst_ids = inst_ids;
         ncsrow_enc_inst_ids(&ra, &idx);
         ncsrow_enc_param(&ra, &pv);

         pv.i_fmat_id = NCSMIB_FMAT_INT;
         pv.i_param_id = ncsTestTableSixData_ID;
         pv.info.i_int = tblsix->tblsix_data;
         ncsrow_enc_param(&ra, &pv);

         pv.i_fmat_id = NCSMIB_FMAT_OCT;
         pv.i_param_id = ncsTestTableSixName_ID;
         pv.i_length = m_NCS_STRLEN((char*)&tblsix->tblsix_name);
         pv.info.i_oct = (uns8*)((char*)&tblsix->tblsix_name);
         ncsrow_enc_param(&ra, &pv);
         ncsrow_enc_done(&ra);
      }
   }  /* while loop completes here */

   if((read_cnt != 0) && ((evt_type != 1) && (evt_type != 2)))
   {
      /* Send the already composed SETALLROWS event */
      arg.i_xch_id = xch_id;
      arg.i_idx.i_inst_len = 1;
      arg.i_op = NCSMIB_OP_RSP_SETALLROWS;
      arg.rsp.info.setallrows_rsp.i_usrbuf = ncssetallrows_enc_done(&ra);
      m_BUFR_STUFF_OWNER(arg.rsp.info.setallrows_rsp.i_usrbuf);
      arg.i_idx.i_inst_len = idx.i_inst_len;
      arg.i_idx.i_inst_ids = idx.i_inst_ids;

      memset(&mab_arg,0,sizeof(NCSOAC_SS_ARG));
      mab_arg.i_op = NCSOAC_SS_OP_PUSH_MIBARG_DATA_TO_PSSV;
      mab_arg.i_oac_hdl = cb->oac_hdl;
      mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLSIX;
      mab_arg.info.push_mibarg_data.arg = &arg;
      if (ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
      {
         return ;       
      }

      if(arg.rsp.info.setallrows_rsp.i_usrbuf != NULL)
         m_MMGR_FREE_BUFR_LIST(\
                       arg.rsp.info.setallrows_rsp.i_usrbuf);
   }

   return;
}

/* Update delete-rows information of TBLSIX to PSS */
static void snmptm_exec_test2(SNMPTM_CB *cb)
{
   SNMPTM_TBLSIX *tblsix = NULL;
   SNMPTM_TBLSIX_KEY lcl_key;
   uns32   inst_ids[1];   /* For TBLSIX, only INDEX parameter */
   uns8   space[2048];
   NCSMEM_AID ma;
   uns32 xch_id = 1;
   NCSMIB_IDX idx;
   NCSOAC_SS_ARG mab_arg;
   NCSREMROW_AID rra;
   NCSMIB_ARG arg;
   USRBUF *ub = NULL;

   /* Delete two rows */
   memset(&lcl_key, '\0', sizeof(lcl_key));
   lcl_key.count = 3;
   tblsix = (SNMPTM_TBLSIX*)ncs_patricia_tree_get(&cb->tblsix_tree,
                                                  (uns8*)&lcl_key);
   if(tblsix != NULL)
      snmptm_delete_tblsix_entry(cb, tblsix);

   memset(&lcl_key, '\0', sizeof(lcl_key));
   lcl_key.count = 4;
   tblsix = (SNMPTM_TBLSIX*)ncs_patricia_tree_get(&cb->tblsix_tree,
                                                  (uns8*)&lcl_key);
   if(tblsix != NULL)
      snmptm_delete_tblsix_entry(cb, tblsix);


   memset(&lcl_key, '\0', sizeof(lcl_key));
   memset(&arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&arg);
   ncsmem_aid_init(&ma, space, sizeof(space));
   arg.i_op = NCSMIB_OP_RSP_REMOVEROWS;
   arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLSIX;
   arg.i_mib_key = 0; /* Not required */
   arg.i_usr_key = 0; /* Not required */
   arg.i_rsp_fnc = NULL;
   arg.i_xch_id = xch_id++;
   arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
   arg.i_idx.i_inst_len = idx.i_inst_len = 1;

   memset(&rra, 0, sizeof(rra));
   ncsremrow_enc_init(&rra);
   inst_ids[0] = 3;   /* First index in the REMOVEROWS event */
   idx.i_inst_ids = inst_ids;
   ncsremrow_enc_inst_ids(&rra, &idx);
   {
      uns32 lcl_inst_ids[1];

      lcl_inst_ids[0] = 4;
      idx.i_inst_ids = lcl_inst_ids;
      ncsremrow_enc_inst_ids(&rra, &idx);
   }
   ub = ncsremrow_enc_done(&rra);
   arg.rsp.info.removerows_rsp.i_usrbuf = ub;

   memset(&mab_arg,0,sizeof(NCSOAC_SS_ARG));
   mab_arg.i_op = NCSOAC_SS_OP_PUSH_MIBARG_DATA_TO_PSSV;
   mab_arg.i_oac_hdl = cb->oac_hdl;
   mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLSIX;
   mab_arg.info.push_mibarg_data.arg = &arg;

   if (ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
   {
      if(arg.rsp.info.removerows_rsp.i_usrbuf != NULL)
            m_MMGR_FREE_BUFR_LIST(\
                            arg.rsp.info.removerows_rsp.i_usrbuf);
        return ;       
   }

   if(arg.rsp.info.removerows_rsp.i_usrbuf != NULL)
            m_MMGR_FREE_BUFR_LIST(\
                            arg.rsp.info.removerows_rsp.i_usrbuf);
      
   return;
}

/* Update row-modify information of TBLSIX to PSS */
static void snmptm_exec_test3(SNMPTM_CB *cb)
{
   SNMPTM_TBLSIX_KEY lcl_key;
   uns32   inst_ids[1];   /* For TBLSIX, only INDEX parameter */
   uns8   space[2048];
   NCSMEM_AID ma;
   uns32 xch_id = 1;
   NCSROW_AID ra;
   NCSPARM_AID pa;
   NCSMIB_IDX idx;
   NCSMIB_PARAM_VAL pv;
   NCSOAC_SS_ARG mab_arg;
   NCSMIB_ARG arg;

   /* Compose MIB SET event for ROW-2 */
   memset(&lcl_key, '\0', sizeof(lcl_key));
   memset(&arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&arg);
   ncsmem_aid_init(&ma, space, sizeof(space));
   arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLSIX;
   arg.i_mib_key = 0; /* Not required */
   arg.i_usr_key = 0; /* Not required */
   arg.i_rsp_fnc = NULL;
   arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
   lcl_key.count = 2;
   pv.i_fmat_id = NCSMIB_FMAT_INT;
   pv.i_param_id = ncsTestTableSixData_ID;
   pv.info.i_int = 103;
   arg.i_op = NCSMIB_OP_RSP_SET;
   arg.i_xch_id = xch_id++;
   inst_ids[0] = 2;
   arg.i_idx.i_inst_len = 1;
   arg.rsp.info.set_rsp.i_param_val = pv;

   memset(&mab_arg,0,sizeof(NCSOAC_SS_ARG));
   mab_arg.i_op = NCSOAC_SS_OP_PUSH_MIBARG_DATA_TO_PSSV;
   mab_arg.i_oac_hdl = cb->oac_hdl;
   mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLSIX;
   mab_arg.info.push_mibarg_data.arg = &arg;
   if (ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
   {
      return ;       
   }

   /* Compose MIB SETROW event for ROW-5 */
   memset(&lcl_key, '\0', sizeof(lcl_key));
   memset(&arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&arg);
   ncsmem_aid_init(&ma, space, sizeof(space));
   arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLSIX;
   arg.i_mib_key = 0; /* Not required */
   arg.i_usr_key = 0; /* Not required */
   arg.i_rsp_fnc = NULL;
   arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;

   lcl_key.count = 5;
   pv.i_fmat_id = NCSMIB_FMAT_INT;
   pv.i_param_id = ncsTestTableSixData_ID;
   pv.info.i_int = 106;
   arg.i_op = NCSMIB_OP_RSP_SETROW;
   arg.i_xch_id = xch_id++;
   arg.i_idx.i_inst_len = 1;
   inst_ids[0] = 5;
   ncsparm_enc_init(&pa);
   ncsparm_enc_param(&pa, &pv);
   arg.rsp.info.setrow_rsp.i_usrbuf = ncsparm_enc_done(&pa);
   m_BUFR_STUFF_OWNER(arg.rsp.info.setrow_rsp.i_usrbuf);

   memset(&mab_arg,0,sizeof(NCSOAC_SS_ARG));
   mab_arg.i_op = NCSOAC_SS_OP_PUSH_MIBARG_DATA_TO_PSSV;
   mab_arg.i_oac_hdl = cb->oac_hdl;
   mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLSIX;
   mab_arg.info.push_mibarg_data.arg = &arg;
   if (ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
   {
      return ;       
   }
   if(arg.rsp.info.setrow_rsp.i_usrbuf != NULL)
      m_MMGR_FREE_BUFR_LIST(\
                            arg.rsp.info.setrow_rsp.i_usrbuf);

   /* Compose MIB SETALLROWS event for ROW-6 */
   memset(&lcl_key, '\0', sizeof(lcl_key));
   memset(&arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&arg);
   ncsmem_aid_init(&ma, space, sizeof(space));
   arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLSIX;
   arg.i_mib_key = 0; /* Not required */
   arg.i_usr_key = 0; /* Not required */
   arg.i_rsp_fnc = NULL;
   arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;
   arg.i_idx.i_inst_len = idx.i_inst_len = 1;

   ncssetallrows_enc_init(&ra);
   ncsrow_enc_init(&ra);
   pv.i_fmat_id = NCSMIB_FMAT_INT;
   pv.i_param_id = ncsTestTableSixData_ID;
   pv.info.i_int = 107;
   inst_ids[0] = 6;
   idx.i_inst_ids = inst_ids;
   ncsrow_enc_inst_ids(&ra, &idx);
   ncsrow_enc_param(&ra, &pv);
   ncsrow_enc_done(&ra);
   arg.i_xch_id = xch_id;
   arg.i_op = NCSMIB_OP_RSP_SETALLROWS;
   arg.rsp.info.setallrows_rsp.i_usrbuf = ncssetallrows_enc_done(&ra);
   m_BUFR_STUFF_OWNER(arg.rsp.info.setallrows_rsp.i_usrbuf);

   memset(&mab_arg,0,sizeof(NCSOAC_SS_ARG));
   mab_arg.i_op = NCSOAC_SS_OP_PUSH_MIBARG_DATA_TO_PSSV;
   mab_arg.i_oac_hdl = cb->oac_hdl;
   mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLSIX;
   mab_arg.info.push_mibarg_data.arg = &arg;
   if (ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
   {
      return ;       
   }

   if(arg.rsp.info.setallrows_rsp.i_usrbuf != NULL)
      m_MMGR_FREE_BUFR_LIST(\
                       arg.rsp.info.setallrows_rsp.i_usrbuf);

   return;
}

/* Update all rows of TBLTEN to PSS */
static void snmptm_exec_test5(SNMPTM_CB *cb)
{
   SNMPTM_TBLTEN *tblten = NULL;
   SNMPTM_TBLTEN_KEY lcl_key;
   uns32   inst_ids[2];   /* For TBLTEN, two INDEX parameters */
   uns8   space[2048];
   NCSMEM_AID ma;
   uns32 xch_id = 1;
   NCSROW_AID ra;
   NCSPARM_AID pa;
   NCSMIB_IDX idx;
   NCSMIB_PARAM_VAL pv;
   NCSOAC_SS_ARG mab_arg;
   NCSMIB_ARG arg;
   uns32 read_cnt = 0;
   char name[9];
   uns32 evt_type = 1; /* Default, SETROW for TBLTEN*/

   memset(&lcl_key, '\0', sizeof(lcl_key));
   memset(&arg, 0, sizeof(NCSMIB_ARG));
   ncsmib_init(&arg);
   ncsmem_aid_init(&ma, space, sizeof(space));
   arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLTEN;
   arg.i_mib_key = 0; /* Not required */
   arg.i_usr_key = 0; /* Not required */
   arg.i_rsp_fnc = NULL;
   arg.i_idx.i_inst_ids = (const uns32 *)inst_ids;

   while((tblten = (SNMPTM_TBLTEN*)ncs_patricia_tree_getnext(&cb->tblten_tree, (uns8*)&lcl_key)) != NULL)
   {
      read_cnt ++;
      lcl_key.tblten_unsigned32 = tblten->tblten_key.tblten_unsigned32;
      lcl_key.tblten_int = tblten->tblten_key.tblten_int;
      memset(&name, '\0', sizeof(name));
      pv.i_fmat_id = NCSMIB_FMAT_INT;
      pv.i_param_id = ncsTestTableTenInt_ID;
      pv.info.i_int = tblten->tblten_key.tblten_int;

      if(evt_type == 1)
      {
         /* Compose MIB SET event */
         arg.i_op = NCSMIB_OP_RSP_SET;
         arg.i_xch_id = xch_id++;
         arg.i_idx.i_inst_len = 2;
         inst_ids[0] = lcl_key.tblten_unsigned32;
         inst_ids[1] = lcl_key.tblten_int;
         arg.rsp.info.set_rsp.i_param_val = pv;

         memset(&mab_arg,0,sizeof(NCSOAC_SS_ARG));
         mab_arg.i_op = NCSOAC_SS_OP_PUSH_MIBARG_DATA_TO_PSSV;
         mab_arg.i_oac_hdl = cb->oac_hdl;
         mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLTEN;
         mab_arg.info.push_mibarg_data.arg = &arg;
         if (ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
         {
            return ;
         }
      }
      else if(evt_type == 2)
      {
         /* Compose MIB SETROW event */
         arg.i_op = NCSMIB_OP_RSP_SETROW;
         arg.i_xch_id = xch_id++;
         arg.i_idx.i_inst_len = 2;
         inst_ids[0] = lcl_key.tblten_unsigned32;
         inst_ids[1] = lcl_key.tblten_int;

         ncsparm_enc_init(&pa);
         ncsparm_enc_param(&pa, &pv);

         /* Compose USRBUF */
         arg.rsp.info.setrow_rsp.i_usrbuf = ncsparm_enc_done(&pa);
         m_BUFR_STUFF_OWNER(arg.req.info.setrow_rsp.i_usrbuf);

         memset(&mab_arg,0,sizeof(NCSOAC_SS_ARG));
         mab_arg.i_op = NCSOAC_SS_OP_PUSH_MIBARG_DATA_TO_PSSV;
         mab_arg.i_oac_hdl = cb->oac_hdl;
         mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLTEN;
         mab_arg.info.push_mibarg_data.arg = &arg;
         if (ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
         {
            return ;       
         }
         if(arg.rsp.info.setrow_rsp.i_usrbuf != NULL)
            m_MMGR_FREE_BUFR_LIST(\
                            arg.rsp.info.setrow_rsp.i_usrbuf);
      }
      else
      {
         /* Default - Compose MIB SETALLROWS event */
         ncssetallrows_enc_init(&ra);
         ncsrow_enc_init(&ra);

         arg.i_idx.i_inst_len = 2;
         inst_ids[0] = lcl_key.tblten_unsigned32;
         inst_ids[1] = lcl_key.tblten_int;
         idx.i_inst_ids = inst_ids;
         ncsrow_enc_inst_ids(&ra, &idx);
         ncsrow_enc_param(&ra, &pv);
         ncsrow_enc_done(&ra);
      }
   }  /* while loop completes here */

   if((read_cnt != 0) && ((evt_type != 1) && (evt_type != 2)))
   {
      /* Send the already composed SETALLROWS event */
      arg.i_xch_id = xch_id;
      arg.i_idx.i_inst_len = 2;
      arg.i_op = NCSMIB_OP_RSP_SETALLROWS;
      arg.rsp.info.setallrows_rsp.i_usrbuf = ncssetallrows_enc_done(&ra);
      m_BUFR_STUFF_OWNER(arg.rsp.info.setallrows_rsp.i_usrbuf);
      arg.i_idx.i_inst_len = idx.i_inst_len;
      arg.i_idx.i_inst_ids = idx.i_inst_ids;

      memset(&mab_arg,0,sizeof(NCSOAC_SS_ARG));
      mab_arg.i_op = NCSOAC_SS_OP_PUSH_MIBARG_DATA_TO_PSSV;
      mab_arg.i_oac_hdl = cb->oac_hdl;
      mab_arg.i_tbl_id = NCSMIB_TBL_SNMPTM_TBLTEN;
      mab_arg.info.push_mibarg_data.arg = &arg;
      if (ncsoac_ss(&mab_arg) != NCSCC_RC_SUCCESS)
      {
         return ;       
      }

      if(arg.rsp.info.setallrows_rsp.i_usrbuf != NULL)
         m_MMGR_FREE_BUFR_LIST(\
                       arg.rsp.info.setallrows_rsp.i_usrbuf);
   }

   return;
}

