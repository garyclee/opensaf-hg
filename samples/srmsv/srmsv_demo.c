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


#include <stdio.h>

/* Common header files */
#include "ncsgl_defs.h"
#include "ncs_osprm.h"
#include "ncssysf_def.h"
#include "ncssysf_tsk.h"

#include "saAis.h"
#include "ncs_lib.h"

#include "mds_papi.h"

#include "srmsv_demo.h"
#include "srmsv_papi.h"
#include "srma_papi.h"

/*======================================================================
                      Global Variables
=======================================================================*/
/* SRMSv toolkit application task handle */
NCSCONTEXT gl_srmsv_task_hdl = 0;

NCS_SRMSV_HANDLE        gl_srmsv_hdl = 0;
NCS_SRMSV_RSRC_HANDLE   gl_rsrc_mon_hdl[50];
NCS_SRMSV_SUBSCR_HANDLE gl_subscr_hdl;
SaSelectionObjectT      gl_srmsv_sel_obj;
uns8                    gl_rsrc_count = 0;

/*======================================================================
                      Macro Definitions
=======================================================================*/
/* SRMSv toolkit application task related parameters  */
#define SRMSV_TASK_PRIORITY  (5)
#define SRMSV_STACKSIZE      NCS_STACKSIZE_HUGE

/* Configuration file from where the SRMSv demo reads resource-monitor 
   specific data */
#define SRMSV_TEST_CONF      "/etc/opt/opensaf/srmsv_test_conf"

NCS_SRMSV_RSRC_TYPE wm_rsrc_type = 0;
uns32 wm_rsrc_pid = 0;

/*======================================================================
                    Function prototype 
=======================================================================*/
static void srmsv_main_process(void);
static void srmsv_demo_subscr_rsrc_mon(void);
static void srmsv_demo_stop_rsrc_mon(void);
static void srmsv_demo_start_rsrc_mon(void);
static void srmsv_demo_get_watermark(void);
static void srmsv_demo_finalize(void);
static void srmsv_demo_scavanger(void);
static void srmsv_demo_cbkinfo(NCS_SRMSV_RSRC_CBK_INFO *rsrc_cbk_info);

static void srmsv_demo_read_mon_cfg(NCS_SRMSV_MON_INFO *mon_info,
                                    uns16 *line_num,
                                    NCS_BOOL *eof);
static void srmsv_demo_update_rsrc_type(NCS_SRMSV_RSRC_TYPE *rsrc_type, 
                                        char *buffer);
static void srmsv_demo_update_user_type(NCS_SRMSV_USER_TYPE *usr_type,
                                        char *buffer);
static void srmsv_demo_update_rsrc_loc(NCS_SRMSV_NODE_LOCATION *loc,
                                       char *buffer);
static void srmsv_demo_update_mon_type(NCS_SRMSV_MONITOR_TYPE *mon_type,
                                       char *buffer);
static void srmsv_demo_update_wm_type(NCS_SRMSV_WATERMARK_TYPE *wm_type,
                                      char *buffer);
static void srmsv_demo_modify_rsrc_mon(void);
static void srmsv_demo_update_threshold_cond(NCS_SRMSV_THRESHOLD_TEST_CONDITION *condition,
                                             char *buffer);
static void srmsv_demo_print_rsrc_val(NCS_SRMSV_VALUE *value);
static void srmsv_demo_print_wm_val(SRMSV_WATERMARK_VAL *watermarks);
static void srmsv_demo_print_rsrc_type(NCS_SRMSV_RSRC_TYPE rsrc_type);



/****************************************************************************
  Name          :  srmsv_demo_print_rsrc_val
 
  Description   :  This routine will print the contents of NCS_SRMSV_VALUE
 
  Arguments     :  value - ptr to the NCS_SRMSV_VALUE structure

  Return Values :  Nothing
 
  Notes         :  None
******************************************************************************/
void  srmsv_demo_print_rsrc_val(NCS_SRMSV_VALUE *value)
{
   if (value == NULL) return;

   switch (value->val_type)
   {
   case NCS_SRMSV_VAL_TYPE_FLOAT:
       m_NCS_CONS_PRINTF("\n Resource Statistics Value: %f", value->val.f_val);
       break;

   case NCS_SRMSV_VAL_TYPE_INT32:
       m_NCS_CONS_PRINTF("\n Resource Statistics Value: %lu", value->val.i_val32);
       break;

   case NCS_SRMSV_VAL_TYPE_UNS32:
       m_NCS_CONS_PRINTF("\n Resource Statistics Value: %lu", value->val.u_val32);
       break;

   case NCS_SRMSV_VAL_TYPE_INT64:
       m_NCS_CONS_PRINTF("\n Resource Statistics Value: %ll", value->val.i_val64);
       break;

   case NCS_SRMSV_VAL_TYPE_UNS64:
       m_NCS_CONS_PRINTF("\n Resource Statistics Value: %ll", value->val.u_val64);
       break;

   default:
      break;
   }

   return;
}


