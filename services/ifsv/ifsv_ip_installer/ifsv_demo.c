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


#include "ifsv_demo.h"
static int print_intf_menu();
NCSCONTEXT task_hdl;

int ncs_ifsv_red_demo_flag=0;


/****************************************************************************
 * Name          : main
 *
 * Description   : This is the main function which gonna call the demo start
 *                 up function.
 *
 * Arguments     : argc - Number of arguments passed.
 *                 argv - argument strings.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
int main (int argc, char *argv[])
{
   uns32 temp_var;
   SHELF_SLOT shlef_slot;
   uns32 tmp_ctr;
   char *pargv[IFSV_DEMO_MAIN_MAX_INPUT];
   
 
   if (argc != 2)
   {
      m_NCS_CONS_PRINTF ("\nWrong Arguments USAGE: <ifsv_demo.out> <1 - app demo create (or) 2 - driver demo create \n");
      return (NCSCC_RC_FAILURE);
   }

     for (tmp_ctr=0; tmp_ctr<IFSV_DEMO_MAIN_MAX_INPUT; tmp_ctr++)
      pargv[tmp_ctr] = (char*)malloc(64);

   temp_var = atoi(argv[1]);
   switch(temp_var)
   {
      case 1:
         /** initialize the leap and agent stuffs **/
         sprintf(pargv[0],"NONE");
         strcpy(pargv[1],"IFSV=y");
         strcpy(pargv[2],"DTSV=y");

         ncs_agents_startup(3, pargv);
         shlef_slot.shelf = 0;
         shlef_slot.slot  = 1;
         /* embedding subslot changes */
         shlef_slot.subslot  = 1;
         if (ifsv_app_demo_start((NCSCONTEXT)&shlef_slot) != NCSCC_RC_SUCCESS)
         {
            m_NCS_CONS_PRINTF ("ifsv_app_demo_start create failed \n");
            return (NCSCC_RC_FAILURE);
         }
         break;
      case 2:
         /** initialize the leap and agent stuffs **/
         sprintf(pargv[0],"NONE");
         strcpy(pargv[1],"IFSV=y");
         strcpy(pargv[2],"DTSV=y");

         ncs_agents_startup(3, pargv);
         if (ifsv_drv_demo_start(NULL) != NCSCC_RC_SUCCESS)
         {
            m_NCS_CONS_PRINTF ("ifsv_drv_demo_start create failed \n");
            return (NCSCC_RC_FAILURE);
         }
         break;

      case 3:
         /** initialize the leap and agent stuffs **/
         sprintf(pargv[0],"NONE");
         strcpy(pargv[1],"IFSV=y");
         strcpy(pargv[2],"DTSV=y");

         ncs_agents_startup(3, pargv);
         shlef_slot.shelf = 0;
         shlef_slot.slot  = 1;
          /* embedding subslot changes */
         shlef_slot.subslot  = 1;
         ncs_ifsv_red_demo_flag = 1;
         if (ifsv_app_demo_start((NCSCONTEXT)&shlef_slot) != NCSCC_RC_SUCCESS)
         {
            m_NCS_CONS_PRINTF ("ifsv_app_demo_start create failed \n");
            return (NCSCC_RC_FAILURE);
         }
        break;
         
      default :
         break;
   }


  for (tmp_ctr=0;tmp_ctr<IFSV_DEMO_MAIN_MAX_INPUT;tmp_ctr++)
       free(pargv[tmp_ctr]);

   /** this is to verify your demo outputs **/
   while(1) {
      sleep(100);
   }
   getchar();
   return (NCSCC_RC_SUCCESS);
}


static int print_intf_menu()
{
       printf("************ Interface Redundancy Demo **********\n");
       printf("1. Add two interfaces\n");
       printf("2. Create a bind interface\n");
       printf("3. Swap Interfaces\n");
       printf("4. Delete Interface 1\n");
       printf("5. Delete Interface 2\n");
       printf("6. Get Local Interface for bind interface\n");
       printf("7. exit\n");
       printf("Enter choice\n");
       return NCSCC_RC_SUCCESS;

}



