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
      
DESCRIPTION:
        
This contains the EDS_CB functions.
          
******************************************************************************
*/
#include "eds.h"


/****************************************************************************
 * Name          : eds_cb_init
 *
 * Description   : This function initializes the EDS_CB including the 
 *                 Patricia trees.
 *                 
 *
 * Arguments     : eds_cb * - Pointer to the EDS_CB.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32
eds_cb_init(EDS_CB *eds_cb)
{
   NCS_PATRICIA_PARAMS     reg_param, cname_param ;
   
   m_NCS_MEMSET(&reg_param, 0, sizeof(NCS_PATRICIA_PARAMS));
   m_NCS_MEMSET(&cname_param, 0, sizeof(NCS_PATRICIA_PARAMS));
   
   reg_param.key_size = sizeof(uns32);
   cname_param.key_size = sizeof(SaNameT);

   /* Assign Initial HA state */
   eds_cb->ha_state = EDS_HA_INIT_STATE;
   eds_cb->csi_assigned=FALSE; 

   /* Assign Version. Currently, hardcoded, This will change later */
   m_GET_MY_VERSION(eds_cb->eds_version);

   /* Initialize default mib variables */
   eds_init_mib_objects(eds_cb);

   /* Initialize patricia tree for reg list*/
   if (NCSCC_RC_SUCCESS != ncs_patricia_tree_init(&eds_cb->eda_reg_list, &reg_param))
      return NCSCC_RC_FAILURE;
   
   /* Initialize patricia tree for channel name list*/
   if (NCSCC_RC_SUCCESS != ncs_patricia_tree_init(&eds_cb->eds_cname_list, &cname_param))
      return NCSCC_RC_FAILURE;
   
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : eds_cb_destroy
 *
 * Description   : This function destroys the EDS_CB including the 
 *                 Patricia trees.
 *                 
 *
 * Arguments     : eds_cb * - Pointer to the EDS_CB.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
void 
eds_cb_destroy(EDS_CB *eds_cb)
{
   ncs_patricia_tree_destroy(&eds_cb->eda_reg_list);
   /* Check if other lists are deleted as well */
   ncs_patricia_tree_destroy(&eds_cb->eds_cname_list);
   
   return;
}

/****************************************************************************
 * Name          : eds_init_mib_objects 
 *
 * Description   : This function initializes all scalar MIB objects to 
 *                 their default values.
 *                 
 * Arguments     : eds_cb * - Pointer to the EDS_CB.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void eds_init_mib_objects(EDS_CB *cb)
{
   m_NCS_OS_STRCPY(cb->scalar_objects.version.value,"B.01.01");
   cb->scalar_objects.version.length=m_NCS_STRLEN("B.01.01");
   m_GET_MY_VENDOR(cb->scalar_objects.vendor);
   cb->scalar_objects.prod_rev=2;
   cb->scalar_objects.svc_status=FALSE;
   cb->scalar_objects.svc_state=STARTING_UP;
   cb->scalar_objects.num_patterns=EDS_MAX_NUM_PATTERNS;
   cb->scalar_objects.max_pattern_size=EDS_MAX_PATTERN_SIZE;
   cb->scalar_objects.max_data_size=EDS_MAX_EVENT_DATA_SIZE;
   cb->scalar_objects.max_filter_num=EDS_MAX_NUM_FILTERS;
   cb->scalar_objects.max_filter_size=EDS_MAX_FILTER_SIZE;
   cb->scalar_objects.num_channels=0;
}

/****************************************************************************
 * Name          : eds_process_mbx
 *
 * Description   : This is the function which process the IPC mail box of 
 *                 EDS 
 *
 * Arguments     : mbx  - This is the mail box pointer on which EDS is 
 *                        going to block.  
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
static void
eds_process_mbx(SYSF_MBX *mbx)
{
   EDSV_EDS_EVT *evt = NULL;

   evt = (EDSV_EDS_EVT *) m_NCS_IPC_NON_BLK_RECEIVE(mbx,evt) ;
   if(evt != NULL)
   {
       if  ((evt->evt_type >= EDSV_EDS_EVT_BASE) &&
            (evt->evt_type < EDSV_EDS_EVT_MAX))
      {
            /* This event belongs to EDS main event dispatcher */
           eds_process_evt(evt);
      } 
      else
      {
          /* Free the event */
          m_LOG_EDSV_S(EDS_EVT_UNKNOWN,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,evt->evt_type,__FILE__,__LINE__,0);
          eds_evt_destroy(evt);
       }
   }
      return;
}