/****************************************************************************
  Name          :  srmsv_demo_print_rsrc_type
 
  Description   :  This routine prints Resource Type 
 
  Arguments     :  rsrc_type -  Type of the resource to be printed

  Return Values :  Nothing
 
  Notes         :  None
******************************************************************************/
void srmsv_demo_print_rsrc_type(NCS_SRMSV_RSRC_TYPE rsrc_type)
{
   switch(rsrc_type)
   {
   case NCS_SRMSV_RSRC_CPU_UTIL: 
       m_NCS_CONS_PRINTF("\n Resource Type:  NCS_SRMSV_RSRC_CPU_UTIL");
       break;

   case NCS_SRMSV_RSRC_CPU_KERNEL:     
       m_NCS_CONS_PRINTF("\n Resource Type:  NCS_SRMSV_RSRC_CPU_KERNEL");
       break;

   case NCS_SRMSV_RSRC_CPU_USER:       
       m_NCS_CONS_PRINTF("\n Resource Type:  NCS_SRMSV_RSRC_CPU_USER");
       break;

   case NCS_SRMSV_RSRC_CPU_UTIL_ONE:  
       m_NCS_CONS_PRINTF("\n Resource Type:  NCS_SRMSV_RSRC_CPU_UTIL_ONE");
       break;

   case NCS_SRMSV_RSRC_CPU_UTIL_FIVE:  
       m_NCS_CONS_PRINTF("\n Resource Type:  NCS_SRMSV_RSRC_CPU_UTIL_FIVE");
       break;

   case NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN: 
       m_NCS_CONS_PRINTF("\n Resource Type:  NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN");
       break;

   case NCS_SRMSV_RSRC_MEM_PHYSICAL_USED:
       m_NCS_CONS_PRINTF("\n Resource Type:  NCS_SRMSV_RSRC_MEM_PHYSICAL_USED");
       break;

   case NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE:
       m_NCS_CONS_PRINTF("\n Resource Type:  NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE");
       break;

   case NCS_SRMSV_RSRC_SWAP_SPACE_USED:
       m_NCS_CONS_PRINTF("\n Resource Type:  NCS_SRMSV_RSRC_SWAP_SPACE_USED");
       break;

   case NCS_SRMSV_RSRC_SWAP_SPACE_FREE:
       m_NCS_CONS_PRINTF("\n Resource Type:  NCS_SRMSV_RSRC_SWAP_SPACE_FREE");
       break;

   case NCS_SRMSV_RSRC_USED_BUFFER_MEM:
       m_NCS_CONS_PRINTF("\n Resource Type:  NCS_SRMSV_RSRC_USED_BUFFER_MEM");
       break;

   case NCS_SRMSV_RSRC_USED_CACHED_MEM:
       m_NCS_CONS_PRINTF("\n Resource Type:  NCS_SRMSV_RSRC_USED_CACHED_MEM");
       break;

   case NCS_SRMSV_RSRC_PROC_EXIT:  
       m_NCS_CONS_PRINTF("\n Resource Type:  NCS_SRMSV_RSRC_PROC_EXIT");
       break;

   case NCS_SRMSV_RSRC_PROC_MEM: 
       m_NCS_CONS_PRINTF("\n Resource Type:  NCS_SRMSV_RSRC_PROC_MEM");
       break;

   case NCS_SRMSV_RSRC_PROC_CPU:
       m_NCS_CONS_PRINTF("\n Resource Type:  NCS_SRMSV_RSRC_PROC_CPU");
       break;

   default:
       m_NCS_CONS_PRINTF("\n Invalid Resource Type");
       break;
   }

   return;
}


/****************************************************************************
  Name          :  srmsv_demo_print_wm_val
 
  Description   :  This routine will print the received watermark values
 
  Arguments     :  watermarks - ptr to the SRMSV_WATERMARK_VAL structure

  Return Values :  Nothing
 
  Notes         :  None
******************************************************************************/
void srmsv_demo_print_wm_val(SRMSV_WATERMARK_VAL *watermarks)
{
   if (!watermarks->wm_exist)
   {
      m_NCS_CONS_PRINTF("\n Watermark values doesn't exists for the rsrc");
      return;
   }

   /* Print the Resource Type */
   srmsv_demo_print_rsrc_type(watermarks->rsrc_type);

   switch(watermarks->wm_type)
   {
   case NCS_SRMSV_WATERMARK_HIGH:
       m_NCS_CONS_PRINTF("\n High Watermark Value");
       srmsv_demo_print_rsrc_val(&watermarks->high_val);
       break; 

   case NCS_SRMSV_WATERMARK_LOW:
       m_NCS_CONS_PRINTF("\n Low Watermark Value");
       srmsv_demo_print_rsrc_val(&watermarks->low_val);
       break; 

   case NCS_SRMSV_WATERMARK_DUAL:
       m_NCS_CONS_PRINTF("\n High Watermark Value");
       srmsv_demo_print_rsrc_val(&watermarks->high_val);

       m_NCS_CONS_PRINTF("\n Low Watermark Value");
       srmsv_demo_print_rsrc_val(&watermarks->low_val);
       break; 

   default:
       break;
   }

   return;
}


/****************************************************************************
  Name          :  srmsv_demo_cbkinfo
 
  Description   :  This routine will be called by SRMSv (agent) to notify the
                   user-application about the threshold events etc..
 
  Arguments     :  rsrc_cbk_info - ptr to the NCS_SRMSV_RSRC_CBK_INFO structure

  Return Values :  Node
 
  Notes         :  None
******************************************************************************/
void srmsv_demo_cbkinfo(NCS_SRMSV_RSRC_CBK_INFO *rsrc_cbk_info)
{
   if (rsrc_cbk_info == NULL)
      return;

   m_NCS_CONS_PRINTF("\n\n\n SRMSv NOTIFICATION:");

   switch (rsrc_cbk_info->notif_type)
   {
   case SRMSV_CBK_NOTIF_RSRC_THRESHOLD:
       m_NCS_CONS_PRINTF("\n Arrived from NODE: 0x%08x", rsrc_cbk_info->node_num);
       m_NCS_CONS_PRINTF("\n THRESHOLD event arrived for Resource Handle: 0x%x", rsrc_cbk_info->rsrc_mon_hdl);
       srmsv_demo_print_rsrc_val(&rsrc_cbk_info->notify.rsrc_value);
       break;

   case SRMSV_CBK_NOTIF_RSRC_MON_EXPIRED:
       m_NCS_CONS_PRINTF("\n RESOURCE EXPIRED event arrived for Resource Handle: 0x%x", rsrc_cbk_info->rsrc_mon_hdl);
       break;

   case SRMSV_CBK_NOTIF_WATERMARK_VAL:
       m_NCS_CONS_PRINTF("\n Arrived from NODE: 0x%08x", rsrc_cbk_info->node_num);
       m_NCS_CONS_PRINTF("\n WATERMARK event arrived");
       srmsv_demo_print_wm_val(&rsrc_cbk_info->notify.watermarks);
       break;

   case SRMSV_CBK_NOTIF_WM_CFG_ALREADY_EXIST:
       m_NCS_CONS_PRINTF("\n Arrived from NODE: 0x%08x", rsrc_cbk_info->node_num);
       m_NCS_CONS_PRINTF("\n WATERMARK already EXISTS for the Resource Handle: 0x%x", rsrc_cbk_info->rsrc_mon_hdl);
       break;

   default:
       break;  
   }

   return;
}