/****************************************************************************
 * Name          : ifsv_app_demo_start
 *
 * Description   : This is the wrapper function used to start the demo 
 *                 application, add/modify the interface and 
 *                 subscribe/unsubscribe with IfA.
 *
 * Arguments     : info - This is info which demo requires.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
/*** This is the function which is going to create a application demo ***/
uns32 
ifsv_app_demo_start(NCSCONTEXT *info)
{
   uns32 app_num = 0;
   char ch,if_name[34] = "ifa_demo";
   SHELF_SLOT *sh_slot = (SHELF_SLOT *)info;
   uns32 shelf = 0;
   uns32 slot = 0;
   uns32 subslot = 0;
   uns32 port_num   = 110;
   uns32 port_type  = 4;
   uns32 MTU        = 1200;
   uns32 speed      = 100;
   uns32 oper_state = 1;
   uns8  i_phy[48] = "1:22:22:22:22:22";

   shelf = sh_slot->shelf; 
   slot  = sh_slot->slot;
   subslot = sh_slot->subslot;  
   /** create an MOCK application for demo **/
   if (IfsvDtTestAppCreate(shelf, slot, subslot) != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("Sorry couldn't able to create demo's\n");
      return(NCSCC_RC_FAILURE);
   }
   /** sleep for some time to create demo application and be stable so that 
       after that we can register the application with IfA **/
   if(!ncs_ifsv_red_demo_flag) 
        m_NCS_TASK_SLEEP(6000);

   if(ncs_ifsv_red_demo_flag) 
   {
   /** application subscribe with IfA **/
   if (IfsvDtTestAppIfaSub(0, 7, 512) != NCSCC_RC_SUCCESS)
   {
   m_NCS_CONS_PRINTF("Sorry couldn't able to subscribe with IfA \n");
   return(NCSCC_RC_FAILURE);
   }
   }
   else 
   {   

   /** application subscribe with IfA **/
   if (IfsvDtTestAppIfaSub(0, 7, 255) != NCSCC_RC_SUCCESS)
   {
   m_NCS_CONS_PRINTF("Sorry couldn't able to subscribe with IfA \n");
   return(NCSCC_RC_FAILURE);
  }

  }

   if(!ncs_ifsv_red_demo_flag)
   {
   if (IfsvDtTestAppAddIntf(app_num, if_name, port_num, port_type, i_phy, oper_state, MTU, speed) !=
       NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("Sorry couldn't add an interface from application\n");
      return(NCSCC_RC_FAILURE);
   }
   }

   if(ncs_ifsv_red_demo_flag)
   {

   while (1)
      {
            print_intf_menu();
            ch = getchar(); 
            switch(ch)
               {
                    case '1' :
                       printf("Adding Interface 1\n");
   
                        /** add an interface from the application **/
                       if (IfsvDtTestAppAddIntf(app_num, if_name, port_num, port_type, i_phy, oper_state, MTU, speed) !=
                              NCSCC_RC_SUCCESS)
                        {
                            m_NCS_CONS_PRINTF("Sorry couldn't add an interface 1 from application\n");
                            return(NCSCC_RC_FAILURE);
                        }
                      printf("Adding Interface 2\n");
                        strcpy(i_phy,"1:22:23:24:26:27");       
                       if (IfsvDtTestAppAddIntf(app_num, if_name, 67, port_type, i_phy, oper_state, MTU, speed) !=
                              NCSCC_RC_SUCCESS)
                       {
                           m_NCS_CONS_PRINTF("Sorry couldn't add an interface 2 from application\n");
                           return(NCSCC_RC_FAILURE);
                       }
                       printf("Added two interfaces\n");
                       printf("The ifindices are assumed to be 1 and 2\n"); 
                       break;

                    case '2':
                             IfsvDtTestAppSwapBondIntf(app_num, 1, NCS_IFSV_INTF_BINDING, 1, 2);
                             printf("Check the show cli command \n");
                             break;
   
                    case '3':
                             IfsvDtTestAppSwapBondIntf(app_num, 1, NCS_IFSV_INTF_BINDING, 2, 1);
                             printf("Check the show cli command \n");
                             break;

                    case '4' :
                          /* disable interface 1 */
                             IfsvDtTestAppDelIntf(app_num,110,4);
                             break;
                    case '5' :
                             IfsvDtTestAppDelIntf(app_num,67,4);
                             break;
                    case '6' :
                             IfsvDtTestAppGetBondLocalIfinfo(app_num, IFSV_BINDING_SHELF_ID, IFSV_BINDING_SLOT_ID, IFSV_BINDING_SUBSLOT_ID ,1 ,NCS_IFSV_INTF_BINDING);
                             break;

                    case '7':
                            goto cleanup;
               }
      }
   }
   
   /** wait for 16 seconds **/
   m_NCS_TASK_SLEEP(16000);
   /*** disable the interface ***/
   oper_state = 2;
   if (IfsvDtTestAppModIntfStatus(app_num, port_num, port_type, oper_state) != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("Sorry couldn't modify an interface from application\n");
      return(NCSCC_RC_FAILURE);
   }

   /*** get statistics for the available interface ***/
   port_num   = 2;
   port_type  = 26;
   if (IfsvDtTestAppGetStats(app_num, shelf, slot, subslot, port_num, port_type) != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("Sorry couldn't get interface statistics from application\n");
      return(NCSCC_RC_FAILURE);
   }
   /** get interface infomation for the give shlef/slot/port/type **/
   if (IfsvDtTestAppGetIfinfo(app_num, shelf, slot, subslot, port_num, port_type) != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("Sorry couldn't get interface info from application\n");
      return(NCSCC_RC_FAILURE);
   }
   /** wait for 30 seconds **/
   m_NCS_TASK_SLEEP(30000);

cleanup:   
   /** unsubscribe with IfA **/
   if (IfsvDtTestAppIfaUnsub(app_num) != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("Sorry couldn't unsubscribe\n");
      return(NCSCC_RC_FAILURE);
   }
   
   /** wait for a character **/
   if(!ncs_ifsv_red_demo_flag)
       getchar();

   /** destroy the application **/
   if (IfsvDtTestAppDestroy(app_num) != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("Sorry couldn't destroy the application\n");
      return(NCSCC_RC_FAILURE);
   }
   return (NCSCC_RC_SUCCESS);
}

