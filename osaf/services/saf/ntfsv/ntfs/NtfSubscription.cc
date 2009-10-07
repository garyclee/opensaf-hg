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
 * Author(s): Ericsson AB
 *
 */

/**
 *   This object handles information about NTF subscriptions.
 */

#include "NtfSubscription.hh"
#include "logtrace.h"
#include "ntfsv_mem.h"

/**
 * This is the constructor.
 *
 * Initial variables are set and pointer to the filter object is stored.
 *
 * @param ntfsv_subscribe_req_t s 
 *           struct received from subscribe request.
 * @param subscriptionId
 *               Client-wide unique id of the subscription.
 * @param filter Pointer to the filter object.
 */
NtfSubscription::NtfSubscription(ntfsv_subscribe_req_t* s):subscriptionId_(s->subscriptionId),s_info_(*s)
{ 
	TRACE_2("Subscription %u created for client_id %u", subscriptionId_, s->client_id);
	if (s->f_rec.alarm_filter) {
		NtfFilter* filter = new NtfAlarmFilter(s->f_rec.alarm_filter);
		filterMap[filter->type()] = filter;
		TRACE_2("Filter type %#x p=%p, added to subscription %u, filterMap size is %u",
			filter->type(), filter, subscriptionId_, (unsigned int)filterMap.size());
	}
	if (s->f_rec.sec_al_filter) {
		NtfFilter* filter = new NtfSecurityAlarmFilter(s->f_rec.sec_al_filter);
		filterMap[filter->type()] = filter;
		TRACE_2("Filter type %#x added to subscription %u, filterMap size is %u",
			filter->type(), subscriptionId_, (unsigned int)filterMap.size());
	}
	if (s->f_rec.obj_cr_del_filter) {
		NtfFilter* filter = new NtfObjectCreateDeleteFilter(s->f_rec.obj_cr_del_filter);
		filterMap[filter->type()] = filter;
		TRACE_2("Filter type %#x added to subscription %u, filterMap size is %u",
			filter->type(), subscriptionId_, (unsigned int)filterMap.size());
	}
	if (s->f_rec.att_ch_filter) {
		NtfFilter* filter = new NtfAttributeChangeFilter(s->f_rec.att_ch_filter);
		filterMap[filter->type()] = filter;
		TRACE_2("Filter type %#x added to subscription %u, filterMap size is %u",
			filter->type(), subscriptionId_, (unsigned int)filterMap.size());
	}
	if (s->f_rec.sta_ch_filter) {
		NtfFilter* filter = new NtfStateChangeFilter(s->f_rec.sta_ch_filter);
		filterMap[filter->type()] = filter;
		TRACE_2("Filter type %#x added to subscription %u, filterMap size is %u",
			filter->type(), subscriptionId_, (unsigned int)filterMap.size());
	}	
}

/**
 * This is the destructor.
 *
 * The filter object is deleted.
 */
NtfSubscription::~NtfSubscription()
{
	TRACE_ENTER();
    TRACE_2("Subscription %u deleted", subscriptionId_);
	 // delete all filters
	 FilterMap::iterator posN = filterMap.begin();
	 while (posN != filterMap.end())
	 {
		  NtfFilter* filter = posN->second;  
		  TRACE_2("filter posN->first %#x filter_ptr=%p", posN->first, filter);
		  if (filter) {
			  TRACE_2("Filter of type %#x destroyed", (int)filter->type());
			  delete filter;
			  filterMap.erase(posN);  
		  }
		  posN++;
	 }
	 TRACE_LEAVE();
}
/**
 * This method is called to get the id of the subscription.
 *
 * @return Id of the subscription.
 */
SaNtfSubscriptionIdT NtfSubscription::getSubscriptionId() const
{
    return(subscriptionId_);
}
/**
 * This method is called to get the subscriptin info struct of 
 * the subscription. 
 *
 * @return pointer to the subscription info struct.
 */
ntfsv_subscribe_req_t* NtfSubscription::getSubscriptionInfo()
{
    return(&s_info_);
}

/**
 * This method is called to check if the subscription matches the received notification.
 *
 * The appropriate filter that matches the type of the received
 * notification is checked.
 *
 * @param notification
 *               Pointer to the received notifiaction object.
 *
 * @return true if the subscription matches the notification
 *         false if the subscription does not match the notification
 */
bool NtfSubscription::checkSubscription(const NtfNotification* notification)
{
	bool rv = false;
   
	FilterMap::iterator pos = filterMap.find(notification->getNotificationType());
	if (pos != filterMap.end()) {
		NtfFilter* filter = pos->second;
		assert(filter); 
		rv = filter->checkFilter(notification);
	}
	return(rv);
}

void NtfSubscription::printInfo()
{
    TRACE("Subscription information");
    TRACE("  subscriptionId %u", subscriptionId_);
}