/****************************************************************************
  Name          :  srmsv_demo_finalize
 
  Description   :  This routine does the finalize of SRMSv service
 
  Arguments     :  Node
 
  Return Values :  Node
 
  Notes         :  None
******************************************************************************/
void srmsv_demo_finalize()
{         
   m_NCS_CONS_PRINTF("\n\n SRMSv-API: ncs_srmsv_finalize()");
   if (ncs_srmsv_finalize(gl_srmsv_hdl) == SA_AIS_OK)
   {  
      int i = 0;

      /* Clean the database */
      for (i=0; i<gl_rsrc_count; i++)
      {     
         gl_rsrc_mon_hdl[i] = 0;
      }
      gl_rsrc_count = 0;
      m_NCS_CONS_PRINTF("\n\n status: SUCCESS, SRMSv Handle: 0x%x", (uns32)gl_srmsv_hdl);
   }
   else 
   {
      m_NCS_CONS_PRINTF("\n\n status: FAILED, SRMSv Handle: 0x%x", (uns32)gl_srmsv_hdl);
   }

   return;
}


/****************************************************************************
  Name          :  srmsv_demo_update_rsrc_type
 
  Description   :  This function returns the respective rsrc type based on
                   the input string. 
 
  Arguments     :  buffer - rsrc type string
 
  Return Values :  *rsrc_type - returns the rsrc type.
 
  Notes         :  None
******************************************************************************/
void  srmsv_demo_update_rsrc_type(NCS_SRMSV_RSRC_TYPE *rsrc_type, char *buffer)
{
   if (!strcmp(buffer, "NCS_SRMSV_RSRC_PROC_EXIT"))
      *rsrc_type = NCS_SRMSV_RSRC_PROC_EXIT;
   else
   if (!strcmp(buffer, "NCS_SRMSV_RSRC_CPU_KERNEL"))
      *rsrc_type = NCS_SRMSV_RSRC_CPU_KERNEL;
   else
   if (!strcmp(buffer, "NCS_SRMSV_RSRC_CPU_USER"))
      *rsrc_type = NCS_SRMSV_RSRC_CPU_USER;
   else
   if (!strcmp(buffer, "NCS_SRMSV_RSRC_CPU_UTIL"))
      *rsrc_type = NCS_SRMSV_RSRC_CPU_UTIL;
   else
   if (!strcmp(buffer, "NCS_SRMSV_RSRC_CPU_UTIL_ONE"))
      *rsrc_type = NCS_SRMSV_RSRC_CPU_UTIL_ONE;
   else
   if (!strcmp(buffer, "NCS_SRMSV_RSRC_CPU_UTIL_FIVE"))
      *rsrc_type = NCS_SRMSV_RSRC_CPU_UTIL_FIVE;
   else
   if (!strcmp(buffer, "NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN"))
      *rsrc_type = NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN;
   else
   if (!strcmp(buffer, "NCS_SRMSV_RSRC_MEM_PHYSICAL_USED"))
      *rsrc_type = NCS_SRMSV_RSRC_MEM_PHYSICAL_USED;
   else
   if (!strcmp(buffer, "NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE"))
      *rsrc_type = NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE;
   else
   if (!strcmp(buffer, "NCS_SRMSV_RSRC_SWAP_SPACE_USED"))
      *rsrc_type = NCS_SRMSV_RSRC_SWAP_SPACE_USED;
   else
   if (!strcmp(buffer, "NCS_SRMSV_RSRC_SWAP_SPACE_FREE"))
      *rsrc_type = NCS_SRMSV_RSRC_SWAP_SPACE_FREE;
   else
   if (!strcmp(buffer, "NCS_SRMSV_RSRC_USED_BUFFER_MEM"))
      *rsrc_type = NCS_SRMSV_RSRC_USED_BUFFER_MEM;
   else
   if (!strcmp(buffer, "NCS_SRMSV_RSRC_USED_CACHED_MEM"))
      *rsrc_type = NCS_SRMSV_RSRC_USED_CACHED_MEM;
   else
   if (!strcmp(buffer, "NCS_SRMSV_RSRC_PROC_MEM"))
      *rsrc_type = NCS_SRMSV_RSRC_PROC_MEM;
   else
   if (!strcmp(buffer, "NCS_SRMSV_RSRC_PROC_CPU"))
      *rsrc_type = NCS_SRMSV_RSRC_PROC_CPU;
   else
      m_NCS_CONS_PRINTF("\n\n Unknown RESOURCE type: srmsv_demo_update_rsrc_type()");

   return;
}


