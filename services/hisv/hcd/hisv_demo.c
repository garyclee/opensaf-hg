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
*                                                                            *
*  MODULE NAME:  hisv_demo.c                                                 *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This is a demo program to illustrate the usage and capabilites of HISv    *
*  The code in this file is commented out because the demo files are to be   *
*  moved to toolkit vob. However functionality of this demo is incomplete.   *
*  This demo is not required for end-user. It is for the developer and this  *
*  needs to be completed for some of the test cases related to hotswap.      *
*                                                                            *
*****************************************************************************/

#include "hcd.h"
#include "hpl.h"
#include "hisv_demo.h"
#include "ncs_main_papi.h"

/* local function declarations */
static uns32 hcd_control(int argc, char *argv[]);
static uns32 hpl_control(void);
static uns32 init_hcd_flag = 0;
static uns32 init_hpl_flag = 0;


/****************************************************************************
 * Name          : 'Main' function
 *
 * Description   : Menu based 'main' program used to run the executable to
 *                 start the HCD threads and check the usage of HPL APIs
 *
 * Arguments     : argc - Number of Arguments
 *                 argv - Arguments, "CLUSTER_ID=19875", "NODE_ID=1",
 *                                   "HUB=y", "PCON_ID=1"
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 hisv_main(int argc, char **argv)
{
   /** format of argv
    ** char *argv[]={"", "CLUSTER_ID=19875", "NODE_ID=1", "HUB=y", "PCON_ID=1"};
    **/

   /* init the services */
   /* ncs_svcs_startup(argc, argv); */

   for (;;)
   {
      uns32 command;
      /* display the menu */
      m_NCS_CONS_PRINTF("\nMENU: Please input number for command\n");
      m_NCS_CONS_PRINTF("     0 : Display MENU again\n");
      m_NCS_CONS_PRINTF("     1 : Start or Stop HCD\n");
      m_NCS_CONS_PRINTF("     2 : Initialize and Use HPL\n");
      m_NCS_CONS_PRINTF("     3 : Exit\n");
      scanf("%d", &command);
      switch(command)
      {
         case HISV_DISPLAY_MENU:
            continue;
            break;

         case HISV_HCD_CONTROL:
            /* program to act as HCD controller */
            hcd_control(argc, argv);
            break;

         case HISV_HPL_CONTROL:
            /* program to initialize and use HPL library */
            hpl_control();
            break;

         case HISV_DEMO_EXIT:
            /* done with demo, exit */
            m_NCS_CONS_PRINTF("Exit demo\n");
            goto demo_ret;
            break;

         default:
            m_NCS_CONS_PRINTF("Invalid option\n");
            break;
      }
   }

