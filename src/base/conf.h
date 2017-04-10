/*      -*- OpenSAF  -*-
 *
 * Copyright Ericsson AB 2017 - All Rights Reserved.
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
 */

#ifndef BASE_CONF_H_
#define BASE_CONF_H_

#include <pthread.h>
#include <cassert>
#include <string>
#include "base/macros.h"
#include "base/mutex.h"

namespace base {

class Conf {
 public:
  // Read the fully qualified domain name and store a copy of it in this class.
  // Must be called at least once before the FullyQualifiedDomainName() method
  // is called. It is harmless to call this method multiple times (though it
  // should be avoided for performance reasons). This method is thread-safe.
  static void InitFullyQualifiedDomainName();

  // Read the configured OpenSAF node name and store a copy of it in this class.
  // Must be called at least once before the NodeName() method is called. It is
  // harmless to call this method multiple times (though it should be avoided
  // for performance reasons). This method is thread-safe.
  static void InitNodeName();

  // Read the short host name (the host name cut at the first dot) and store a
  // copy of it in this class.  Must be called at least once before the
  // ShortHostName() method is called. It is harmless to call this method
  // multiple times (though it should be avoided for performance reasons). This
  // method is thread-safe.
  static void InitShortHostName();

  // Returns the fully qualified domain name (FQDN) of this node. An empty
  // string is returned if the the FQDN could not be determined. Note: you must
  // call InitFullyQualifiedDomainName() at least once before calling this
  // method. This method is thread-safe.
  static const std::string& FullyQualifiedDomainName() {
    const Conf* i = instance();
    assert(i->fully_qualified_domain_name_initialized_);
    return i->fully_qualified_domain_name_;
  }

  // Returns the configured OpenSAF node name of this node. If no name has been
  // configured, or if the configuration could not be read, the short host name
  // will be returned instead. If the short host name could not be determined
  // either, an empty string is returned. Note: you must call InitNodeName() at
  // least once before calling this method. This method is thread-safe.
  static const std::string& NodeName() {
    const Conf* i = instance();
    assert(i->node_name_initialized_);
    return i->node_name_;
  }

  // Returns the short host name (the host name cut at the first dot) of this
  // node. If the short host name could not be determined, an empty string is
  // returned. Note: you must call InitShortHostName() at least once before
  // calling this method. This method is thread-safe.
  static const std::string& ShortHostName() {
    const Conf* i = instance();
    assert(i->short_host_name_initialized_);
    return i->short_host_name_;
  }

 private:
  Conf();
  ~Conf();
  static const Conf* instance() { return instance_; }
  static Conf* GetOrCreateInstance();
  static void PthreadOnceInitRoutine();
  static std::string GetFullyQualifiedDomainName(
      const std::string& short_host_name);
  static std::string GetFullyQualifiedDomainNameUsingDns(
      const std::string& short_host_name);
  static std::string GetNodeName(const std::string& short_host_name);
  static std::string GetShortHostName();
  static std::string ReadFile(const std::string& path_name,
                              std::string::size_type max_length,
                              const std::string& default_contents);
  static void WriteFileAtomically(const std::string& path_name,
                                  const std::string& contents);
  static pthread_once_t once_control_;
  static Conf* instance_;
  std::string fully_qualified_domain_name_;
  std::string node_name_;
  std::string short_host_name_;
  bool fully_qualified_domain_name_initialized_;
  bool node_name_initialized_;
  bool short_host_name_initialized_;
  Mutex mutex_;

  DELETE_COPY_AND_MOVE_OPERATORS(Conf);
};

}  // namespace base

#endif  // BASE_CONF_H_
