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
 *   This object handles information about NTF filters.
 */

#include "NtfFilter.hh"
#include "logtrace.h"
#include "ntfsv_mem.h"

/**
 * This is the constructor.
 *
 * Initial variables are set.
 *
 * @param filterType Type of this filter object (i.e. alarm, object change etc.).
 */
NtfFilter::NtfFilter(SaNtfNotificationTypeT filterType)
{

    filterType_ = filterType;
    TRACE_1("Filter %d created", filterType_);
}

/**
 * This is the destructor.
 */
NtfFilter::~NtfFilter()
{
    TRACE_1("Filter %d destroyed", filterType_);
}

SaNtfNotificationTypeT NtfFilter::type()
{
    return filterType_;
}

/**
 *  
 * checkFilter - check if the notification matches the filter.
 * true returned the fiter matches and notification should be 
 * sent to the subscriber. 
 *
 * @param notif 
 * 	the notification to be sent 
 * @return 
 *    true if the filter matches and the notification should be
 *    sent to the subscriber.
 */
bool NtfFilter::checkFilter(const NtfNotification *notif)
{
	return (notif->getNotificationType() == filterType_);
}

NtfAlarmFilter::NtfAlarmFilter(SaNtfAlarmNotificationFilterT *f):NtfFilter(SA_NTF_TYPE_ALARM), filter_(f)
{ 
}

NtfAlarmFilter::~NtfAlarmFilter()
{
	TRACE_8("destructor p = %p", filter_);
	ntfsv_filter_alarm_free(filter_);
}

NtfSecurityAlarmFilter::NtfSecurityAlarmFilter(SaNtfSecurityAlarmNotificationFilterT *f):NtfFilter(SA_NTF_TYPE_SECURITY_ALARM), filter_(f)
{
}

NtfSecurityAlarmFilter::~NtfSecurityAlarmFilter()
{
	TRACE_8("destructor p = %p", filter_);
	ntfsv_filter_sec_alarm_free(filter_);
}

NtfObjectCreateDeleteFilter::NtfObjectCreateDeleteFilter(SaNtfObjectCreateDeleteNotificationFilterT *f):NtfFilter(SA_NTF_TYPE_OBJECT_CREATE_DELETE), filter_(f)
{	
}

NtfObjectCreateDeleteFilter::~NtfObjectCreateDeleteFilter()
{
	TRACE_8("destructor p = %p", filter_);
	ntfsv_filter_obj_cr_del_free(filter_);
}

NtfStateChangeFilter::NtfStateChangeFilter(SaNtfStateChangeNotificationFilterT *f):NtfFilter(SA_NTF_TYPE_STATE_CHANGE), filter_(f)
{
}

NtfStateChangeFilter::~NtfStateChangeFilter()
{
	TRACE_8("destructor p = %p", filter_);
	ntfsv_filter_state_ch_free(filter_);
}

NtfAttributeChangeFilter::NtfAttributeChangeFilter(SaNtfAttributeChangeNotificationFilterT *f):NtfFilter(SA_NTF_TYPE_ATTRIBUTE_CHANGE), filter_(f)
{
}

NtfAttributeChangeFilter::~NtfAttributeChangeFilter()
{
	TRACE_8("destructor p = %p", filter_);
	ntfsv_filter_attr_ch_free(filter_);
}