/****************************************************************************
 * Name          : eds_main_process
 *
 * Description   : This is the function which is given as a input to the 
 *                 EDS task.
 *                 This function will be select of both the FD's (AMF FD and
 *                 Mail Box FD), depending on which FD has been selected, it
 *                 will call the corresponding routines.
 *
 * Arguments     : mbx  - This is the mail box pointer on which EDS is 
 *                        going to block.  
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void
eds_main_process(SYSF_MBX *mbx)
{
   EDS_CB              *eds_cb = NULL;
   NCS_SEL_OBJ_SET     all_sel_obj;
   NCS_SEL_OBJ         mbx_fd, amf_ncs_sel_obj, mbcsv_sel_obj , numfds;
   SaAisErrorT         error = SA_AIS_OK;
   uns32               status;
   int                 count = 0;

   if (NULL == (eds_cb = (EDS_CB*) ncshm_take_hdl(NCS_SERVICE_ID_EDS,gl_eds_hdl)))
   {
       m_LOG_EDSV_S(EDS_CB_TAKE_HANDLE_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
      return;
   }

   
   /* Set the ServiceState MIB object */
   eds_cb->scalar_objects.svc_state=RUNNING;
   mbx_fd = m_NCS_IPC_GET_SEL_OBJ(&eds_cb->mbx);

   /* Give back the handle */
   ncshm_give_hdl(gl_eds_hdl);

   /* Set the fd for mbcsv events */
   m_SET_FD_IN_SEL_OBJ(eds_cb->mbcsv_sel_obj, mbcsv_sel_obj);
    
   while(1)
   {
      /* re-intialize the FDs and count */
      numfds.raise_obj = 0;
      numfds.rmv_obj = 0;
      m_NCS_SEL_OBJ_ZERO(&all_sel_obj);

      /* Get the selection object from the MBX */
      /* Set the fd for internal events */
      m_NCS_SEL_OBJ_SET(mbx_fd,&all_sel_obj);
      numfds = m_GET_HIGHER_SEL_OBJ(mbx_fd, numfds);
      
      /* Set the fd for amf events */
      if(eds_cb->amfSelectionObject != 0)
      {
         m_SET_FD_IN_SEL_OBJ(eds_cb->amfSelectionObject, amf_ncs_sel_obj);
         m_NCS_SEL_OBJ_SET(amf_ncs_sel_obj, &all_sel_obj);
         numfds = m_GET_HIGHER_SEL_OBJ(amf_ncs_sel_obj, numfds);
      }
   
      /* Add signal hdlr selection obj to readfds */
      if (eds_cb->sighdlr_sel_obj.rmv_obj != 0)
      {
         m_NCS_SEL_OBJ_SET(eds_cb->sighdlr_sel_obj , &all_sel_obj);
         numfds = m_GET_HIGHER_SEL_OBJ(eds_cb->sighdlr_sel_obj, numfds);
      }
       
       /* set the Mbcsv selection object */ 
       m_NCS_SEL_OBJ_SET(mbcsv_sel_obj, &all_sel_obj);
       numfds = m_GET_HIGHER_SEL_OBJ(mbcsv_sel_obj, numfds);

      /** EDS thread main processing loop.
       **/
      count = m_NCS_SEL_OBJ_SELECT(numfds, &all_sel_obj,0,0,0); 
      if(count > 0)
      {
        m_EDSV_DEBUG_CONS_PRINTF(" SELECT RETURNED\n");
        /* Signal handler FD is selected*/
        if(m_NCS_SEL_OBJ_ISSET(eds_cb->sighdlr_sel_obj, &all_sel_obj))
        {
            m_NCS_CONS_PRINTF(" EDS got SIGUSR1 signal \n");
            status = eds_amf_register(eds_cb);
            if(status != NCSCC_RC_SUCCESS)
            { 
               m_LOG_EDSV_S(EDS_AMF_REG_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,status,__FILE__,__LINE__,0);
               m_NCS_CONS_PRINTF("eds_main_process : eds_amf_register()- AMF Registration FAILED \n");
            }
            else
            {
               m_LOG_EDSV_S(EDS_AMF_REG_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_NOTICE,status,__FILE__,__LINE__,0);
               m_NCS_CONS_PRINTF("eds_main_process: AMF Registration SUCCESS...... \n");
            }
            m_NCS_SEL_OBJ_RMV_IND(eds_cb->sighdlr_sel_obj, TRUE, TRUE);
            m_NCS_SEL_OBJ_CLR(eds_cb->sighdlr_sel_obj, &all_sel_obj);
            m_NCS_SEL_OBJ_DESTROY(eds_cb->sighdlr_sel_obj);
            eds_cb->sighdlr_sel_obj.raise_obj = 0;
            eds_cb->sighdlr_sel_obj.rmv_obj = 0;/* Check whether this is needed or not */
        }
 
        /* process all the AMF messages */
        if (m_NCS_SEL_OBJ_ISSET(amf_ncs_sel_obj,&all_sel_obj))
        {
           m_EDSV_DEBUG_CONS_PRINTF("AMF EVENT HAS OCCURRED....\n");
           /* dispatch all the AMF pending function */
           error = saAmfDispatch(eds_cb->amf_hdl, SA_DISPATCH_ALL);
           if (error != SA_AIS_OK)
           {
              m_LOG_EDSV_S(EDS_AMF_DISPATCH_FAILURE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,error,__FILE__,__LINE__,0);
           }
        }
        /* process all mbcsv messages */
        if (m_NCS_SEL_OBJ_ISSET(mbcsv_sel_obj,&all_sel_obj))
        {
           m_EDSV_DEBUG_CONS_PRINTF("MBCSV EVENT HAS OCCURRED....\n");
           error = eds_mbcsv_dispatch(eds_cb->mbcsv_hdl);
           if(NCSCC_RC_SUCCESS != error)
              m_EDSV_DEBUG_CONS_PRINTF("MBCSV DISPATCH FAILED...\n");
           else
              m_EDSV_DEBUG_CONS_PRINTF("MBCSV DISPATCH SUCCESS...\n");
/*           mbcsv_prt_inv(); */
           m_NCS_SEL_OBJ_CLR(mbcsv_sel_obj,&all_sel_obj);
        }

        /* Process the EDS Mail box, if eds is ACTIVE. */
         if (m_NCS_SEL_OBJ_ISSET(mbx_fd, &all_sel_obj))
         {
             m_EDSV_DEBUG_CONS_PRINTF("MAILBOX EVENT HAS OCCURRED....\n");
             /* now got the IPC mail box event */
             eds_process_mbx(mbx);
          }
        
          /*** set the fd's again ***/
         /*eds_dump_reglist();
          eds_dump_worklist();*/
         
      }/*End of If */
      else
      { 
         m_NCS_CONS_PRINTF("eds_main_process: select FAILED ......\n");
      } 
    }

   return;
}
