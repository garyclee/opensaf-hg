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

#ifndef BASE_CONDITION_VARIABLE_H_
#define BASE_CONDITION_VARIABLE_H_

#include <pthread.h>
#include <cerrno>
#include "base/macros.h"
#include "base/mutex.h"
#include "base/osaf_utility.h"
#include "base/time.h"

namespace base {

enum class CvStatus { kNoTimeout, kTimeout };

// A condition variable, with an API similar to std::condition_variable. The
// timed methods always use the monotonic clock (clock id CLOCK_MONOTONIC).
class ConditionVariable {
 public:
  ConditionVariable();
  ~ConditionVariable();
  void Wait(Lock* lock) {
    int result =
        pthread_cond_wait(&condition_variable_, lock->mutex()->native_handle());
    if (result != 0) osaf_abort(result);
  }
  template <typename Predicate>
  void Wait(Lock* lock, Predicate predicate) {
    while (!predicate()) Wait(lock);
  }
  CvStatus WaitUntil(Lock* lock, const struct timespec& absolute_time) {
    int result = pthread_cond_timedwait(
        &condition_variable_, lock->mutex()->native_handle(), &absolute_time);
    if (result == 0) {
      return CvStatus::kNoTimeout;
    } else if (result == ETIMEDOUT) {
      return CvStatus::kTimeout;
    } else {
      osaf_abort(result);
    }
  }
  template <typename Predicate>
  bool WaitUntil(Lock* lock, const struct timespec& absolute_time,
                 Predicate predicate) {
    while (!predicate()) {
      if (WaitUntil(lock, absolute_time) == CvStatus::kTimeout) {
        return predicate();
      }
    }
    return true;
  }
  CvStatus WaitFor(Lock* lock, const struct timespec& relative_time) {
    struct timespec absolute_time = base::ReadMonotonicClock() + relative_time;
    return WaitUntil(lock, absolute_time);
  }
  template <typename Predicate>
  bool WaitFor(Lock* lock, const struct timespec& relative_time,
               Predicate predicate) {
    struct timespec absolute_time = base::ReadMonotonicClock() + relative_time;
    return WaitUntil(lock, absolute_time, predicate);
  }
  void NotifyOne() {
    int result = pthread_cond_signal(&condition_variable_);
    if (result != 0) osaf_abort(result);
  }
  void NotifyAll() {
    int result = pthread_cond_broadcast(&condition_variable_);
    if (result != 0) osaf_abort(result);
  }

 private:
  pthread_cond_t condition_variable_;
  DELETE_COPY_AND_MOVE_OPERATORS(ConditionVariable);
};

}  // namespace base

#endif  // BASE_CONDITION_VARIABLE_H_
