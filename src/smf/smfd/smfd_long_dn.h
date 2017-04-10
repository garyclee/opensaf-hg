/*       OpenSAF
 *
 * (C) Copyright 2016 The OpenSAF Foundation
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

#ifndef SMF_SMFD_SMFD_LONG_DN_H_
#define SMF_SMFD_SMFD_LONG_DN_H_

#include "smf/smfd/SmfLongDnApplier.h"
#include "base/osaf_extended_name.h"

/* Store for SmfLongDnApplier. Must live as long as the process lives
 * See smfd_main.cc
 */
extern SmfLongDnApplier *SmfLongDnInfo;

/* This inline function replaces the maxDnLength variable in the
 * smfd_cb structure that was updated in some places in the code by
 * specifically reading the long dn setting in the IMM config object.
 * This function will instead return a value corresponding to
 * maxDnLength based on the same long dn setting but that is monitored
 * using an IMM applier
 * NOTE:
 * I there is no applier or applier is not working, SmfMaxDnLength will
 * return the length corresponding to that long dn is not allowed.
 */
static inline uint32_t GetSmfMaxDnLength(void) {
  // here is the only place where "kOsafMaxDnLength" constant is
  // directly used
  uint32_t maxDnLength_long = kOsafMaxDnLength;
  uint32_t maxDnLength_short = SA_MAX_UNEXTENDED_NAME_LENGTH - 1;

  if (SmfLongDnInfo == nullptr) {
    return maxDnLength_short;
  }

  if (SmfLongDnInfo->get_value() == true) {
    return maxDnLength_long;
  } else {
    return maxDnLength_short;
  }
}

#endif  // SMF_SMFD_SMFD_LONG_DN_H_
