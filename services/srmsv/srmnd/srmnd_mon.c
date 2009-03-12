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

  MODULE NAME: SRMND_MON.C
 
..............................................................................

  DESCRIPTION: This file contains the monitor specific routines defined for
               CPU specific, Memory specific, Process Specific resources.

..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/

#include "srmnd.h"

/* Static Function Prototypes */
static void srmnd_avg_rsrc_sample_stats(SRMND_MONITOR_INFO *mon_data,
                                        NCS_SRMSV_VALUE *avg_val);
static void srmnd_check_signed_threshold(NCS_SRMSV_THRESHOLD_CFG *threshold_cfg,
                                        NCS_SRMSV_VALUE *value,
                                        NCS_BOOL *condition_met);
static void srmnd_check_unsigned_threshold(NCS_SRMSV_THRESHOLD_CFG *threshold_cfg,
                                        NCS_SRMSV_VALUE *value,
                                        NCS_BOOL *condition_met);
static void srmnd_check_float_threshold(NCS_SRMSV_THRESHOLD_CFG *threshold_cfg,
                                        NCS_SRMSV_VALUE *value,
                                        NCS_BOOL *condition_met);
static uns32 srmnd_check_thresholds_and_notify(SRMND_CB *srmnd,
                                        SRMND_RSRC_MON_NODE *rsrc,
                                        NCS_SRMSV_VALUE *value);
static uns32 srmnd_check_memory_thresholds(SRMND_CB *srmnd,
                                        SRMND_RSRC_MON_NODE *rsrc);
static uns32 srmnd_check_cpu_thresholds(SRMND_CB *srmnd,
                                        SRMND_RSRC_MON_NODE *rsrc);
static uns32 srmnd_check_processes_exists(SRMND_CB *srmnd,
                                        SRMND_RSRC_MON_NODE *rsrc,
                                        NCS_BOOL *rsrc_delete);
static uns32 srmnd_check_cpu_ldavg_thresholds(SRMND_CB *srmnd,
                                        SRMND_RSRC_MON_NODE *rsrc);
static void srmnd_update_watermark_vals(SRMND_RSRC_MON_NODE *rsrc);
static void srmnd_update_watermark_val(NCS_SRMSV_VALUE *rsrc_val, 
                                        SRMND_WATERMARK_VAL *wm_val,
                                        NCS_SRMSV_WATERMARK_TYPE wm_type);
static uns32 srmnd_check_process_thresholds(SRMND_CB *srmnd,
                                            SRMND_RSRC_MON_NODE *rsrc,
                                            NCS_BOOL *rsrc_delete);


/****************************************************************************
  Name          :  srmnd_avg_rsrc_sample_stats
 
  Description   :  This routine calculates the average of statistic samples 
                   of a resource being monitor.

  Arguments     :  mon_data - ptr to the SRMND_MONITOR_INFO struct                   
                   avg_val - average of resource-mon statistics sample

  Return Values :  Nothing
 
  Notes         :  None.
******************************************************************************/
void srmnd_avg_rsrc_sample_stats(SRMND_MONITOR_INFO *mon_data,
                                 NCS_SRMSV_VALUE *avg_val)   
{
   uns64 u_total = 0;
   int64 i_total = 0;
   double f_total = 0;
   uns32 samples = mon_data->monitor_data.mon_cfg.threshold.samples;   
   SRMND_SAMPLE_DATA *sample_data = mon_data->sample_data;
   NCS_SRMSV_VAL_TYPE val_type = mon_data->result_value.val_type;
     
   /* if samples are > 1 then get the sum of all samples */
   if (samples > 1)
   {      
      while (sample_data)
      {
         switch (val_type)
         {
         case NCS_SRMSV_VAL_TYPE_FLOAT:
             f_total += sample_data->value.val.f_val;
             break;

         case NCS_SRMSV_VAL_TYPE_INT32:
             i_total += sample_data->value.val.i_val32;
             break;

         case NCS_SRMSV_VAL_TYPE_UNS32:
             u_total += sample_data->value.val.u_val32;
             break;

         case NCS_SRMSV_VAL_TYPE_INT64:
             i_total += sample_data->value.val.i_val64;
             break;

         case NCS_SRMSV_VAL_TYPE_UNS64:
             u_total += sample_data->value.val.u_val64;
             break;

         default:
             break;
         }

         sample_data = sample_data->next_sample;
      }
   }
   else
   {
      mon_data->monitor_data.mon_cfg.threshold.samples = 1;   
      samples = 1;
   }

   switch (val_type)
   {
   case NCS_SRMSV_VAL_TYPE_FLOAT:
       if (samples == 1)
          avg_val->val.f_val = mon_data->result_value.val.f_val;
       else
       {
          /* Calculate an avg of sum */
          avg_val->val.f_val = (double)(f_total/samples);
       }
       break;

   case NCS_SRMSV_VAL_TYPE_INT32:
       if (samples == 1)
          avg_val->val.i_val32 = mon_data->result_value.val.i_val32;
       else
       {
          /* Calculate an avg of sum */
          avg_val->val.i_val32 = (int32)(i_total/samples);
       }
       break;

   case NCS_SRMSV_VAL_TYPE_UNS32:
       if (samples == 1)
          avg_val->val.u_val32 = mon_data->result_value.val.u_val32;
       else
       {
          /* Calculate an avg of sum */
          avg_val->val.u_val32 = (uns32)(u_total/samples);
       }
       break;

   case NCS_SRMSV_VAL_TYPE_INT64:
       if (samples == 1)
          avg_val->val.i_val64 = mon_data->result_value.val.i_val64;
       else
       {
          /* Calculate an avg of sum */
          avg_val->val.i_val64 = (int64)(i_total/samples);
       }
       break;

   case NCS_SRMSV_VAL_TYPE_UNS64:
       if (samples == 1)
          avg_val->val.u_val64 = mon_data->result_value.val.u_val64;
       else
       {
          /* Calculate an avg of sum */
          avg_val->val.u_val64 = (uns64)(u_total/samples);
       }
       break;

   default:
       break;
   }
      
   return;
}


