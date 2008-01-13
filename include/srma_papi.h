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

  MODULE NAME: SRMA_PAPI.H

$Header: 
..............................................................................

  DESCRIPTION: This file has the data structure definitions defined for SRMA 
               that need to be published to the external world to make use of
               them for SRMSv APIs. It also includes the definitions of SRMSv  
               API function prototypes 

******************************************************************************/

#ifndef SRMA_PAPI_H
#define SRMA_PAPI_H

#include "ncsgl_defs.h"
#include "ncs_lib.h"
#include "srmsv_papi.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* HANDLE definitions used for SRMSv service */
typedef unsigned int NCS_SRMSV_HANDLE;        /* Application specific SRMA hdl */
typedef unsigned int NCS_SRMSV_RSRC_HANDLE;   /* Resource specific SRMA hdl */
typedef unsigned int NCS_SRMSV_SUBSCR_HANDLE; /* Subscriber specific SRMA hdl */

#define   NCS_SRMSV_ERR  SaAisErrorT

typedef enum 
{
   SRMSV_CBK_NOTIF_RSRC_THRESHOLD,
   SRMSV_CBK_NOTIF_WATERMARK_VAL,
   SRMSV_CBK_NOTIF_WM_CFG_ALREADY_EXIST,
   SRMSV_CBK_NOTIF_RSRC_MON_EXPIRED,
   SRMSV_CBK_NOTIF_PROCESS_EXPIRED
} NCS_SRMSV_CBK_NOTIF_TYPE;


typedef struct ncs_srmsv_rsrc_cbk_info
{
   NCS_SRMSV_CBK_NOTIF_TYPE notif_type;
   uns32 rsrc_mon_hdl;
   uns32 node_num;

   union
   { 
      uns32 pid;
      NCS_SRMSV_RSRC_TYPE rsrc_type;
      NCS_SRMSV_VALUE     rsrc_value;
      SRMSV_WATERMARK_VAL watermarks;
   } notify;
} NCS_SRMSV_RSRC_CBK_INFO;

#define  NCS_SRMSV_RSRC_CBK_INFO_NULL  ((NCS_SRMSV_RSRC_CBK_INFO*)0)


/****************************************************************************  
 *                               SRMSv APIs                                 *
 ****************************************************************************/
/* Function pointer definition for Call Back function */
typedef void (*NCS_SRMSV_CALLBACK) (NCS_SRMSV_RSRC_CBK_INFO *rsrc_callback_info);

/* Following API will hold the functionality of creating & destroying the 
 * SRMSv agemt, User application executes this function to create SRM agent
 * and to destroy the existing SRM agent.
 */
uns32 ncs_srmsv_lib_req(NCS_LIB_REQ_INFO *req_info);

/* This is the first request that should be called before any other SRMSv 
 * service APIs. Through this API, application will register with SRMSv for
 * its services. On invocation of this API, SRMSv stores the callback function
 * and other request parameters provided by the application..  
 */
NCS_SRMSV_ERR ncs_srmsv_initialize(NCS_SRMSV_CALLBACK srmsv_callback,
                                   NCS_SRMSV_HANDLE   *srmsv_handle);

/* This function returns the operating system handle associated with the 
 * handle srmsv_handle. The invoking process can use this handle to detect
 * pending callbacks.
 */
NCS_SRMSV_ERR   ncs_srmsv_selection_object_get (NCS_SRMSV_HANDLE   srmsv_handle,
                                                SaSelectionObjectT *srmsv_sel_obj);

/* This function invokes, in the context of the calling thread, pending
 * callbacks for the handle srmsv_handle in a way that is specified by the
 * srmsv_dispatch_flags parameter.
 */
NCS_SRMSV_ERR   ncs_srmsv_dispatch (NCS_SRMSV_HANDLE srmsv_handle,
                                    SaDispatchFlagsT srmsv_dispatch_flags);

/* This request de-registers the application from SRMSv. After this call, 
 * application cannot avail any services of SRMSv. On getting this request, 
 * SRMSv will free the application specific registered information.
 */
NCS_SRMSV_ERR ncs_srmsv_finalize(NCS_SRMSV_HANDLE srmsv_hdl);

/* On getting this request, SRMSv restarts the monitoring process on all the
 * resources (that are in stop mode) requested by the application.
 */
NCS_SRMSV_ERR ncs_srmsv_restart_monitor(NCS_SRMSV_HANDLE srmsv_hdl);

/* On getting this request, SRMSv stops the monitoring process on all the
 * resources requested by an application.
 */
NCS_SRMSV_ERR ncs_srmsv_stop_monitor(NCS_SRMSV_HANDLE srmsv_hdl);


/* if *rsrc_mon_hdl == 0, This request configures the application requested 
 * resource monitor data with SRMSv. On getting this request, SRMSv starts 
 * monitoring process on the specified resources.
 */
/* if *rsrc_mon_hdl != 0, This request modifies the application requested 
 * resource monitor data. On getting the request, SRMSv restarts monitoring
 * process on the requested resources based on the modified data.
 */
NCS_SRMSV_ERR ncs_srmsv_start_rsrc_mon(NCS_SRMSV_HANDLE srmsv_hdl,
                                       NCS_SRMSV_MON_INFO *mon_info,
                                       NCS_SRMSV_RSRC_HANDLE *rsrc_mon_hdl);

/* This request deletes the application requested resources from SRMSv.
 * On getting this request, SRMSv stops monitoring process on the resources
 * requested by application and deletes the respective resource monitor data.
 */
NCS_SRMSV_ERR ncs_srmsv_stop_rsrc_mon(NCS_SRMSV_RSRC_HANDLE rsrc_mon_hdl);


/* Through this request application will subscribe to all the SRMSv threshold events
 * that are specific to a resource or resources. On getting this request, SRMSv 
 * sends the resource specific threshold events to the application.
 */
NCS_SRMSV_ERR ncs_srmsv_subscr_rsrc_mon(NCS_SRMSV_HANDLE srmsv_hdl,
                                        NCS_SRMSV_RSRC_INFO *i_rsrc_info,
                                        NCS_SRMSV_SUBSCR_HANDLE *subscr_handle);

/* Through this request application will unsubscribe to all the subscribed SRMSv 
 * threshold events of a resource or resources. On getting this request, SRMSv stops
 * sending the resource specific threshold events to the application. Through one
 * request, application can unsubscribe from the subscribed threshold events of 
 * multiple resources.
 */
NCS_SRMSV_ERR ncs_srmsv_unsubscr_rsrc_mon(NCS_SRMSV_SUBSCR_HANDLE subscr_handle);

/* API to read the local node's resource statisticz */
NCS_SRMSV_ERR ncs_srmsv_req_rsrc_statistics(NCS_SRMSV_RSRC_INFO *rsrc_key,
                                            NCS_SRMSV_VALUE *ret_val);

/* Through this request, application can get the watermark levels of a particular 
 * rsrc. Before calling this function application should have configured a particular 
 * rsrc to record its watermark levels(high/low/dual) through ncs_srmsv_start_rsrc_mon()
 * with monitor type as WATERMARK.
 */
NCS_SRMSV_ERR ncs_srmsv_get_watermark_val(NCS_SRMSV_HANDLE srmsv_hdl,
                                      NCS_SRMSV_RSRC_INFO *rsrc_info,
                                      NCS_SRMSV_WATERMARK_TYPE watermark_type);


#ifdef  __cplusplus
}
#endif

#endif /* SRMA_PAPI_H */