/****************************************************************************
  Name          :  srmsv_demo_update_user_type
 
  Description   :  This function returns the respective user type based on
                   the input string. 
 
  Arguments     :  buffer - user type string
 
  Return Values :  *user_type - returns the user type.
 
  Notes         :  None
******************************************************************************/
void  srmsv_demo_update_user_type(NCS_SRMSV_USER_TYPE *usr_type, char *buffer)
{
   if (!strcmp(buffer, "NCS_SRMSV_USER_REQUESTOR"))
      *usr_type = NCS_SRMSV_USER_REQUESTOR;
   else
   if (!strcmp(buffer, "NCS_SRMSV_USER_SUBSCRIBER"))
      *usr_type = NCS_SRMSV_USER_SUBSCRIBER;
   else
   if (!strcmp(buffer, "NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR"))
      *usr_type = NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR;
   else
      m_NCS_CONS_PRINTF("\n\n Unknown USER type: srmsv_demo_update_user_type()");

   return;
}


/****************************************************************************
  Name          :  srmsv_demo_update_rsrc_loc
 
  Description   :  This function returns the respective resource location (on 
                   which NODE it resides) type based on the input string. 
 
  Arguments     :  buffer - rsrc-loc type string
 
  Return Values :  *loc - returns the location type.
 
  Notes         :  None
******************************************************************************/
void  srmsv_demo_update_rsrc_loc(NCS_SRMSV_NODE_LOCATION *loc, char *buffer)
{
   if (!strcmp(buffer, "NCS_SRMSV_NODE_LOCAL"))
      *loc = NCS_SRMSV_NODE_LOCAL;
   else
   if (!strcmp(buffer, "NCS_SRMSV_NODE_REMOTE"))
      *loc = NCS_SRMSV_NODE_REMOTE;
   else
   if (!strcmp(buffer, "NCS_SRMSV_NODE_ALL"))
      *loc = NCS_SRMSV_NODE_ALL;
   else
      m_NCS_CONS_PRINTF("\n\n Unknown LOCATION type: srmsv_demo_update_rsrc_loc()");
  
   return;
}


/****************************************************************************
  Name          :  srmsv_demo_update_mon_type
 
  Description   :  This function returns the respective monitor type based 
                   on the input string.
 
  Arguments     :  buffer - water-mark type string
 
  Return Values :  *mon_type - returns the watermark type.
 
  Notes         :  None
******************************************************************************/
void srmsv_demo_update_mon_type(NCS_SRMSV_MONITOR_TYPE *mon_type, char *buffer)
{
   if (!strcmp(buffer, "NCS_SRMSV_MON_TYPE_THRESHOLD"))
      *mon_type = NCS_SRMSV_MON_TYPE_THRESHOLD;
   else
   if (!strcmp(buffer, "NCS_SRMSV_MON_TYPE_WATERMARK"))
      *mon_type = NCS_SRMSV_MON_TYPE_WATERMARK;
   else
      m_NCS_CONS_PRINTF("\n\n Unknown MONITOR type: srmsv_demo_update_mon_type()");

   return;
}


/****************************************************************************
  Name          :  srmsv_demo_update_wm_type
 
  Description   :  This function returns the respective water-mark type based 
                   on the input string.
 
  Arguments     :  buffer - water-mark type string
 
  Return Values :  *wm_type - returns the watermark type.
 
  Notes         :  None
******************************************************************************/
void srmsv_demo_update_wm_type(NCS_SRMSV_WATERMARK_TYPE *wm_type, char *buffer)
{
   if (!strcmp(buffer, "NCS_SRMSV_WATERMARK_HIGH"))
      *wm_type = NCS_SRMSV_WATERMARK_HIGH;
   else
   if (!strcmp(buffer, "NCS_SRMSV_WATERMARK_LOW"))
      *wm_type = NCS_SRMSV_WATERMARK_LOW;
   else
   if (!strcmp(buffer, "NCS_SRMSV_WATERMARK_DUAL"))
      *wm_type = NCS_SRMSV_WATERMARK_DUAL;
   else
      m_NCS_CONS_PRINTF("\n\n Unknown WATERMARK type: srmsv_demo_update_wm_type()");

   return;
}


/****************************************************************************
  Name          :  srmsv_demo_update_threshold_cond
 
  Description   :  This function returns the respective test-condition based 
                   on the input string.
 
  Arguments     :  buffer - condition string.
 
  Return Values :  *cond - returns the test-condition type.
 
  Notes         :  None
******************************************************************************/
void  srmsv_demo_update_threshold_cond(NCS_SRMSV_THRESHOLD_TEST_CONDITION *cond,
                                       char *buffer)
{
   if (!strcmp(buffer, "NCS_SRMSV_THRESHOLD_VAL_IS_AT"))
      *cond = NCS_SRMSV_THRESHOLD_VAL_IS_AT;
   else
   if (!strcmp(buffer, "NCS_SRMSV_THRESHOLD_VAL_IS_NOT_AT"))
      *cond = NCS_SRMSV_THRESHOLD_VAL_IS_NOT_AT;
   else
   if (!strcmp(buffer, "NCS_SRMSV_THRESHOLD_VAL_IS_ABOVE"))
      *cond = NCS_SRMSV_THRESHOLD_VAL_IS_ABOVE;
   else
   if (!strcmp(buffer, "NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE"))
      *cond = NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE;
   else
   if (!strcmp(buffer, "NCS_SRMSV_THRESHOLD_VAL_IS_BELOW"))
      *cond = NCS_SRMSV_THRESHOLD_VAL_IS_BELOW;
   else
   if (!strcmp(buffer, "NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW"))
      *cond = NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW;
   else
      m_NCS_CONS_PRINTF("\n\n Unknown THRESHOLD CONDITION type: srmsv_demo_update_threshold_cond()");
  
   return;   
}