/****************************************************************************
  Name          :  srmnd_check_signed_threshold
 
  Description   :  This routine checks the resource monitor statistic values
                   meets the requested test-condition against the defined
                   threshold value.

  Arguments     :  threshold_cfg - ptr to the NCS_SRMSV_THRESHOLD_CFG struct                   
                   value - rsrc statistic value to check
                   condition_met - TRUE if test-condition satisfies else
                                   FALSE

  Return Values :  Nothing
 
  Notes         :  done for SIGNED resource statistic values.
******************************************************************************/
void srmnd_check_signed_threshold(NCS_SRMSV_THRESHOLD_CFG *threshold_cfg,
                                  NCS_SRMSV_VALUE *value,
                                  NCS_BOOL *condition_met)
{
   int64 threshold_val = 0;
   int64 tolerable_val = 0;
   int64 min_val = 0;
   int64 max_val = 0;
   int64 avg_val = 0;


   switch (threshold_cfg->threshold_val.val_type)
   {
   case NCS_SRMSV_VAL_TYPE_INT32:
       threshold_val = threshold_cfg->threshold_val.val.i_val32;
       tolerable_val = threshold_cfg->tolerable_val.val.i_val32;
       avg_val = value->val.i_val32;
       break;

   case NCS_SRMSV_VAL_TYPE_INT64:
       threshold_val = threshold_cfg->threshold_val.val.i_val64;
       tolerable_val = threshold_cfg->tolerable_val.val.i_val64;
       avg_val = value->val.i_val64;
       break;

   default:
       break;
   }
   
   switch (threshold_cfg->condition)
   {
   case NCS_SRMSV_THRESHOLD_VAL_IS_AT:
       {
          min_val = (threshold_val - tolerable_val);
          max_val = (threshold_val + tolerable_val);

          if ((avg_val >= min_val) && (avg_val <= max_val))
             *condition_met = TRUE;       
       }
       break;

   case NCS_SRMSV_THRESHOLD_VAL_IS_NOT_AT:
       {
          min_val = (threshold_val - tolerable_val);
          max_val = (threshold_val + tolerable_val);

          if ((avg_val < min_val) && (avg_val > max_val))
             *condition_met = TRUE;
       }
       break;

   case NCS_SRMSV_THRESHOLD_VAL_IS_ABOVE:
       if (avg_val > threshold_val) 
          *condition_met = TRUE;
       break;

   case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE:
       if (avg_val >= threshold_val) 
          *condition_met = TRUE;
       break;

   case NCS_SRMSV_THRESHOLD_VAL_IS_BELOW:
       if (avg_val < threshold_val) 
          *condition_met = TRUE;
       break;

   case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW:
       if (avg_val <= threshold_val) 
          *condition_met = TRUE;
       break;

   default:
       break;
   }

   return;
}