demo_ret:
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : hcd_control
 *
 * Description   : Menu based HCD control function. Used to start and stop
 *                 HCD on a given chassis
 *
 * Arguments     : chassis_id - chassis identifier.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 hcd_control(int argc, char *argv[])
{
   NCS_LIB_REQ_INFO req_info;
   uns32 chassis_id;
   char *argv_def[]={"6", "NULL"};

   /* Initialize the input argument for chassis_id */
   req_info.i_op = NCS_LIB_REQ_CREATE;
   req_info.info.create.argc = 1;

   req_info.info.create.argv = argv;
   if (argc == 0)
      req_info.info.create.argv = argv_def;

   for (;;)
   {
      uns32 command;
      /* Display the Menu */
      m_NCS_CONS_PRINTF("\nHCD MENU: Please input number for command\n");
      m_NCS_CONS_PRINTF("     0 : Display MENU again\n");
      m_NCS_CONS_PRINTF("     1 : Initialize and start HCD\n");
      m_NCS_CONS_PRINTF("     2 : Finalize and stop HCD\n");
      m_NCS_CONS_PRINTF("     3 : Finalize, stop HCD & return\n");
      m_NCS_CONS_PRINTF("     4 : Just return to Main Menu\n");
      scanf("%d", &command);
      switch(command)
      {
      case HCD_DISPLAY_MENU:
         /* Re-Display Menu */
         break;

      case HCD_INITIALIZE:
         /* check if already initialized */
         if (init_hcd_flag)
         {
            m_NCS_CONS_PRINTF("HCD already running\n");
            break;
         }
         m_NCS_CONS_PRINTF("Enter Chassis Identifier\n");
         scanf("%d",&chassis_id);

         /* initialize HAM and HSM on this chassis */
         if (NCSCC_RC_FAILURE == ncs_hisv_hcd_lib_req(&req_info))
         {
            m_NCS_CONS_PRINTF("Failed to Initialize HCD\n");
            break;
         }
         init_hcd_flag = 1;
         m_NCS_CONS_PRINTF("HCD Initialization done\n");
         break;

      case HCD_FINALIZE:
         /* check if HCD not yet initialized */
         if (0 == init_hcd_flag)
         {
            m_NCS_CONS_PRINTF("HCD Not initiailized\n");
            break;
         }
         /* finalize HAM and HSM */
         hsm_finalize();
         ham_finalize();
         init_hcd_flag = 0;
         m_NCS_CONS_PRINTF("HCD finalization done\n");
         break;

      case HCD_RET_MAIN_MENU:
         /* just return to main menu without terminating HCD */
         goto ret;
         break;

      case HCD_FINALIZE_RET:
         /* finalize and return to Main menu */
         if (0 == init_hcd_flag)
            m_NCS_CONS_PRINTF("HCD was Not initiailized\n");
         else
         {
            /* finalize HSM */
            hsm_finalize();
            /* finalize HAM */
            ham_finalize();
            init_hcd_flag = 0;
         }
         goto ret;
         break;

      default:
         /* unknow command option */
         m_NCS_CONS_PRINTF("Invalid option\n");
         break;
      }
   }
ret:
   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : hpl_control
 *
 * Description   : Menu based HPL control function. Used to initialize, use
 *                 and finalize the HPL library. It also creates an EDS server
 *                 so that HSM can publish events on to it. Also the services
 *                 can subscribe to it to receive the events published by HSM.
 *
 * Arguments     : None.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

static uns32 hpl_control()
{
   NCS_LIB_REQ_INFO req_info;
   uns32 rc, chassis_id, arg;
   uns8 entity_path[MAX_ENTITY_PATH_LEN];
   HISV_API_CMD cmd;

   /** Initialize the EDSV server with this HPL program
    ** so that the EDA agent in HSM can publish event
    ** on to it. And users of HPL can also subscribe
    ** to EDS to receive those events
    **/
#if 0
   if (init_hpl_flag == 0)
   {
      m_NCS_CONS_PRINTF("Starting EDS...\n");

      m_NCS_MEMSET(&req_info, '\0', sizeof(req_info));
      req_info.i_op = NCS_LIB_REQ_CREATE;
      rc = ncs_edsv_eds_lib_req(&req_info);
      if (rc != NCSCC_RC_SUCCESS)
      {
         m_NCS_CONS_PRINTF("ncs_edsv_eds_lib_req() failed. rc=%d\n", rc);
         return(NCSCC_RC_FAILURE);
      }
      m_NCS_CONS_PRINTF("\nEDS Running...\n");
   }
#endif /* 0 */
   for (;;)
   {
      uns32 command;
      /* Display the Menu */
      m_NCS_CONS_PRINTF("\nHPL MENU: Please input number for command\n");
      m_NCS_CONS_PRINTF("     0 : Display MENU again\n");
      m_NCS_CONS_PRINTF("     1 : Initialize HPL Library\n");
      m_NCS_CONS_PRINTF("     2 : Reset the Resource\n");
      m_NCS_CONS_PRINTF("     3 : Change the Power State of Resource\n");
      m_NCS_CONS_PRINTF("     4 : Clear the System Event Log\n");
      m_NCS_CONS_PRINTF("     5 : Finalize HPL Library\n");
      m_NCS_CONS_PRINTF("     6 : Finalize, HPL & return\n");
      m_NCS_CONS_PRINTF("     7 : Just return to Main Menu\n");
      scanf("%d", &command);
      switch(command)
      {
      case HPL_DISPLAY_MENU:
         /* Re-Display Menu */
         break;

      case HPL_INITIALIZE:
         /* check if already initialized */
         if (init_hpl_flag)
         {
            m_NCS_CONS_PRINTF("HPL already initialized\n");
            break;
         }
         m_NCS_MEMSET(&req_info, '\0', sizeof(req_info));
         req_info.i_op = NCS_LIB_REQ_CREATE;

         /* request to initialize HPL library */
         rc = ncs_hpl_lib_req(&req_info);
         if (rc != NCSCC_RC_SUCCESS)
         {
            m_NCS_CONS_PRINTF("ncs_hpl_lib_req() failed. rc=%d\n", rc);
            break;
         }
         m_NCS_CONS_PRINTF("HPL initialization done\n");
         init_hpl_flag = 1;
         break;

      case HPL_RESOURCE_RESET:
         /* check if HPL not yet initialized */
         if (0 == init_hpl_flag)
         {
            m_NCS_CONS_PRINTF("HPL Not initiailized\n");
            break;
         }
         m_NCS_CONS_PRINTF("Enter Chassis Identifier\n");
         scanf("%d",&chassis_id);

         /* read the resource entity path */
         m_NCS_CONS_PRINTF("Enter the Entity path of the resource\n");
         scanf("%s",&entity_path[0]);
         m_NCS_CONS_PRINTF("Entity path = %s\n", entity_path);

         /* read the reset type */
         m_NCS_CONS_PRINTF("Enter the Reset Type\n");
         m_NCS_CONS_PRINTF("Cold=0, Warm=1, Asserive=2, De-Assertive=3\n");
         scanf("%d",&arg);

         /* invoke HPL API to reset the resource */
         rc = hpl_resource_reset(chassis_id, entity_path, arg);
         m_NCS_CONS_PRINTF("HPL API hpl_resource_reset() invoked, rc = %d\n", rc);
         break;

      case HPL_RESOURCE_POWER_SET:
         /* check if HPL not yet initialized */
         if (0 == init_hpl_flag)
         {
            m_NCS_CONS_PRINTF("HPL Not initiailized\n");
            break;
         }
         m_NCS_CONS_PRINTF("Enter Chassis Identifier\n");
         scanf("%d",&chassis_id);

         /* read the resource entity path */
         m_NCS_CONS_PRINTF("Enter the Entity path of the resource\n");
         scanf("%s",&entity_path[0]);

         /* read the power state for this resource */
         m_NCS_CONS_PRINTF("Enter the Power State\n");
         m_NCS_CONS_PRINTF("Power-Off=0, Power-On=1, Power-Cycle=2\n");
         scanf("%d",&arg);

         /* invoke HPL API to change the power state of resource */
         rc = hpl_resource_power_set(chassis_id, entity_path, arg);
         m_NCS_CONS_PRINTF("HPL API hpl_resource_power_set() invoked rc = %d\n", rc);
         break;

      case HPL_SEL_CLEAR:
         /* check if HPL not yet initialized */
         if (0 == init_hpl_flag)
         {
            m_NCS_CONS_PRINTF("HPL Not initiailized\n");
            break;
         }
         m_NCS_CONS_PRINTF("Enter Chassis Identifier\n");
         scanf("%d",&chassis_id);

         /* invoke HPL API to clear the system event log of HPI */
         rc = hpl_sel_clear(chassis_id);
         m_NCS_CONS_PRINTF("HPL API hpl_sel_clear() invoked, rc = %d\n", rc);
         break;

      case HPL_CONFIG_HOTSWAP:
         /* check if HPL not yet initialized */
         if (0 == init_hpl_flag)
         {
            m_NCS_CONS_PRINTF("HPL Not initiailized\n");
            break;
         }
         m_NCS_CONS_PRINTF("Enter Chassis Identifier\n");
         scanf("%d",&chassis_id);

         /* read the hotswap config command */
         m_NCS_CONS_PRINTF("Enter\n0 - HS_AUTO_INSERT_TIMEOUT_SET\n1 - HS_AUTO_INSERT_TIMEOUT_GET\n");
         if (cmd == 0) cmd = HS_AUTO_INSERT_TIMEOUT_SET;
         else cmd = HS_AUTO_INSERT_TIMEOUT_GET;

         /* read the auto insert time in case of set */
         if (cmd == HS_AUTO_INSERT_TIMEOUT_SET)
         {
            m_NCS_CONS_PRINTF("Enter the auto insert time to set\n");
            scanf("%d",&arg);
         }

         /* invoke hpl_config_hotswap cmd */
         rc = hpl_config_hotswap(chassis_id, cmd, (uns64 *)&arg);
         m_NCS_CONS_PRINTF("HPL API hpl_config_hotswap() invoked, rc = %d, arg = %d\n", rc, arg);
         break;

      case HPL_CONFIG_HS_STATE_GET:
         /* check if HPL not yet initialized */
         if (0 == init_hpl_flag)
         {
            m_NCS_CONS_PRINTF("HPL Not initiailized\n");
            break;
         }
         m_NCS_CONS_PRINTF("Enter Chassis Identifier\n");
         scanf("%d",&chassis_id);

         /* read the resource entity path */
         m_NCS_CONS_PRINTF("Enter the Entity path of the resource\n");
         scanf("%s",&entity_path[0]);
         m_NCS_CONS_PRINTF("Entity path = %s\n", entity_path);

         /* read the reset type */
         m_NCS_CONS_PRINTF("Enter the Reset Type\n");
         m_NCS_CONS_PRINTF("Cold=0, Warm=1, Asserive=2, De-Assertive=3\n");
         scanf("%d",&arg);

         /* invoke HPL API to reset the resource */
         rc = hpl_config_hs_state_get(chassis_id, entity_path, &arg);
         m_NCS_CONS_PRINTF("HPL API hpl_resource_reset() invoked, rc = %d\n", rc);
         break;

      case HPL_CONFIG_HS_INDICATOR:
         /* check if HPL not yet initialized */
         if (0 == init_hpl_flag)
         {
            m_NCS_CONS_PRINTF("HPL Not initiailized\n");
            break;
         }
         m_NCS_CONS_PRINTF("Enter Chassis Identifier\n");
         scanf("%d",&chassis_id);

         /* read the resource entity path */
         m_NCS_CONS_PRINTF("Enter the Entity path of the resource\n");
         scanf("%s",&entity_path[0]);
         m_NCS_CONS_PRINTF("Entity path = %s\n", entity_path);

         /* read the reset type */
         m_NCS_CONS_PRINTF("Enter the Reset Type\n");
         m_NCS_CONS_PRINTF("Cold=0, Warm=1, Asserive=2, De-Assertive=3\n");
         scanf("%d",&arg);

         /* invoke HPL API to reset the resource */
         hpl_config_hs_indicator(chassis_id, entity_path, cmd, &arg);
         m_NCS_CONS_PRINTF("HPL API hpl_config_hs_indicator() invoked\n");
         break;

      case HPL_CONFIG_HS_AUTOEXTRACT:
         /* check if HPL not yet initialized */
         if (0 == init_hpl_flag)
         {
            m_NCS_CONS_PRINTF("HPL Not initiailized\n");
            break;
         }
         m_NCS_CONS_PRINTF("Enter Chassis Identifier\n");
         scanf("%d",&chassis_id);

         /* read the resource entity path */
         m_NCS_CONS_PRINTF("Enter the Entity path of the resource\n");
         scanf("%s",&entity_path[0]);
         m_NCS_CONS_PRINTF("Entity path = %s\n", entity_path);

         /* read the reset type */
         m_NCS_CONS_PRINTF("Enter the Reset Type\n");
         m_NCS_CONS_PRINTF("Cold=0, Warm=1, Asserive=2, De-Assertive=3\n");
         scanf("%d",&arg);

         /* invoke HPL API to reset the resource */
         hpl_config_hs_autoextract(chassis_id, entity_path, cmd, (uns64 *)&arg);
         m_NCS_CONS_PRINTF("HPL API hpl_config_hs_autoextract() invoked\n");
         break;

      case HPL_MANAGE_HOTSWAP:
         /* check if HPL not yet initialized */
         if (0 == init_hpl_flag)
         {
            m_NCS_CONS_PRINTF("HPL Not initiailized\n");
            break;
         }
         m_NCS_CONS_PRINTF("Enter Chassis Identifier\n");
         scanf("%d",&chassis_id);

         /* read the resource entity path */
         m_NCS_CONS_PRINTF("Enter the Entity path of the resource\n");
         scanf("%s",&entity_path[0]);
         m_NCS_CONS_PRINTF("Entity path = %s\n", entity_path);

         /* read the reset type */
         m_NCS_CONS_PRINTF("Enter the Reset Type\n");
         m_NCS_CONS_PRINTF("Cold=0, Warm=1, Asserive=2, De-Assertive=3\n");
         scanf("%d",&arg);

         /* read the manage hotswap command */
         m_NCS_CONS_PRINTF("Enter\n0 - HS_POLICY_CANCEL\n1 - HS_RESOURCE_ACTIVE_SET\n");
         m_NCS_CONS_PRINTF("\n2 - HS_RESOURCE_INACTIVE_SET\n3 - HS_ACTION_REQUEST\n");

         /* invoke HPL API to reset the resource */
         hpl_manage_hotswap(chassis_id, entity_path, cmd, arg);
         m_NCS_CONS_PRINTF("HPL API hpl_manage_hotswap() invoked\n");
         break;

      case HPL_ALARM_ADD:
         /* check if HPL not yet initialized */
         if (0 == init_hpl_flag)
         {
            m_NCS_CONS_PRINTF("HPL Not initiailized\n");
            break;
         }
         m_NCS_CONS_PRINTF("Enter Chassis Identifier\n");
         scanf("%d",&chassis_id);

         /* read the resource entity path */
         m_NCS_CONS_PRINTF("Enter the Entity path of the resource\n");
         scanf("%s",&entity_path[0]);
         m_NCS_CONS_PRINTF("Entity path = %s\n", entity_path);

         /* read the reset type */
         m_NCS_CONS_PRINTF("Enter the Reset Type\n");
         m_NCS_CONS_PRINTF("Cold=0, Warm=1, Asserive=2, De-Assertive=3\n");
         scanf("%d",&arg);

         /* invoke HPL API to reset the resource */
         /*
         hpl_alarm_add(uns32 chassis_id, HISV_API_CMD alarm_cmd,
                         uns16 arg_len, uns8* arg);*/
         m_NCS_CONS_PRINTF("HPL API hpl_alarm_add() invoked\n");
         break;

      case HPL_ALARM_GET:
         /* check if HPL not yet initialized */
         if (0 == init_hpl_flag)
         {
            m_NCS_CONS_PRINTF("HPL Not initiailized\n");
            break;
         }
         m_NCS_CONS_PRINTF("Enter Chassis Identifier\n");
         scanf("%d",&chassis_id);

         /* read the resource entity path */
         m_NCS_CONS_PRINTF("Enter the Entity path of the resource\n");
         scanf("%s",&entity_path[0]);
         m_NCS_CONS_PRINTF("Entity path = %s\n", entity_path);

         /* read the reset type */
         m_NCS_CONS_PRINTF("Enter the Reset Type\n");
         m_NCS_CONS_PRINTF("Cold=0, Warm=1, Asserive=2, De-Assertive=3\n");
         scanf("%d",&arg);

         /* invoke HPL API to reset the resource */
         /* hpl_alarm_get(uns32 chassis_id, HISV_API_CMD alarm_cmd, uns32 alarm_id,
                         uns16 arg_len, uns8* arg); */
         m_NCS_CONS_PRINTF("HPL API hpl_alarm_get() invoked\n");
         break;

      case HPL_ALARM_DELETE:
         /* check if HPL not yet initialized */
         if (0 == init_hpl_flag)
         {
            m_NCS_CONS_PRINTF("HPL Not initiailized\n");
            break;
         }
         m_NCS_CONS_PRINTF("Enter Chassis Identifier\n");
         scanf("%d",&chassis_id);

         /* read the resource entity path */
         m_NCS_CONS_PRINTF("Enter the Entity path of the resource\n");
         scanf("%s",&entity_path[0]);
         m_NCS_CONS_PRINTF("Entity path = %s\n", entity_path);

         /* read the reset type */
         m_NCS_CONS_PRINTF("Enter the Reset Type\n");
         m_NCS_CONS_PRINTF("Cold=0, Warm=1, Asserive=2, De-Assertive=3\n");
         scanf("%d",&arg);

         /* invoke HPL API to reset the resource */
         /* hpl_alarm_delete(uns32 chassis_id, HISV_API_CMD alarm_cmd, uns32 alarm_id,
                         uns32 alarm_severity); */
         m_NCS_CONS_PRINTF("HPL API hpl_alarm_delete() invoked\n");
         break;

      case HPL_EVENT_LOT_TIME:
         /* check if HPL not yet initialized */
         if (0 == init_hpl_flag)
         {
            m_NCS_CONS_PRINTF("HPL Not initiailized\n");
            break;
         }
         m_NCS_CONS_PRINTF("Enter Chassis Identifier\n");
         scanf("%d",&chassis_id);

         /* read the resource entity path */
         m_NCS_CONS_PRINTF("Enter the Entity path of the resource\n");
         scanf("%s",&entity_path[0]);
         m_NCS_CONS_PRINTF("Entity path = %s\n", entity_path);

         /* read the reset type */
         m_NCS_CONS_PRINTF("Enter the Reset Type\n");
         m_NCS_CONS_PRINTF("Cold=0, Warm=1, Asserive=2, De-Assertive=3\n");
         scanf("%d",&arg);

         /* invoke HPL API to reset the resource */
         /* hpl_event_log_time(uns32 chassis_id, uns8 *entity_path,
                         HISV_API_CMD evlog_time_cmd, uns64 *arg); */
         m_NCS_CONS_PRINTF("HPL API hpl_event_log_time() invoked\n");
         break;

      case HPL_FINALIZE:
         /* check if HPL not yet initialized */
         if (0 == init_hpl_flag)
         {
            m_NCS_CONS_PRINTF("HPL Not initiailized\n");
            break;
         }
         m_NCS_MEMSET(&req_info, '\0', sizeof(req_info));
         req_info.i_op = NCS_LIB_REQ_DESTROY;

         /* request to finalize HPL library */
         rc = ncs_hpl_lib_req(&req_info);
         if (rc != NCSCC_RC_SUCCESS)
         {
            m_NCS_CONS_PRINTF("ncs_hpl_lib_req() failed. rc=%d\n", rc);
            break;
         }
         m_NCS_CONS_PRINTF("HPL finalization done\n");
         init_hpl_flag = 0;
         break;

      case HPL_FINALIZE_RET:
         /* finalize and return to Main menu */
         if (0 == init_hpl_flag)
            m_NCS_CONS_PRINTF("HPL was Not initiailized\n");
         else
         {
            m_NCS_MEMSET(&req_info, '\0', sizeof(req_info));
            req_info.i_op = NCS_LIB_REQ_DESTROY;

            /* request to finalize HPL library */
            rc = ncs_hpl_lib_req(&req_info);
            if (rc != NCSCC_RC_SUCCESS)
            {
               m_NCS_CONS_PRINTF("ncs_hpl_lib_req() failed. rc=%d\n", rc);
               goto ret;
            }
            init_hpl_flag = 0;
         }
         goto ret;
         break;

      case HPL_RET_MAIN_MENU:
         /* just return to main menu without finalizing HPL */
         return NCSCC_RC_SUCCESS;
         break;

      default:
         /* unknow command option */
         m_NCS_CONS_PRINTF("Invalid option\n");
         break;
      }
   }

ret:
#if 0
   /* Done with HPL usage, Now destroy EDS server */
   m_NCS_MEMSET(&req_info, '\0', sizeof(req_info));
   req_info.i_op = NCS_LIB_REQ_DESTROY;
   rc = ncs_edsv_eds_lib_req(&req_info);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("ncs_edsv_eds_lib_req() failed. rc=%d\n", rc);
      return(NCSCC_RC_FAILURE);
   }
   m_NCS_CONS_PRINTF("\nEDS Stopped...\n");
#endif /* 0 */
   return NCSCC_RC_SUCCESS;
}

