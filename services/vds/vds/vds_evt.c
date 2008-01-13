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

  MODULE NAME: VDS_EVT.C
 
..............................................................................

  DESCRIPTION: This file contains VDS event specific routines


..............................................................................

  FUNCTIONS INCLUDED in this module:
  
******************************************************************************/
#include "vds.h"

/* Static function prototype */
static uns32 vds_process_vda_evt(VDS_CB *vds, VDS_EVT *vds_evt);
static void vds_process_amf_health_check_evt(VDS_CB *vds,
                                                VDS_EVT *vds_evt);
/****************************************************************************
  Name          :  vds_evt_create

  Description   :  Allocate and partially populate a VDS_EVT.

  Arguments     :  evt_data - void * pointer to 
                   evt - One of enum VDS_EVT_TYPE

  Return Values :  vds_evt: Success, a ptr to a newly created VDS_EVT
                   NULL:     Failure

  Notes         :
******************************************************************************/
VDS_EVT *vds_evt_create(NCSCONTEXT evt_data,
                        VDS_EVT_TYPE evt)
{
   VDS_EVT *vds_evt = NULL;
   VDS_VDA_EVT *vds_vda_msg = NULL;

   VDS_TRACE1_ARG1("vds_evt_create\n"); 

   if ((vds_evt = m_MMGR_ALLOC_VDS_EVT) == NULL)
   {
      m_VDS_LOG_MEM(VDS_LOG_MEM_VDS_EVT,
                            VDS_LOG_MEM_ALLOC_FAILURE,
                                    NCSFL_SEV_CRITICAL);
      return NULL;
   }
   

   m_NCS_OS_MEMSET((char *)vds_evt, 0, sizeof(VDS_EVT));

   /* Update the event type */
   vds_evt->evt_type = evt;

   switch (evt)
   {
   case VDS_VDA_EVT_TYPE:
       
       vds_vda_msg = ((VDS_VDA_EVT *)evt_data);
       vds_evt->evt_data.vda_evt_msg.dest = vds_vda_msg->dest; 
       vds_evt->evt_data.vda_evt_msg.i_msg_ctxt = vds_vda_msg->i_msg_ctxt;
       vds_evt->evt_data.vda_evt_msg.vda_info = (NCSVDA_INFO *)vds_vda_msg->vda_info;
       break;
   case VDS_AMF_HEALTH_CHECK_EVT_TYPE:
       vds_evt->evt_data.invocation = *((SaInvocationT *)evt_data);
       break;

   case VDS_VDA_DESTROY_EVT_TYPE:
       vds_evt->evt_data.vda_evt_msg.dest =*((MDS_DEST *)evt_data); 
       break;
   default:
       m_MMGR_FREE_VDS_EVT(vds_evt);
       vds_evt = NULL;
       break;
   }
   return vds_evt;
}


/****************************************************************************
  Name          :  vds_process_vda_evt

  Description   :  This routine process vda events.

  Arguments     :  vds - ptr to the VDS cb
                   vds_evt  - ptr to the VDS_EVT

  Return Values :  None.

  Notes         :  None.
******************************************************************************/
uns32 vds_process_vda_evt(VDS_CB *vds, VDS_EVT *vds_evt)
{
   NCSVDA_INFO *vda_info  = NULL;
   uns32 rc = NCSCC_RC_SUCCESS;

   VDS_TRACE1_ARG1("vds_process_vda_evt\n");
   
   vda_info = (NCSVDA_INFO *)vds_evt->evt_data.vda_evt_msg.vda_info;

   vda_info->o_result = NCSCC_RC_FAILURE; /* Initialize */

   switch(vda_info->req)
   {
   case NCSVDA_VDEST_CREATE:
      
      m_VDS_LOG_EVT(VDS_LOG_EVT_VDEST_CREATE,
                            VDS_LOG_EVT_NOTHING,
                                    NCSFL_SEV_INFO);      
      vda_info->o_result = vds_process_vdest_create(vds, &vds_evt->evt_data.vda_evt_msg.dest,
                                      &vda_info->info.vdest_create);
      break;

   case NCSVDA_VDEST_LOOKUP:

      m_VDS_LOG_EVT(VDS_LOG_EVT_VDEST_LOOKUP,
                            VDS_LOG_EVT_NOTHING,
                                    NCSFL_SEV_INFO);  
      vda_info->o_result = vds_process_vdest_lookup(vds, 
                                            &vda_info->info.vdest_lookup);
      break;

   case NCSVDA_VDEST_DESTROY:

      m_VDS_LOG_EVT(VDS_LOG_EVT_VDEST_DESTROY,
                              VDS_LOG_EVT_NOTHING,
                                      NCSFL_SEV_INFO);    
      vda_info->o_result = vds_process_vdest_destroy(vds, &vds_evt->evt_data.vda_evt_msg.dest,
                                     (NCSCONTEXT)&vda_info->info.vdest_destroy,
                                     TRUE);
      break;

   default:
      /* VDA always uses synchronous messaging */
      break;
   }

   rc = vds_mds_send(vds, vds_evt);
   return rc;
}