/****************************************************************************
  Name          :  srmnd_check_unsigned_threshold
 
  Description   :  This routine checks the resource monitor statistic values
                   meets the requested test-condition against the defined
                   threshold value.

  Arguments     :  threshold_cfg - ptr to the NCS_SRMSV_THRESHOLD_CFG struct                   
                   value - rsrc statistic value to check
                   condition_met - TRUE if test-condition satisfies else
                                   FALSE

  Return Values :  Nothing
 
  Notes         :  done for UNSIGNED resource statistic values.
******************************************************************************/
void srmnd_check_unsigned_threshold(NCS_SRMSV_THRESHOLD_CFG *threshold_cfg,
                                    NCS_SRMSV_VALUE *value,
                                    NCS_BOOL *condition_met)
{
   uns64 threshold_val = 0;
   uns64 tolerable_val = 0;
   uns64 min_val = 0;
   uns64 max_val = 0;
   uns64 avg_val = 0;

   switch (threshold_cfg->threshold_val.val_type)
   {
   case NCS_SRMSV_VAL_TYPE_UNS32:
       threshold_val = threshold_cfg->threshold_val.val.u_val32;
       tolerable_val = threshold_cfg->tolerable_val.val.u_val32;
       avg_val = value->val.u_val32;
       break;

   case NCS_SRMSV_VAL_TYPE_UNS64:
       threshold_val = threshold_cfg->threshold_val.val.u_val64;
       tolerable_val = threshold_cfg->tolerable_val.val.u_val64;
       avg_val = value->val.u_val64;
       break;

   default:
       break;
   }
   
   switch (threshold_cfg->condition)
   {
   case NCS_SRMSV_THRESHOLD_VAL_IS_AT:
       {
          min_val = (threshold_val - tolerable_val);
          max_val = (threshold_val + tolerable_val);

          if ((avg_val >= min_val) && (avg_val <= max_val))
             *condition_met = TRUE;       
       }
       break;

   case NCS_SRMSV_THRESHOLD_VAL_IS_NOT_AT:
       {
          min_val = (threshold_val - tolerable_val);
          max_val = (threshold_val + tolerable_val);

          if ((avg_val < min_val) && (avg_val > max_val))
             *condition_met = TRUE;
       }
       break;

   case NCS_SRMSV_THRESHOLD_VAL_IS_ABOVE:
       if (avg_val > threshold_val) 
          *condition_met = TRUE;
       break;

   case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE:
       if (avg_val >= threshold_val) 
          *condition_met = TRUE;
       break;

   case NCS_SRMSV_THRESHOLD_VAL_IS_BELOW:
       if (avg_val < threshold_val) 
          *condition_met = TRUE;
       break;

   case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW:
       if (avg_val <= threshold_val) 
          *condition_met = TRUE;
       break;

   default:
       break;
   }

   return;
}


/****************************************************************************
  Name          :  srmnd_check_float_threshold
 
  Description   :  This routine checks the resource monitor statistic values
                   meets the requested test-condition against the defined
                   threshold value.

  Arguments     :  threshold_cfg - ptr to the NCS_SRMSV_THRESHOLD_CFG struct                   
                   value - rsrc statistic value to check
                   condition_met - TRUE if test-condition satisfies else
                                   FALSE

  Return Values :  Nothing
 
  Notes         :  done for FLOAT resource statistic values.
******************************************************************************/
void srmnd_check_float_threshold(NCS_SRMSV_THRESHOLD_CFG *threshold_cfg,
                                 NCS_SRMSV_VALUE *value,
                                 NCS_BOOL *condition_met)
{
   double threshold_val = 0;
   double tolerable_val = 0;
   double min_val = 0;
   double max_val = 0;
   double avg_val = 0;

   switch (threshold_cfg->threshold_val.val_type)
   {
   case NCS_SRMSV_VAL_TYPE_FLOAT:
       threshold_val = threshold_cfg->threshold_val.val.f_val;
       tolerable_val = threshold_cfg->tolerable_val.val.f_val;
       avg_val = value->val.f_val;
       break;

   default:
       break;
   }
   
   switch (threshold_cfg->condition)
   {
   case NCS_SRMSV_THRESHOLD_VAL_IS_AT:
       {
          min_val = (threshold_val - tolerable_val);
          max_val = (threshold_val + tolerable_val);

          if ((avg_val >= min_val) && (avg_val <= max_val))
             *condition_met = TRUE;       
       }
       break;

   case NCS_SRMSV_THRESHOLD_VAL_IS_NOT_AT:
       {
          min_val = (threshold_val - tolerable_val);
          max_val = (threshold_val + tolerable_val);

          if ((avg_val < min_val) && (avg_val > max_val))
             *condition_met = TRUE;
       }
       break;

   case NCS_SRMSV_THRESHOLD_VAL_IS_ABOVE:
       if (avg_val > threshold_val) 
          *condition_met = TRUE;
       break;

   case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_ABOVE:
       if (avg_val >= threshold_val) 
          *condition_met = TRUE;
       break;

   case NCS_SRMSV_THRESHOLD_VAL_IS_BELOW:
       if (avg_val < threshold_val) 
          *condition_met = TRUE;
       break;

   case NCS_SRMSV_THRESHOLD_VAL_IS_AT_OR_BELOW:
       if (avg_val <= threshold_val) 
          *condition_met = TRUE;
       break;

   default:
       break;
   }

   return;
}