/****************************************************************************
 * Name          : ifsv_drv_demo_start
 *
 * Description   : This is the wrapper function used to start the driver demo 
 *                 which initialize the driver and polls the interface to get
 *                 the present interface status and inform it to IfND. 
 *
 * Arguments     : info - This is info which demo requires.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
/** This is the function which is used to start demo **/
uns32
ifsv_drv_demo_start(NCSCONTEXT *info)
{
   uns32 res;
   NCS_IFSV_DRV_SVC_REQ drv_svc_req;

   m_NCS_MEMSET(&drv_svc_req, 0, sizeof(drv_svc_req));
   drv_svc_req.req_type           = NCS_IFSV_DRV_INIT_REQ;

   /** SE API used to initialize driver service request **/
   if (ncs_ifsv_drv_svc_req(&drv_svc_req) != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("Sorry couldn't able to destroy the application\n");
      return(NCSCC_RC_FAILURE);
   }

   /** Create a user spcae driver thread which polls on the interface and 
    ** let the applications known about the interface status
    **/
   if ((res = m_NCS_TASK_CREATE ((NCS_OS_CB)ifsv_drv_poll_intf_info,
   (NCSCONTEXT)NULL, "IFSV_DRV_DEMO", IFSV_DRV_DEMO_PRIORITY, IFSV_DRV_DEMO_STACK_SIZE,
   &task_hdl)) != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("Sorry couldn't able to create the task \n");
      return(NCSCC_RC_FAILURE);
   }
   if ((res = m_NCS_TASK_START(task_hdl)) != NCSCC_RC_SUCCESS)
   {
      m_NCS_CONS_PRINTF("Sorry couldn't able to create the task \n");
      return(NCSCC_RC_FAILURE);
   }
   return (NCSCC_RC_SUCCESS);
}
