/*      -*- OpenSAF  -*-
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

#include "base/condition_variable.h"

namespace base {

ConditionVariable::ConditionVariable() : condition_variable_{} {
  pthread_condattr_t attr;
  int result = pthread_condattr_init(&attr);
  if (result != 0) osaf_abort(result);
  result = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
  if (result != 0) osaf_abort(result);
  result = pthread_cond_init(&condition_variable_, &attr);
  if (result != 0) osaf_abort(result);
  result = pthread_condattr_destroy(&attr);
  if (result != 0) osaf_abort(result);
}

ConditionVariable::~ConditionVariable() {
  int result = pthread_cond_destroy(&condition_variable_);
  if (result != 0) osaf_abort(result);
}

}  // namespace base