/****************************************************************************
  Name          :  vds_process_amf_health_check_evt

  Description   :  This routine gives response to health done by AMF.

  Arguments     :  vds - ptr to  VDS cb
                   vds_evt - ptr to VDS_EVT

  Return Values :  None.

  Notes         :  None.

****************************************************************************/
void vds_process_amf_health_check_evt(VDS_CB *vds,
                                   VDS_EVT *vds_evt)
{ 
   VDS_TRACE1_ARG1("vds_process_amf_health_check_evt\n");

   /* send the SA_AIS_OK response to the AVA */
   if (saAmfResponse(vds->amf.amf_handle, vds_evt->evt_data.invocation, SA_AIS_OK) != SA_AIS_OK)
   {
       m_VDS_LOG_HDL(VDS_LOG_AMF_RESPONSE,
                                 VDS_LOG_AMF_FAILURE,
                                         NCSFL_SEV_CRITICAL);
   }
   return;
}


/****************************************************************************
  Name          :  vds_evt_process
 
  Description   :  This routine processes the VDS event and destroys
                   the event.
 
  Arguments     :  evt - pointer to VDS_EVT
 
  Return Values :  None.
 
  Notes         :  None
******************************************************************************/
void vds_evt_process(VDS_EVT *evt)
{
   VDS_CB *vds = NULL;
   
   VDS_TRACE1_ARG1("vds_evt_process\n"); 
   
   /* validate the event type */
   if ((evt->evt_type < VDS_VDA_EVT_TYPE) ||
       (evt->evt_type >= VDS_MAX_EVT_TYPE))
   {      
      /* free the event */
      vds_evt_destroy(evt);
      return;
   }

   /* retrieve VDS cb */
   if ((vds = (VDS_CB *)ncshm_take_hdl(NCS_SERVICE_ID_VDS,
                                       gl_vds_hdl)) == NULL)
   {
      m_VDS_LOG_HDL(VDS_LOG_HDL_RETRIEVE_CB,
                            VDS_LOG_HDL_FAILURE,
                                    NCSFL_SEV_CRITICAL);
      vds_evt_destroy(evt);
      return;
   }
      
   /* acquire cb write lock */
   m_NCS_LOCK(&vds->cb_lock, NCS_LOCK_WRITE);

   switch (evt->evt_type)
   {
   case VDS_VDA_EVT_TYPE:
       
       /* check that VDA event are not processed in 
          other states apart from ACTIVE */
       if (vds->amf.ha_cstate == VDS_HA_STATE_ACTIVE) 
          /* process the VDA message */
          vds_process_vda_evt(vds, evt);
       break;

   case VDS_AMF_HEALTH_CHECK_EVT_TYPE:
       /* process the amf Health check event */
       vds_process_amf_health_check_evt(vds,evt);   
       break;
   case VDS_VDA_DESTROY_EVT_TYPE:
       vds_clean_non_persistent_vdests(vds, &evt->evt_data.vda_evt_msg.dest);
       break;
   default:
       break;
   }
   
   /* release cb write lock */
   m_NCS_UNLOCK(&vds->cb_lock, NCS_LOCK_WRITE);

   ncshm_give_hdl(gl_vds_hdl);

   /* free the event */
   vds_evt_destroy(evt);

   return;
}


/****************************************************************************
  Name          :  vds_evt_destroy
 
  Description   :  This routine frees VDS event.
 
  Arguments     :  evt - ptr to the VDS event
 
  Return Values :  None.
 
  Notes         :  None.
******************************************************************************/
void vds_evt_destroy (VDS_EVT *evt)
{
   uns32 type = evt->evt_type;

   if (!evt) return;

   if (type == VDS_VDA_EVT_TYPE)
   { 
                  
      m_VDS_LOG_MEM(VDS_LOG_MEM_VDA_INFO_FREE,
                            VDS_LOG_MEM_NOTHING,
                                    NCSFL_SEV_INFO);
      m_MMGR_FREE_NCSVDA_INFO(evt->evt_data.vda_evt_msg.vda_info); 
   }   

   /* Free the event now */
   m_MMGR_FREE_VDS_EVT(evt);
 
#if 0  
   m_VDS_LOG_MEM(VDS_LOG_MEM_VDS_EVT_FREE,
                            VDS_LOG_MEM_NOTHING,
                                    NCSFL_SEV_INFO);
#endif
   return;
}