/****************************************************************************
  Name          :  srmsv_demo_read_mon_cfg
 
  Description   :  This function reads the rsrc-mon entries from the config 
                   file.
 
  Arguments     :  None.
 
  Return Values :  None.
 
  Notes         :  None
******************************************************************************/
void srmsv_demo_read_mon_cfg(NCS_SRMSV_MON_INFO *mon_info,
                        uns16 *line_num,
                        NCS_BOOL *eof)
{
   char line_buffer[1024];
   char item_buffer[128];
   char val_buffer[128];
   int len = 0;

   m_NCS_MEMSET(mon_info, 0, sizeof(NCS_SRMSV_MON_INFO));

   while (1)
   {
      m_NCS_MEMSET(line_buffer, 0, 1024);
      m_NCS_MEMSET(item_buffer, 0, 128);
      m_NCS_MEMSET(val_buffer, 0, 128);

      *line_num = (*line_num) + 1;

      /* get a line from the file */
      if (srmst_file_parser(SRMSV_TEST_CONF, line_buffer, *line_num)
         != NCSCC_RC_SUCCESS)
      { 
         *eof = TRUE;
         break;
      }

      /* Comment.. continue */      
      if (line_buffer[0] == '#') continue;  
      if (line_buffer[0] == '*') break;

      /* get the data from the line */
      if (srmst_str_parser(line_buffer, "=", item_buffer, 0)
          != NCSCC_RC_SUCCESS)  continue;

      /* get the data from the line */
      if (srmst_str_parser(line_buffer, "=", val_buffer, 1)
          != NCSCC_RC_SUCCESS) continue;
     
      len = strlen(val_buffer);
      val_buffer[len-1] = '\0';
 
      if (!strcmp(item_buffer, "RSRC_TYPE"))
         srmsv_demo_update_rsrc_type(&mon_info->rsrc_info.rsrc_type, val_buffer);
      else
      if (!strcmp(item_buffer, "USER_TYPE"))
         srmsv_demo_update_user_type(&mon_info->rsrc_info.usr_type, val_buffer);
      else
      if (!strcmp(item_buffer, "RSRC_PID"))
         mon_info->rsrc_info.pid = atol(val_buffer);
      else
      if (!strcmp(item_buffer, "CHILD_LEVEL"))
         mon_info->rsrc_info.child_level = atol(val_buffer);
      else
      if (!strcmp(item_buffer, "RSRC_LOC"))
         srmsv_demo_update_rsrc_loc(&mon_info->rsrc_info.srmsv_loc, val_buffer);
      else
      if (!strcmp(item_buffer, "RSRC_NODE_ID"))
         mon_info->rsrc_info.node_num = atol(val_buffer);
      else
      if (!strcmp(item_buffer, "MONITORING_RATE"))
         mon_info->monitor_data.monitoring_rate = atol(val_buffer);
      else
      if (!strcmp(item_buffer, "MONITORING_INTERVAL"))
         mon_info->monitor_data.monitoring_interval = atol(val_buffer);
      else
      if (!strcmp(item_buffer, "MONITOR_TYPE"))
         srmsv_demo_update_mon_type(&mon_info->monitor_data.monitor_type, val_buffer);
      else
      if (!strcmp(item_buffer, "WATERMARK_TYPE"))
         srmsv_demo_update_wm_type(&mon_info->monitor_data.mon_cfg.watermark_type, val_buffer);
      else
      if (!strcmp(item_buffer, "THRESHOLD_SAMPLES"))
         mon_info->monitor_data.mon_cfg.threshold.samples = atol(val_buffer);
      else
      if (!strcmp(item_buffer, "THRESHOLD_VAL"))
      {
         mon_info->monitor_data.mon_cfg.threshold.threshold_val.val_type = NCS_SRMSV_VAL_TYPE_FLOAT;
         mon_info->monitor_data.mon_cfg.threshold.threshold_val.scale_type = NCS_SRMSV_STAT_SCALE_PERCENT;
         mon_info->monitor_data.mon_cfg.threshold.threshold_val.val.f_val = atol(val_buffer);
      }
      else
      if (!strcmp(item_buffer, "THRESHOLD_COND"))
         srmsv_demo_update_threshold_cond(&mon_info->monitor_data.mon_cfg.threshold.condition, val_buffer);
      else
      if (!strcmp(item_buffer, "THRESHOLD_TL_VAL"))
         mon_info->monitor_data.mon_cfg.threshold.tolerable_val.val.u_val32 = atol(val_buffer);
      else
         m_NCS_CONS_PRINTF("\n\n Unknown Input in CONFIGURATION file: srmsv_demo_read_mon_cfg()");
   }
  
   return;
}


/****************************************************************************
  Name          :  srmsv_demo_get_watermark
 
  Description   :  This routine is send a request for watermark values
 
  Arguments     :  None.
 
  Return Values :  None.
 
  Notes         :  None
******************************************************************************/
void srmsv_demo_get_watermark()
{
   SaAisErrorT  saf_status = SA_AIS_OK; 
   NCS_SRMSV_RSRC_INFO rsrc_info;

   m_NCS_OS_MEMSET(&rsrc_info, 0, sizeof(NCS_SRMSV_RSRC_INFO ));

   m_NCS_TASK_SLEEP(10000); 
   m_NCS_CONS_PRINTF("\n\n SRMSv-API: ncs_srmsv_get_watermark_val()");

   /* Print the Resource Type */
   srmsv_demo_print_rsrc_type(wm_rsrc_type);

   rsrc_info.rsrc_type = wm_rsrc_type;

   if ((wm_rsrc_type == NCS_SRMSV_RSRC_PROC_MEM) ||
       (wm_rsrc_type == NCS_SRMSV_RSRC_PROC_CPU))
      rsrc_info.pid = wm_rsrc_pid;

   saf_status = ncs_srmsv_get_watermark_val(gl_srmsv_hdl,
                                            &rsrc_info,
                                            NCS_SRMSV_WATERMARK_DUAL);
   switch(saf_status)
   {
   case SA_AIS_OK:
       m_NCS_CONS_PRINTF("\n status: SUCCESS");
       break;

   case SA_AIS_ERR_NOT_EXIST:
       m_NCS_CONS_PRINTF("\n status: DESTINATION SRMND NOT EXIST");
       break;

   default:
       m_NCS_CONS_PRINTF("\n status: FAILED");
       break;
   }

   return;
}


