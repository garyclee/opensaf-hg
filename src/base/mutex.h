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

#ifndef BASE_MUTEX_H_
#define BASE_MUTEX_H_

#include <pthread.h>
#include <cerrno>
#include "base/macros.h"
#include "base/osaf_utility.h"

namespace base {

// A mutex, with an API compatible with std::mutex. When built with the
// preprocessor macro ENABLE_DEBUG defined, the mutex will be created with error
// checking enabled.
class Mutex {
 public:
  using NativeHandleType = pthread_mutex_t*;
  Mutex();
  ~Mutex();
  void Lock() {
    int result = pthread_mutex_lock(&mutex_);
    if (result != 0) osaf_abort(result);
  }
  bool TryLock() {
    int result = pthread_mutex_trylock(&mutex_);
    if (result == 0) {
      return true;
    } else if (result == EBUSY) {
      return false;
    } else {
      osaf_abort(result);
    }
  }
  void Unlock() {
    int result = pthread_mutex_unlock(&mutex_);
    if (result != 0) osaf_abort(result);
  }
  NativeHandleType native_handle() { return &mutex_; }

 private:
  pthread_mutex_t mutex_;
  DELETE_COPY_AND_MOVE_OPERATORS(Mutex);
};

// A lock manager, with an API similar to std::unique_lock, but simplified so
// that the mutex is always held during the lifetime of the lock object - i.e.
// this class enforces the RAAI idiom.
class Lock {
 public:
  explicit Lock(Mutex& mutex) : mutex_{&mutex} { mutex_->Lock(); }
  ~Lock() { mutex_->Unlock(); }
  void Swap(Lock& lock) {
    Mutex* tmp = mutex_;
    mutex_ = lock.mutex_;
    lock.mutex_ = tmp;
  }
  Mutex* mutex() const { return mutex_; }

 private:
  Mutex* mutex_;
  DELETE_COPY_AND_MOVE_OPERATORS(Lock);
};

}  // namespace base

#endif  // BASE_MUTEX_H_
