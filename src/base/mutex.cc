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

#include "base/mutex.h"
#include "osaf/config.h"

namespace base {

Mutex::Mutex() : mutex_{} {
  pthread_mutexattr_t attr;
  int result = pthread_mutexattr_init(&attr);
  if (result != 0) osaf_abort(result);
  result = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
  if (result != 0) osaf_abort(result);
#ifdef ENABLE_DEBUG
  result = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
  if (result != 0) osaf_abort(result);
  result = pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
  if (result != 0) osaf_abort(result);
#endif
  result = pthread_mutex_init(&mutex_, &attr);
  if (result != 0) osaf_abort(result);
  result = pthread_mutexattr_destroy(&attr);
  if (result != 0) osaf_abort(result);
}

Mutex::~Mutex() {
  int result = pthread_mutex_destroy(&mutex_);
  if (result != 0) osaf_abort(result);
}

}  // namespace base