/****************************************************************************
  Name          :  srmsv_demo_start_rsrc_mon
 
  Description   :  This routine is reads the rsrc-mon entries in the config 
                   file and cfg's them to the SRMSv service for monitoring.
 
  Arguments     :  None.
 
  Return Values :  None.
 
  Notes         :  None
******************************************************************************/
void srmsv_demo_start_rsrc_mon()
{
   uns16   line_num = 0;
   NCS_SRMSV_MON_INFO mon_info;
   NCS_BOOL eof = FALSE;
   NCS_SRMSV_RSRC_HANDLE rsrc_mon_hdl;
   SaAisErrorT  saf_status = SA_AIS_OK; 

   while (eof != TRUE)
   {
      srmsv_demo_read_mon_cfg(&mon_info, &line_num, &eof);
     
      rsrc_mon_hdl = 0;

      m_NCS_CONS_PRINTF("\n\n SRMSv-API: ncs_srmsv_start_rsrc_mon()");
      if (mon_info.monitor_data.monitor_type == NCS_SRMSV_MON_TYPE_WATERMARK)
      {
         wm_rsrc_type = mon_info.rsrc_info.rsrc_type;
         wm_rsrc_pid = mon_info.rsrc_info.pid;

         m_NCS_CONS_PRINTF("\n Monitor Type:  WATERMARK");
      }
      else
          m_NCS_CONS_PRINTF("\n Monitor Type:  THRESHOLD");

      /* Print the Resource Type */
      srmsv_demo_print_rsrc_type(mon_info.rsrc_info.rsrc_type);

      /*==================================================================
             Demonstrating the use of ncs_srmsv_start_rsrc_mon()
      ==================================================================*/
      saf_status = ncs_srmsv_start_rsrc_mon(gl_srmsv_hdl,
                                            &mon_info,
                                            &rsrc_mon_hdl); 
      switch(saf_status)
      {
      case SA_AIS_OK:
         m_NCS_CONS_PRINTF("\n status: SUCCESS,  rsrc_hdl: 0x%x", (uns32)rsrc_mon_hdl);
         gl_rsrc_mon_hdl[gl_rsrc_count++] = rsrc_mon_hdl;
         break;

      case SA_AIS_ERR_NOT_EXIST:
          m_NCS_CONS_PRINTF("\n status: DESTINATION SRMND NOT EXIST for NodeId: 0x%08x,  rsrc_hdl: 0x%x", mon_info.rsrc_info.node_num, (uns32)rsrc_mon_hdl);
          gl_rsrc_mon_hdl[gl_rsrc_count++] = rsrc_mon_hdl;
          break;

      default:
         m_NCS_CONS_PRINTF("\n status: FAILED");
         break;
      }
 
      if (eof) break;
   }

   return;
}


/****************************************************************************
  Name          :  srmsv_demo_stop_rsrc_mon
 
  Description   :  This routine deletes all the configured rsrc's from SRMSv
                   service (from monitoring).
 
  Arguments     :  None 

  Return Values :  Node
 
  Notes         :  None
******************************************************************************/
void srmsv_demo_stop_rsrc_mon()
{
   int i = 0;

   for (i=0; i<gl_rsrc_count; i++)
   {     
      m_NCS_CONS_PRINTF("\n\n SRMSv-API: ncs_srmsv_stop_rsrc_mon(), rsrc_hdl: 0x%x", (uns32)gl_rsrc_mon_hdl[i]);
      if (ncs_srmsv_stop_rsrc_mon(gl_rsrc_mon_hdl[i]) == SA_AIS_OK)
      {
         m_NCS_CONS_PRINTF("\n status: SUCCESS");
         gl_rsrc_mon_hdl[i] = 0;
      }
      else 
      { 
         m_NCS_CONS_PRINTF("\n status: FAILED");
      }
   }

   gl_rsrc_count = 0;
           
   return;
}


/****************************************************************************
  Name          :  srmsv_demo_subscr_rsrc_mon
 
  Description   :  This routine subscr's to the configured rsrc's (SRMSv
                   service for monitoring).
 
  Arguments     :  None 

  Return Values :  Node
 
  Notes         :  None
******************************************************************************/
void srmsv_demo_subscr_rsrc_mon()
{     
   NCS_SRMSV_RSRC_INFO rsrc_info;

   m_NCS_MEMSET(&rsrc_info, 0, sizeof(NCS_SRMSV_RSRC_INFO));
   rsrc_info.rsrc_type = NCS_SRMSV_RSRC_MEM_PHYSICAL_USED; 
     

   m_NCS_CONS_PRINTF("\n\n\n SRMSv-API: ncs_srmsv_subscr_rsrc_mon()");
   m_NCS_CONS_PRINTF("\n Monitor Type:  THRESHOLD");
   /* Print the Resource Type */
   srmsv_demo_print_rsrc_type(rsrc_info.rsrc_type);

   /*=====================================================================
              Demonstrating the use of ncs_srmsv_subscr_rsrc_mon()
   =====================================================================*/
   if (ncs_srmsv_subscr_rsrc_mon(gl_srmsv_hdl,
                                 &rsrc_info,
                                 &gl_subscr_hdl) == SA_AIS_OK)
      m_NCS_CONS_PRINTF("\n status: SUCCESS, subscr_hdl: 0x%x ", (uns32)gl_subscr_hdl);
   else 
      m_NCS_CONS_PRINTF("\n\n status: FAILED ");

   return;
}


