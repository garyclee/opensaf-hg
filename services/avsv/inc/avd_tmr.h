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

  This module is the include file for Availability Directors timer module.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVD_TMR_H
#define AVD_TMR_H

#define AVD_CFG_TMR_INTVL ((SaTimeT)30000000)

/* timer type enums */
typedef enum avd_tmr_type
{
   AVD_TMR_SND_HB,                    /* heart beat send timer */ 
   AVD_TMR_RCV_HB_D,                  /* heart beat receive from other
                                       * director timer */
   AVD_TMR_RCV_HB_ND,                 /* heart beat receive from Node
                                       * director timer */
   AVD_TMR_RCV_HB_INIT,               /* heart beat received from other
                                       * director during init phase timer */
   AVD_TMR_CL_INIT,                   /* This is the AvD initialisation 
                                       * timer after which AvD will assign
                                       * SIs to application SU. */

   AVD_TMR_CFG,                       /* Timer for receiving the cfg msgs */
   AVD_TMR_MAX 
} AVD_TMR_TYPE;


/* AVD Timer definition */
typedef struct avd_tmr_tag
{   
   tmr_t            tmr_id;   
   AVD_TMR_TYPE     type;
   uns32            cb_hdl;      /* cb hdl to retrieve the AvD cb ptr */
   SaClmNodeIdT     node_id;
   NCS_BOOL         is_active;
} AVD_TMR;

/* macro to start the heart beat timer. The cb and avnd structures
 * are the inputs.
 */
#define m_AVD_HB_TMR_START(cb,avnd) \
{\
   avnd->heartbeat_rcv_avnd.cb_hdl = cb->cb_handle; \
   avnd->heartbeat_rcv_avnd.is_active = FALSE; \
   avnd->heartbeat_rcv_avnd.node_id = avnd->node_info.nodeId; \
   avnd->heartbeat_rcv_avnd.type = AVD_TMR_RCV_HB_ND; \
   avd_start_tmr(cb,&(avnd->heartbeat_rcv_avnd),cb->rcv_hb_intvl); \
}

/* macro to start the cluster init timer. The cb structure
 * is the input.
 */
#define m_AVD_CLINIT_TMR_START(cb) \
{\
   cb->amf_init_tmr.cb_hdl = cb->cb_handle; \
   cb->amf_init_tmr.is_active = FALSE; \
   cb->amf_init_tmr.type = AVD_TMR_CL_INIT; \
   avd_start_tmr(cb,&(cb->amf_init_tmr),cb->amf_init_intvl); \
}

/* macro to start the Send Heart beat timer. The cb structure
 * is the input.
 */
#define m_AVD_SND_HB_TMR_START(cb) \
{\
   cb->heartbeat_send_avd.cb_hdl = cb->cb_handle; \
   cb->heartbeat_send_avd.is_active = FALSE; \
   cb->heartbeat_send_avd.type = AVD_TMR_SND_HB; \
   cb->heartbeat_send_avd.node_id = cb->node_id_avd_other; \
   avd_start_tmr(cb,&(cb->heartbeat_send_avd),cb->snd_hb_intvl); \
}

/* macro to start the Receive Heart beat timer. The cb structure
 * is the input.
 */
#define m_AVD_RCV_HB_TMR_START(cb) \
{\
   cb->heartbeat_rcv_avd.cb_hdl = cb->cb_handle; \
   cb->heartbeat_rcv_avd.is_active = FALSE; \
   cb->heartbeat_rcv_avd.type = AVD_TMR_RCV_HB_D; \
   cb->heartbeat_rcv_avd.node_id = cb->node_id_avd_other; \
   avd_start_tmr(cb,&(cb->heartbeat_rcv_avd),cb->rcv_hb_intvl); \
}


/*** Extern function declarations ***/

struct  cl_cb_tag;

EXTERN_C void avd_tmr_exp (void *);

EXTERN_C uns32 avd_start_tmr (struct  cl_cb_tag *, AVD_TMR *,  
                 SaTimeT);

EXTERN_C void avd_stop_tmr (struct  cl_cb_tag *, AVD_TMR *);



#endif