/****************************************************************************
  Name          :  srmnd_check_thresholds_and_notify
 
  Description   :  This routine checks the resource monitor statistic values
                   meets the requested test-condition against the defined
                   threshold value and it notifies the user-application if 
                   the test-condition is met.

  Arguments     :  srmnd - ptr to the SRMND CB                  
                   rsrc  - ptr to the resource monitor record
                   value - Resource statistic value to check against threshold
                           value.

  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  
******************************************************************************/
uns32 srmnd_check_thresholds_and_notify(SRMND_CB *srmnd,
                                        SRMND_RSRC_MON_NODE *rsrc,
                                        NCS_SRMSV_VALUE *value)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   NCS_BOOL condition_met = FALSE;
   NCS_SRMSV_THRESHOLD_CFG *threshold_cfg;
       
   /* Hold the ptr of Threshold configuration data */
   threshold_cfg = &rsrc->mon_data.monitor_data.mon_cfg.threshold;

   switch (threshold_cfg->threshold_val.val_type)
   {
   case NCS_SRMSV_VAL_TYPE_FLOAT:
       srmnd_check_float_threshold(threshold_cfg,
                                   value,
                                   &condition_met);
       break;

   case NCS_SRMSV_VAL_TYPE_UNS32:
   case NCS_SRMSV_VAL_TYPE_UNS64:
       srmnd_check_unsigned_threshold(threshold_cfg,
                                      value,
                                      &condition_met);
       break;

   case NCS_SRMSV_VAL_TYPE_INT32:
   case NCS_SRMSV_VAL_TYPE_INT64:
       srmnd_check_signed_threshold(threshold_cfg,
                                    value,
                                    &condition_met);
       break;
   
   default:
       break;
   }
   
   /* Oh! the test condition is met.. so don't wait to notify the 
      respective subscriber(s) */
   if (condition_met)
   {
      /* Already notified and no subscriber to sent, so just ignore */ 
      if ((rsrc->sent_flag & RSRC_APPL_NOTIF_SENT) && 
          (!(rsrc->sent_flag & RSRC_APPL_SUBS_SENT_PENDING)))
         return rc;
      
      /* Update the types of value */ 
      value->val_type = threshold_cfg->threshold_val.val_type;
      value->scale_type = threshold_cfg->threshold_val.scale_type;

      /* Update the notification value */
      rsrc->notify_val = *value;

      rc = srmnd_send_msg(srmnd, rsrc, SRMND_APPL_NOTIF_MSG);
      if (rc == NCSCC_RC_SUCCESS)
      {
         /* The respective notification has been sent */
         rsrc->sent_flag |= (RSRC_APPL_NOTIF_SENT);
      }
   }
   else  /* Condition doesn't met, ok! reset the sent flag */
      rsrc->sent_flag &= ~(RSRC_APPL_NOTIF_SENT);

   return rc;
}