/****************************************************************************
  Name          :  srmsv_demo_modify_rsrc_mon
 
  Description   :  This routine modifies the configured rsrc's (configured to
                   SRMSv service for monitoring).
 
  Arguments     :  None 

  Return Values :  Node
 
  Notes         :  None
******************************************************************************/
void srmsv_demo_modify_rsrc_mon()
{     
   NCS_SRMSV_MON_INFO mon_info;

   m_NCS_MEMSET(&mon_info, 0, sizeof(NCS_SRMSV_MON_INFO));
   mon_info.rsrc_info.rsrc_type = NCS_SRMSV_RSRC_MEM_PHYSICAL_USED; 
   mon_info.rsrc_info.usr_type = NCS_SRMSV_USER_REQUESTOR_AND_SUBSCR;
   mon_info.monitor_data.monitoring_rate = 10;
   mon_info.monitor_data.monitor_type = NCS_SRMSV_MON_TYPE_THRESHOLD;
   mon_info.monitor_data.monitoring_interval = 600;
   mon_info.monitor_data.mon_cfg.threshold.threshold_val.scale_type = NCS_SRMSV_STAT_SCALE_PERCENT;
   mon_info.monitor_data.mon_cfg.threshold.threshold_val.val.u_val32 = 90;
   mon_info.monitor_data.mon_cfg.threshold.condition = NCS_SRMSV_THRESHOLD_VAL_IS_ABOVE; 

   m_NCS_CONS_PRINTF("\n\n\n SRMSv-API: ncs_srmsv_start_rsrc_mon(), MODIFY rsrc_hdl: 0x%x", gl_rsrc_mon_hdl[5]);

   if (ncs_srmsv_start_rsrc_mon(gl_srmsv_hdl,
                                &mon_info,
                                &gl_rsrc_mon_hdl[5]) == SA_AIS_OK)
      m_NCS_CONS_PRINTF("\n status: SUCCESS");
   else 
      m_NCS_CONS_PRINTF("\n status: FAILED");

   return;
}


/****************************************************************************
  Name          :  srmsv_demo_scavanger
 
  Description   :  This routine cleans up all the SRMSv specific configuration
                   made by an application.
 
  Arguments     :  None 

  Return Values :  Node
 
  Notes         :  None
******************************************************************************/
void srmsv_demo_scavanger()   
{
   /*===============================================================
        Demonstrating the use of ncs_srmsv_unsubscr_rsrc_mon()
   ================================================================*/
   m_NCS_TASK_SLEEP(2000);
   m_NCS_CONS_PRINTF("\n\n SRMSv-API: ncs_srmsv_unsubscr_rsrc_mon(), subscr_hdl: 0x%x", (uns32)gl_subscr_hdl);
   if (ncs_srmsv_unsubscr_rsrc_mon(gl_subscr_hdl) == SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF("\n status: SUCCESS");
      gl_subscr_hdl = 0;
   }
   else 
      m_NCS_CONS_PRINTF("\n status: SUCCESS");

   /*===============================================================
           Demonstrating the use of ncs_srmsv_stop_rsrc_mon()
   ================================================================*/
   m_NCS_TASK_SLEEP(2000);
   srmsv_demo_stop_rsrc_mon(); 

   /*===============================================================
            Demonstrating the use of ncs_srmsv_finalize()
   ================================================================*/
   m_NCS_TASK_SLEEP(2000);
   srmsv_demo_finalize(); 

   m_NCS_CONS_PRINTF("\n SRMSv service DEMO OVER!!!");
   m_NCS_CONS_PRINTF("\n\n\n************ THANKYOU *************\n\n");

   exit(1);

   return;
}


