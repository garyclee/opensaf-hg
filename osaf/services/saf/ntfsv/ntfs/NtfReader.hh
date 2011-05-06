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


#ifndef __NTF_READER_HH
#define __NTF_READER_HH

/* ========================================================================
 *   INCLUDE FILES
 * ========================================================================
 */
#include "NtfNotification.hh"
#include "NtfFilter.hh"
/* ========================================================================
 *   DEFINITIONS
 * ========================================================================
 */

/* ========================================================================
 *   TYPE DEFINITIONS
 * ========================================================================
 */
// typedef std::deque<ntfsv_send_not_req_t> readerNotificationListT;
// typedef std::deque<ntfsv_send_not_req_t>::iterator
//         readerNotificationListTIter;
// typedef std::deque<ntfsv_send_not_req_t>::reverse_iterator
//         readerNotReverseIterT;

typedef std::deque<NtfSmartPtr> readerNotificationListT;
typedef std::deque<NtfSmartPtr>::iterator readerNotificationListTIter;
typedef std::deque<NtfSmartPtr>::reverse_iterator readerNotReverseIterT;
/* ========================================================================
 *   DATA DECLARATIONS
 * ========================================================================
 */
class NtfLogger;

class NtfReader
{
public:
    NtfReader(NtfLogger& ntfLogger, unsigned int readerId);
	 NtfReader(NtfLogger& ntfLogger,
		 unsigned int readerId,
		 SaNtfSearchCriteriaT searchCriteria,
		 ntfsv_filter_ptrs_t *f_rec);
//    virtual ~NtfReader();
	 void filterCacheList(NtfLogger& ntfLogger);
	 void sortCacheList();
	 void setStartPoint();
    NtfSmartPtr next(SaNtfSearchDirectionT direction,
                                           SaAisErrorT* error);
    unsigned int getId();

private:
    readerNotificationListT coll_;
    readerNotificationListTIter ffIter;
    bool lastRead;
	 FilterMap filterMap;
    unsigned int readerId_;
	 SaNtfSearchCriteriaT searchCriteria_;
};

#endif // NTF_READER_HH