/****************************************************************************
  Name          :  srmnd_update_watermark_val
 
  Description   :  This routine updates the watermark level values (LOW || 
                   HIGH) based on the watermark type.

  Arguments     :  rsrc_val  - Current resource value
                   wm_val - Recorded watermark level values
                   wm_type - Water Mark type

  Return Values :  Nothing
 
  Notes         :  
******************************************************************************/
void srmnd_update_watermark_val(NCS_SRMSV_VALUE *rsrc_val, 
                                SRMND_WATERMARK_VAL *wm_val,
                                NCS_SRMSV_WATERMARK_TYPE wm_type)
{
   switch (rsrc_val->val_type)
   {
   case NCS_SRMSV_VAL_TYPE_FLOAT:
       if ((wm_val->l_val.val.f_val > rsrc_val->val.f_val) &&
           ((wm_type == NCS_SRMSV_WATERMARK_LOW) ||
            (wm_type == NCS_SRMSV_WATERMARK_DUAL)))
          wm_val->l_val.val.f_val = rsrc_val->val.f_val;

       if ((wm_val->h_val.val.f_val < rsrc_val->val.f_val) &&
           ((wm_type == NCS_SRMSV_WATERMARK_HIGH) ||
            (wm_type == NCS_SRMSV_WATERMARK_DUAL)))
          wm_val->h_val.val.f_val = rsrc_val->val.f_val;
       break;

   case NCS_SRMSV_VAL_TYPE_UNS32:
       if ((wm_val->l_val.val.u_val32 > rsrc_val->val.u_val32) &&
           ((wm_type == NCS_SRMSV_WATERMARK_LOW) ||
            (wm_type == NCS_SRMSV_WATERMARK_DUAL)))
          wm_val->l_val.val.u_val32 = rsrc_val->val.u_val32;

       if ((wm_val->h_val.val.u_val32 < rsrc_val->val.u_val32) &&
           ((wm_type == NCS_SRMSV_WATERMARK_HIGH) ||
            (wm_type == NCS_SRMSV_WATERMARK_DUAL)))
          wm_val->h_val.val.u_val32 = rsrc_val->val.u_val32;
       break;

   case NCS_SRMSV_VAL_TYPE_UNS64:
       if ((wm_val->l_val.val.u_val64 > rsrc_val->val.u_val64) &&
           ((wm_type == NCS_SRMSV_WATERMARK_LOW) ||
            (wm_type == NCS_SRMSV_WATERMARK_DUAL)))
          wm_val->l_val.val.u_val64 = rsrc_val->val.u_val64;

       if ((wm_val->h_val.val.u_val64 < rsrc_val->val.u_val64) &&
           ((wm_type == NCS_SRMSV_WATERMARK_HIGH) ||
            (wm_type == NCS_SRMSV_WATERMARK_DUAL)))
          wm_val->h_val.val.u_val64 = rsrc_val->val.u_val64;
       break;

   case NCS_SRMSV_VAL_TYPE_INT32:
      if ((wm_val->l_val.val.i_val32 > rsrc_val->val.i_val32) &&
           ((wm_type == NCS_SRMSV_WATERMARK_LOW) ||
            (wm_type == NCS_SRMSV_WATERMARK_DUAL)))
          wm_val->l_val.val.i_val32 = rsrc_val->val.i_val32;

       if ((wm_val->h_val.val.i_val32 < rsrc_val->val.i_val32) &&
           ((wm_type == NCS_SRMSV_WATERMARK_HIGH) ||
            (wm_type == NCS_SRMSV_WATERMARK_DUAL)))
          wm_val->h_val.val.i_val32 = rsrc_val->val.i_val32;
       break;

   case NCS_SRMSV_VAL_TYPE_INT64:
       if ((wm_val->l_val.val.i_val64 > rsrc_val->val.i_val64) &&
           ((wm_type == NCS_SRMSV_WATERMARK_LOW) ||
            (wm_type == NCS_SRMSV_WATERMARK_DUAL)))
          wm_val->l_val.val.i_val64 = rsrc_val->val.i_val64;

       if ((wm_val->h_val.val.i_val64 < rsrc_val->val.i_val64) &&
           ((wm_type == NCS_SRMSV_WATERMARK_HIGH) ||
            (wm_type == NCS_SRMSV_WATERMARK_DUAL)))
          wm_val->h_val.val.i_val64 = rsrc_val->val.i_val64;
       break;
   
   default:
       break;
   }

   return;
}


/****************************************************************************
  Name          :  srmnd_update_watermark_vals
 
  Description   :  This routine checks the watermark level values of a 
                   resource and updates/records the respective LOW || HIGH
                   values depends on the watermark type.

  Arguments     :  rsrc  - ptr to the resource monitor record

  Return Values :  Nothing
 
  Notes         :  
******************************************************************************/
void srmnd_update_watermark_vals(SRMND_RSRC_MON_NODE *rsrc)
{
   NCS_SRMSV_VALUE     *rsrc_val = &rsrc->mon_data.result_value;
   SRMND_WATERMARK_VAL *wm_val = &rsrc->mon_data.wm_val;
   NCS_SRMSV_WATERMARK_TYPE wm_type = rsrc->mon_data.monitor_data.mon_cfg.watermark_type;


   /* Based on the watermark type take appropriate action */
   switch (wm_type)
   {
   case NCS_SRMSV_WATERMARK_HIGH:
       /* None of the value has been updated, 
          so update the rsrc current value */
       if (wm_val->h_val.val_type == 0) 
          wm_val->h_val = *rsrc_val;
       else
          srmnd_update_watermark_val(rsrc_val, wm_val, wm_type);      
       break;

   case NCS_SRMSV_WATERMARK_LOW:
       /* None of the value has been updated, 
          so update the rsrc current value */
       if (wm_val->l_val.val_type == 0)
          wm_val->l_val = *rsrc_val;
       else
          srmnd_update_watermark_val(rsrc_val, wm_val, wm_type);
       break;

   case NCS_SRMSV_WATERMARK_DUAL:
       /* None of the value has been updated, 
          so update the rsrc current value */
       if ((wm_val->h_val.val_type == 0) || (wm_val->l_val.val_type == 0))
       {
          wm_val->h_val = *rsrc_val;
          wm_val->l_val = *rsrc_val;
       }
       else
          srmnd_update_watermark_val(rsrc_val, wm_val, wm_type);
       break;

   default:
       break;
   }
   
   return;
}


