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


/* ========================================================================
 *   INCLUDE FILES
 * ========================================================================
 */
#include "NtfReader.hh"
#include "NtfLogger.hh"
#include <iostream>
#include "logtrace.h"
#include "NtfNotification.hh"

/* ========================================================================
 *   DEFINITIONS
 * ========================================================================
 */

/* ========================================================================
 *   TYPE DEFINITIONS
 * ========================================================================
 */

/* ========================================================================
 *   DATA DECLARATIONS
 * ========================================================================
 */

/* ========================================================================
 *   FUNCTION PROTOTYPES
 * ========================================================================
 */


/**
 *   NtfReader object creates a copy of the current cashed
 *   notifications. The private forward iterator ffIter is set
 *   to first element. 
 *   @param ntfLogger
 *   @param readerId
 * 
 */
NtfReader::NtfReader(NtfLogger& ntfLogger, unsigned int readerId):
coll_(ntfLogger.coll_),
ffIter(coll_.begin()),
readerId_(readerId)
{
    TRACE_3("ntfLogger.coll_.size: %u", (unsigned int)ntfLogger.coll_.size());
}

/**
 *   NtfReader object creates a copy of the current cashed
 *   notifications. The private forward iterator ffIter is set
 *   to first element. 
 *   @param ntfLogger
 *   @param readerId
 *   @param searchCriteria
 *   @param f_rec - filter record
 * 
 */
NtfReader::NtfReader(NtfLogger& ntfLogger,
	unsigned int readerId,
	SaNtfSearchCriteriaT searchCriteria,
	ntfsv_filter_ptrs_t *f_rec):readerId_(readerId),searchCriteria_(searchCriteria)
{
    TRACE_3("New NtfReader with filter, ntfLogger.coll_.size: %u", (unsigned int)ntfLogger.coll_.size());
	 if (f_rec->alarm_filter) {
		 NtfFilter* filter = new NtfAlarmFilter(f_rec->alarm_filter);
		 filterMap[filter->type()] = filter;
		 TRACE_2("Filter type %#x p=%p, added to reader %u, filterMap size is %u",
			 filter->type(), filter, readerId_, (unsigned int)filterMap.size());
	 }
	 if (f_rec->sec_al_filter) {
		 NtfFilter* filter = new NtfSecurityAlarmFilter(f_rec->sec_al_filter);
		 filterMap[filter->type()] = filter;
		 TRACE_2("Filter type %#x added to reader %u, filterMap size is %u",
			 filter->type(), readerId_, (unsigned int)filterMap.size());
	 }
	 filterCacheList(ntfLogger);
	 sortCacheList();
	 setStartPoint();
}


void NtfReader::sortCacheList()
{
}

void NtfReader::setStartPoint()
{ /* TODO: set startpoint according to searchCriteria */
	ffIter = coll_.begin();	
}

/**
 * This method is called to check which notifications that 
 * matches the filtering criterias and save pointer to those 
 * notifications.
 *
 */
void NtfReader::filterCacheList(NtfLogger& ntfLogger)
{
//:coll_(ntfLogger.coll_),ffIter(coll_.begin()),
// 
// 	 
	readerNotificationListT::iterator rpos;
	for (rpos = ntfLogger.coll_.begin(); rpos != ntfLogger.coll_.end(); rpos++)
	{
		unsigned int nId;
		 
		NtfSmartPtr n (*rpos);
		bool rv = false;
		FilterMap::iterator pos = filterMap.find(n->getNotificationType());
		if (pos != filterMap.end()) {
			NtfFilter* filter = pos->second;
			assert(filter); 
			rv = filter->checkFilter(n);
		}
		if (rv){
			nId =  n->getNotificationId();
			coll_.push_back(n);
			TRACE_3("nId: %u added to readerList", nId);
		}
	} 
}

/** 
 *   Will return the notification at the current position of the
 *   iterator ffIter if search direction is
 *   SA_NTF_SEARCH_YOUNGER.
 *   @param searchDirection
 *   @param error - outparam tells if the operation succeded
 *   @return NtfNotification - an empty non initialized
 *           notification will be returned if error != SA_AIS_OK
 */
NtfSmartPtr
NtfReader::next(SaNtfSearchDirectionT direction,
                SaAisErrorT* error)
{
    TRACE_ENTER();

    *error = SA_AIS_ERR_NOT_EXIST;

    if (direction == SA_NTF_SEARCH_YOUNGER)
    {
        if (ffIter >= coll_.end())
        {
            *error = SA_AIS_ERR_NOT_EXIST;
            lastRead=true;
            TRACE_LEAVE();
            NtfSmartPtr notif;
            return notif;
        }
        NtfSmartPtr notif(*ffIter);
        ffIter++;
        *error = SA_AIS_OK;
        TRACE_LEAVE();
        return notif;
    }
    else  // SA_NTF_SEARCH_OLDER
    {
        readerNotReverseIterT rIter(ffIter);
        if (rIter == coll_.rend() || ++rIter >= coll_.rend())
        {
            *error = SA_AIS_ERR_NOT_EXIST;
            ffIter = rIter.base();
            TRACE_LEAVE();
            NtfSmartPtr notif;
            return notif;
        }
        NtfSmartPtr notif(*rIter);
        ffIter = rIter.base();
        *error = SA_AIS_OK;
        TRACE_LEAVE();
        return notif;
    }
}

unsigned int NtfReader::getId()
{
    return readerId_;
}

