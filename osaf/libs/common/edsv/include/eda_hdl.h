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

  EDA handle database related definitions.
  
*******************************************************************************/

#ifndef EDA_HDL_H
#define EDA_HDL_H

#include "eda.h"

uns32 eda_hdl_cbk_dispatch(EDA_CB *, EDA_CLIENT_HDL_REC *, SaDispatchFlagsT);

EDA_CLIENT_HDL_REC *eda_hdl_rec_add(EDA_CB **eda_cb,
					     const SaEvtCallbacksT *reg_cbks, uns32 reg_id, SaVersionT version);

EDA_CHANNEL_HDL_REC *eda_channel_hdl_rec_add(EDA_CLIENT_HDL_REC **hdl_rec,
						      uns32 chan_id,
						      uns32 chan_open_id,
						      uns32 channel_open_flags, const SaNameT *channelName);

EDA_EVENT_HDL_REC *eda_event_hdl_rec_add(EDA_CHANNEL_HDL_REC **);

void eda_hdl_list_del(EDA_CLIENT_HDL_REC **);
uns32 eda_hdl_rec_del(EDA_CLIENT_HDL_REC **, EDA_CLIENT_HDL_REC *);
uns32 eda_channel_hdl_rec_del(EDA_CHANNEL_HDL_REC **, EDA_CHANNEL_HDL_REC *);
uns32 eda_del_subsc_rec(EDA_SUBSC_REC **list_head, EDA_SUBSC_REC *rm_node);
uns32 eda_event_hdl_rec_del(EDA_EVENT_HDL_REC **, EDA_EVENT_HDL_REC *);

int32 eda_find_mark_channel_to_ulink(EDA_CLIENT_HDL_REC *eda_hdl_rec, const SaNameT *channelName);

NCS_BOOL eda_validate_eda_client_hdl(EDA_CB *eda_cb, EDA_CLIENT_HDL_REC *find_hdl_rec);

EDA_SUBSC_REC *eda_find_subsc_by_subsc_id(EDA_CHANNEL_HDL_REC *, uns32 sub_id);

EDA_CLIENT_HDL_REC *eda_find_hdl_rec_by_regid(EDA_CB *eda_cb, uns32 reg_id);

EDA_CHANNEL_HDL_REC *eda_find_chan_hdl_rec_by_chan_id(EDA_CLIENT_HDL_REC *eda_hdl_rec,
							       uns32 chan_id, uns32 chan_open_id);

void eda_msg_destroy(EDSV_MSG *msg);

uns32 eda_extract_pattern_from_event(SaEvtEventPatternArrayT *from_pattern_array,
					      SaEvtEventPatternArrayT **to_pattern_array);

uns32 eda_allocate_and_extract_pattern_from_event(SaEvtEventPatternArrayT *from_pattern_array,
							   SaEvtEventPatternArrayT **to_pattern_array);

#endif   /* !EDA_HDL_H */