/****************************************************************************
  Name          :  srmnd_check_memory_thresholds
 
  Description   :  This routine checks the memory specific resource statistic
                   values meets the requested test-condition against the 
                   defined threshold value and it notifies the user-application
                   if the test-condition is met.

  Arguments     :  srmnd - ptr to the SRMND CB                  
                   rsrc  - ptr to the resource monitor record

  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  
******************************************************************************/
uns32 srmnd_check_memory_thresholds(SRMND_CB *srmnd, SRMND_RSRC_MON_NODE *rsrc)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 samples = 0;
   NCS_SRMSV_VALUE mem_avg;

   
   memset(&mem_avg, 0, sizeof(NCS_SRMSV_VALUE));

   if ((rc = srmnd_get_memory_utilization_stats(rsrc)) != NCSCC_RC_SUCCESS)
      return rc;

   switch(rsrc->mon_data.monitor_data.monitor_type)
   {
   case NCS_SRMSV_MON_TYPE_THRESHOLD:
       samples = rsrc->mon_data.monitor_data.mon_cfg.threshold.samples;
       /* Still samples need to update */
       if ((samples > 1) && (rsrc->mon_data.samples_updated < samples))
          return rc;

       /* Get an average of resource statistics samples */
       srmnd_avg_rsrc_sample_stats(&rsrc->mon_data, &mem_avg);   

       rc = srmnd_check_thresholds_and_notify(srmnd, rsrc, &mem_avg);
       break;

   case NCS_SRMSV_MON_TYPE_WATERMARK:
       /* Check the current rsrc value with the recorded rsrc 
          specific WM values and do the updations accordingly */
       srmnd_update_watermark_vals(rsrc);
       break;

   case NCS_SRMSV_MON_TYPE_LEAKY_BUCKET:
       /* TBD */
       break;

   default:
       break;
   }

   return rc;
}


/****************************************************************************
  Name          :  srmnd_check_cpu_ldavg_thresholds
 
  Description   :  This routine checks the CPU load avg specific statistic
                   values meets the requested test-condition against the 
                   defined threshold value and it notifies the user-application
                   if the test-condition is met.

  Arguments     :  srmnd - ptr to the SRMND CB                  
                   rsrc  - ptr to the resource monitor record

  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  
******************************************************************************/
uns32 srmnd_check_cpu_ldavg_thresholds(SRMND_CB *srmnd, SRMND_RSRC_MON_NODE *rsrc)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Get the system CPU load avg stats */
   if ((rc = srmnd_get_cpu_ldavg_stats(rsrc)) != NCSCC_RC_SUCCESS)
      return rc;

   /* Check whether the rsrc val meets the requested test condition.. if yes
      notify the subscr application */
   switch(rsrc->mon_data.monitor_data.monitor_type)
   {
   case NCS_SRMSV_MON_TYPE_THRESHOLD:
       rc = srmnd_check_thresholds_and_notify(srmnd, 
                                             rsrc,
                                             &rsrc->mon_data.result_value);
       break;

   case NCS_SRMSV_MON_TYPE_WATERMARK:
       srmnd_update_watermark_vals(rsrc);
       break;

   case NCS_SRMSV_MON_TYPE_LEAKY_BUCKET:
       /* TBD */
       break;

   default:
       break;
   }

   return rc;
}