/****************************************************************************
  Name          :  srmsv_main_process
 
  Description   :  This routine is an entry point for the SRMSv toolkit task.
                   It demonstrates the use of following SRMSv APIs

                      ncs_srmsv_initialize()
                      ncs_srmsv_selection_object_get()
                      ncs_srmsv_start_rsrc_mon()
                      ncs_srmsv_subscr_rsrc_mon()
                      ncs_srmsv_dispatch()
                      ncs_srmsv_modify_rsrc_mon()
                      ncs_srmsv_stop_monitor()
                      ncs_srmsv_restart_monitor()
                      ncs_srmsv_unsubscr_rsrc_mon()
                      ncs_srmsv_stop_rsrc_mon()
                      ncs_srmsv_finalize()
 
  Arguments     :  None.
 
  Return Values :  None.
 
  Notes         :  None
******************************************************************************/
void srmsv_main_process(void)
{
   NCS_SEL_OBJ_SET wait_sel_objs;
   NCS_SEL_OBJ     srmsv_sel_obj;
   SaAisErrorT     saf_status = SA_AIS_OK; 
   NCS_BOOL        demo_done = FALSE;
   NCS_BOOL        modify_done = FALSE;
   NCS_SRMSV_CALLBACK srmsv_callback = srmsv_demo_cbkinfo;


   /* this is to allow to establish MDS session with SRMSv 
   m_NCS_TASK_SLEEP(3000);*/

   /*=======================================================================
                Demonstrating the use of ncs_srmsv_initialize()
   =======================================================================*/
   m_NCS_CONS_PRINTF("\n\n SRMSv-API:  ncs_srmsv_initialize()\n");
   if (ncs_srmsv_initialize(srmsv_callback, &gl_srmsv_hdl) != SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF("\n status: FAILED \n");
      return;
   }
   else
      m_NCS_CONS_PRINTF("\n status: SUCCESS, ncs_srmsv_initialize(): SRMSv Handle: 0x%x \n", (uns32)gl_srmsv_hdl);

   /*======================================================================
           Demonstrating the use of ncs_srmsv_selection_object_get()
   ======================================================================*/
   /* m_NCS_TASK_SLEEP(2000); */
   /* Get the selection object corresponding to this SRMSv handle */
   m_NCS_CONS_PRINTF("\n SRMSv-API:  ncs_srmsv_selection_object_get()");
   if (ncs_srmsv_selection_object_get(gl_srmsv_hdl, &gl_srmsv_sel_obj) 
      != SA_AIS_OK)
   {
      m_NCS_CONS_PRINTF("\n status: FAILED \n");
      ncs_srmsv_finalize(gl_srmsv_hdl);
      return;
   }
   else
      m_NCS_CONS_PRINTF("\n status: SUCCESS \n");

   /*=====================================================================
              Demonstrating the use of ncs_srmsv_start_rsrc_mon()
   =====================================================================*/
   m_NCS_TASK_SLEEP(2000); 
   srmsv_demo_start_rsrc_mon();   

   /*=====================================================================
              Demonstrating the use of ncs_srmsv_subscr_rsrc_mon()
   =====================================================================*/
   m_NCS_TASK_SLEEP(2000); 
   srmsv_demo_subscr_rsrc_mon();

   /* m_NCS_TASK_SLEEP(2000); */

   /* reset the wait select objects */
   m_NCS_SEL_OBJ_ZERO(&wait_sel_objs);

   m_SET_FD_IN_SEL_OBJ((uns32)gl_srmsv_sel_obj,
                       srmsv_sel_obj);

   m_NCS_SEL_OBJ_SET(srmsv_sel_obj, &wait_sel_objs);

   /* now wait forever */
   while (m_NCS_SEL_OBJ_SELECT(srmsv_sel_obj, &wait_sel_objs, 
                               NULL, NULL, NULL) != -1)
   {
       /*=====================================================================
              Demonstrating the use of ncs_srmsv_get_watermark_val()
       =====================================================================*/
       srmsv_demo_get_watermark();   

      /* srmnd mbx processing */
      if (m_NCS_SEL_OBJ_ISSET(srmsv_sel_obj, &wait_sel_objs))
      {
         /* dispatch all the SRMSv pending function */
         saf_status = ncs_srmsv_dispatch(gl_srmsv_hdl, SA_DISPATCH_ALL);
         if (saf_status != SA_AIS_OK)
         {
            ncs_srmsv_finalize(gl_srmsv_hdl);
            return;
         }
          
         if (demo_done) 
         {
            srmsv_demo_scavanger();
            break;
         }
         else
         if (!(modify_done))
         {
            srmsv_demo_modify_rsrc_mon();
            modify_done = TRUE;
         }
         else
         {
            /*=====================================================================
                   Demonstrating the use of ncs_srmsv_stop_monitor()
            =====================================================================*/
            m_NCS_TASK_SLEEP(2000);
            m_NCS_CONS_PRINTF("\n\n SRMSv-API:  ncs_srmsv_stop_monitor(), srmsv_hdl: 0x%x", gl_srmsv_hdl);
            if (ncs_srmsv_stop_monitor(gl_srmsv_hdl) == SA_AIS_OK)
               m_NCS_CONS_PRINTF("\n status: SUCCESS");
            else 
               m_NCS_CONS_PRINTF("\n status: FAILED");

            /*=====================================================================
                   Demonstrating the use of ncs_srmsv_restart_monitor()
            =====================================================================*/
            m_NCS_TASK_SLEEP(2000);
            m_NCS_CONS_PRINTF("\n\n SRMSv-API: ncs_srmsv_restart_monitor(), srmsv_hdl: 0x%x", gl_srmsv_hdl);
            if (ncs_srmsv_restart_monitor(gl_srmsv_hdl) == SA_AIS_OK)
               m_NCS_CONS_PRINTF("\n status: SUCCESS");
            else 
               m_NCS_CONS_PRINTF("\n status: FAILED");

            m_NCS_TASK_SLEEP(2000);

            demo_done = TRUE;
         }
      }
         
      /* reset the wait select objects */
      m_NCS_SEL_OBJ_ZERO(&wait_sel_objs);

      /* again set the wait select objs */      
      m_NCS_SEL_OBJ_SET(srmsv_sel_obj, &wait_sel_objs);
   }

   return;
}


/****************************************************************************
  Name          :  ncs_srmsv_init
 
  Description   :  This routine intializes the SRMSv toolkit. It creates & 
                   starts the SRMSv task.
 
  Arguments     :  None.
 
  Return Values :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         :  None.
******************************************************************************/
uns32 ncs_srmsv_init(void)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Create SRMSv task */
   rc = m_NCS_TASK_CREATE((NCS_OS_CB)srmsv_main_process,
                          (void *)0,
                          "SRMSV",
                          SRMSV_TASK_PRIORITY,
                          SRMSV_STACKSIZE,
                          &gl_srmsv_task_hdl);
   if (rc != NCSCC_RC_SUCCESS)
   {
      goto err;
   }

   /* Start the task */
   rc = m_NCS_TASK_START(gl_srmsv_task_hdl);
   if (rc != NCSCC_RC_SUCCESS)
   {
      goto err;
   }

   m_NCS_CONS_PRINTF("\n\n SUCCESS: SRMSV TASK CREATION !!!");
   return rc;

err:
   /* destroy the task */
   if (gl_srmsv_task_hdl)
       m_NCS_TASK_RELEASE(gl_srmsv_task_hdl);

   m_NCS_CONS_PRINTF("\n\n SRMSV TASK CREATION FAILED !!! \n\n");
   return rc;
}


