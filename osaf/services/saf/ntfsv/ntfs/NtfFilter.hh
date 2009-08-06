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

#ifndef FILTER_H
#define FILTER_H

#include "saNtf.h"

class NtfFilter{
public:

    NtfFilter(SaNtfNotificationTypeT filterType);
    virtual ~NtfFilter();
    void checkFilter();
    SaNtfNotificationTypeT type();

private:

    SaNtfNotificationTypeT filterType_;
};

#endif // FILTER_H