/****************************************************************************
  Name          :  srmnd_check_cpu_thresholds
 
  Description   :  This routine checks the CPU specific resource statistic
                   values meets the requested test-condition against the 
                   defined threshold value and it notifies the user-application
                   if the test-condition is met.

  Arguments     :  srmnd - ptr to the SRMND CB                  
                   rsrc  - ptr to the resource monitor record

  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  
******************************************************************************/
uns32 srmnd_check_cpu_thresholds(SRMND_CB *srmnd, SRMND_RSRC_MON_NODE *rsrc)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 samples = rsrc->mon_data.monitor_data.mon_cfg.threshold.samples;
   NCS_SRMSV_VALUE cpu_util;

   memset(&cpu_util, 0, sizeof(NCS_SRMSV_VALUE));

   if ((rc = srmnd_get_cpu_utilization_stats(rsrc)) != NCSCC_RC_SUCCESS)
      return rc;

   switch(rsrc->mon_data.monitor_data.monitor_type)
   {
   case NCS_SRMSV_MON_TYPE_THRESHOLD:
       /* Still samples need to be updated */
       if ((samples > 1) && (rsrc->mon_data.samples_updated < samples))
          return rc;

       /* Get an average of resource statistics samples */
       srmnd_avg_rsrc_sample_stats(&rsrc->mon_data, &cpu_util);   

       rc = srmnd_check_thresholds_and_notify(srmnd, rsrc, &cpu_util);
       break;

   case NCS_SRMSV_MON_TYPE_WATERMARK:
       srmnd_update_watermark_vals(rsrc);
       break;

   case NCS_SRMSV_MON_TYPE_LEAKY_BUCKET:
       /* TBD */
       break;
   
   default:
       break;
   }

   return rc;
}


/****************************************************************************
  Name          :  srmnd_check_processes_exists
 
  Description   :  This routine checks the whether the process (with PID)
                   exists in the system or not.

  Arguments     :  srmnd - ptr to the SRMND CB                  
                   rsrc  - ptr to the resource monitor record

  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  
******************************************************************************/
uns32 srmnd_check_processes_exists(SRMND_CB *srmnd,
                                   SRMND_RSRC_MON_NODE *rsrc,
                                   NCS_BOOL *rsrc_delete)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 pid = rsrc->rsrc_type_node->pid;
   SRMND_PID_DATA *pid_data = rsrc->descendant_pids;
   SRMND_PID_DATA *next_pid = NULL;
   NCS_BOOL is_thread = FALSE;

   /* First check for the main PID before checking its descendants */
   if (m_SRMSV_CHECK_PROCESS_EXIST(pid, &is_thread) != TRUE)
   {
      /* Update the PID into notify value */
      rsrc->notify_val.val.u_val32 = pid;

      /* Notify the application about the process exit */
      rc = srmnd_send_msg(srmnd, rsrc, SRMND_APPL_NOTIF_MSG);
      if (rc == NCSCC_RC_SUCCESS)
      {
         /* Now no more required to have process specific resource monitor data */
         *rsrc_delete = TRUE;
      }
   }
   else /* Check for the descendants now */
   while (pid_data)
   {
      next_pid = pid_data->next_pid;
      pid = pid_data->pid;
      if (m_SRMSV_CHECK_PROCESS_EXIST(pid, &is_thread) != TRUE)
      {
         /* Update the PID into notify value */
         rsrc->notify_val.val.u_val32 = pid;

         /* Notify the application about the process exit */
         rc = srmnd_send_msg(srmnd, rsrc, SRMND_APPL_NOTIF_MSG);
         if (rc == NCSCC_RC_SUCCESS)
         {
            /* Now no more required to have process specific data */
            srmnd_remove_pid_data(rsrc, 0, pid_data);
         }
      }
      
      /* Go for next child PID now */
      pid_data = next_pid;
   }

   return rc;  
}


/****************************************************************************
  Name          :  srmnd_check_process_memory
 
  Description   :  This routine checks the process-memory specific resource 
                   statistics values meets the requested test-condition against
                   the defined threshold value and it notifies the user-appl
                   if the test-condition is met.

  Arguments     :  srmnd - ptr to the SRMND CB                  
                   rsrc  - ptr to the resource monitor record
                   rsrc_delete - set to TRUE if the process exits

  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  
******************************************************************************/
uns32 srmnd_check_process_thresholds(SRMND_CB *srmnd,
                                     SRMND_RSRC_MON_NODE *rsrc,
                                     NCS_BOOL *rsrc_delete)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 pid = rsrc->rsrc_type_node->pid;
   NCS_SRMSV_VALUE avg_val;
   NCS_BOOL is_thread = FALSE;
   uns32 samples = 0;

   /* First check for the main PID before checking its descendants */
   if (m_SRMSV_CHECK_PROCESS_EXIST(pid, &is_thread) != TRUE)
   {
      /* Notify the application about the process exit */
      rc = srmnd_send_msg(srmnd, rsrc, SRMND_PROC_EXIT_MSG);
      if (rc == NCSCC_RC_SUCCESS)
      {
         /* Now no more required to have process specific resource monitor data */
         *rsrc_delete = TRUE;
      }

      return NCSCC_RC_FAILURE;
   }

   memset(&avg_val, 0, sizeof(NCS_SRMSV_VALUE));

   switch (rsrc->rsrc_type_node->rsrc_type)
   {
   case NCS_SRMSV_RSRC_PROC_MEM:  /* checking memory consumed by process */
       if ((rc = srmnd_get_proc_memory_util_stats(rsrc)) != NCSCC_RC_SUCCESS)
          return rc;
       break;

   case NCS_SRMSV_RSRC_PROC_CPU:  /* checking CPU consumed by process */
       if ((rc = srmnd_get_proc_cpu_util_stats(rsrc)) != NCSCC_RC_SUCCESS)
       {
          /* Not required to update further */ 
          if (rc == NCSCC_RC_CONTINUE)
             return NCSCC_RC_SUCCESS;

          return rc;
       }
       break;

   default:
       break;
   }

   switch(rsrc->mon_data.monitor_data.monitor_type)
   {
   case NCS_SRMSV_MON_TYPE_THRESHOLD:
       samples = rsrc->mon_data.monitor_data.mon_cfg.threshold.samples;
       /* Still samples need to update */
       if ((samples > 1) && (rsrc->mon_data.samples_updated < samples))
          return rc;

       /* Get an average of resource statistics samples */
       srmnd_avg_rsrc_sample_stats(&rsrc->mon_data, &avg_val);   

       rc = srmnd_check_thresholds_and_notify(srmnd, rsrc, &avg_val);
       break;

   case NCS_SRMSV_MON_TYPE_WATERMARK:
       /* Check the current rsrc value with the recorded rsrc 
          specific WM values and do the updations accordingly */
       srmnd_update_watermark_vals(rsrc);
       break;

   case NCS_SRMSV_MON_TYPE_LEAKY_BUCKET:
       /* TBD */
       break;

   default:
       break;
   }

   return rc;
}


/****************************************************************************
  Name          :  srmnd_check_rsrc_stats
 
  Description   :  This routine checks the statistics of a resource, whether
                   they meets monitor test-condition. If it meets.. SRMND
                   process notifies the interested subscribers.

  Arguments     :  srmnd - ptr to the SRMND CB                  
                   rsrc  - ptr to the resource monitor record

  Return Values :  NCSCC_RC_SUCCESS (or) NCSCC_RC_FAILURE
 
  Notes         :  
******************************************************************************/
uns32 srmnd_check_rsrc_stats(SRMND_CB *srmnd,
                             SRMND_RSRC_MON_NODE *rsrc,
                             NCS_BOOL *rsrc_delete)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   switch (rsrc->rsrc_type_node->rsrc_type)
   {
   case NCS_SRMSV_RSRC_CPU_UTIL:
   case NCS_SRMSV_RSRC_CPU_KERNEL:
   case NCS_SRMSV_RSRC_CPU_USER:
       rc = srmnd_check_cpu_thresholds(srmnd, rsrc);
       break;

   case NCS_SRMSV_RSRC_CPU_UTIL_ONE:
   case NCS_SRMSV_RSRC_CPU_UTIL_FIVE:
   case NCS_SRMSV_RSRC_CPU_UTIL_FIFTEEN:
       srmnd_check_cpu_ldavg_thresholds(srmnd, rsrc);
       break;

   /* Memory Specific Resources */
   case NCS_SRMSV_RSRC_MEM_PHYSICAL_USED:
   case NCS_SRMSV_RSRC_MEM_PHYSICAL_FREE:
   case NCS_SRMSV_RSRC_SWAP_SPACE_USED:
   case NCS_SRMSV_RSRC_SWAP_SPACE_FREE:
   case NCS_SRMSV_RSRC_USED_BUFFER_MEM:
       rc = srmnd_check_memory_thresholds(srmnd, rsrc);
       break;
   
   case NCS_SRMSV_RSRC_USED_CACHED_MEM:
       /* TBD */
       break;

   /* Process specific resources */
   case NCS_SRMSV_RSRC_PROC_EXIT:
       rc = srmnd_check_processes_exists(srmnd, rsrc, rsrc_delete);
       break;

   case NCS_SRMSV_RSRC_PROC_MEM:  /* checking memory consumed by process */
   case NCS_SRMSV_RSRC_PROC_CPU:  /* checking memory consumed by process */
       rc = srmnd_check_process_thresholds(srmnd, rsrc, rsrc_delete);
       break;

   default:
       break;
   }

   return rc;
}


