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

#include "base/conf.h"
#include <ifaddrs.h>
#include <limits.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <fstream>
#include "base/logtrace.h"
#include "base/osaf_utility.h"
#include "osaf/configmake.h"

namespace base {

pthread_once_t Conf::once_control_ = PTHREAD_ONCE_INIT;
Conf* Conf::instance_ = nullptr;

Conf::Conf() :
    fully_qualified_domain_name_{},
    node_name_{},
    short_host_name_{},
    fully_qualified_domain_name_initialized_{false},
    node_name_initialized_{false},
    short_host_name_initialized_{false},
    mutex_{} {
}

Conf::~Conf() {
  Lock lock(instance_->mutex_);
  assert(instance_ == this);
  instance_ = nullptr;
}

void Conf::InitFullyQualifiedDomainName() {
  InitShortHostName();
  Conf* i = GetOrCreateInstance();
  Lock lock(i->mutex_);
  if (i->fully_qualified_domain_name_initialized_ == false) {
    i->fully_qualified_domain_name_ =
        GetFullyQualifiedDomainName(i->short_host_name_);
    i->fully_qualified_domain_name_initialized_ = true;
  }
}

void Conf::InitNodeName() {
  InitShortHostName();
  Conf* i = GetOrCreateInstance();
  Lock lock(i->mutex_);
  if (i->node_name_initialized_ == false) {
    i->node_name_ = GetNodeName(i->short_host_name_);
    i->node_name_initialized_ = true;
  }
}

void Conf::InitShortHostName() {
  Conf* i = GetOrCreateInstance();
  Lock lock(i->mutex_);
  if (i->short_host_name_initialized_ == false) {
    i->short_host_name_ = GetShortHostName();
    i->short_host_name_initialized_ = true;
  }
}

Conf* Conf::GetOrCreateInstance() {
  int result = pthread_once(&once_control_, PthreadOnceInitRoutine);
  if (result != 0) osaf_abort(result);
  return instance_;
}

void Conf::PthreadOnceInitRoutine() {
  assert(instance_ == nullptr);
  instance_ = new Conf();
  if (instance_ == nullptr) osaf_abort(0);
}

std::string Conf::GetFullyQualifiedDomainName(
    const std::string& short_host_name) {
  char host[NI_MAXHOST];
  std::string fqdn{short_host_name};
  ifaddrs* ifa;
  if (getifaddrs(&ifa) == 0) {
    for (ifaddrs* i = ifa; i != nullptr; i = i->ifa_next) {
      sockaddr* addr = i->ifa_addr;
      sockaddr_in6* addr_ipv6 = reinterpret_cast<sockaddr_in6*>(addr);
      if (addr != nullptr &&
          (addr->sa_family == AF_INET || addr->sa_family == AF_INET6) &&
          (i->ifa_flags & IFF_LOOPBACK) == 0 &&
          (i->ifa_flags & IFF_UP) != 0 &&
          (addr->sa_family != AF_INET6 ||
           (!IN6_IS_ADDR_LOOPBACK(&addr_ipv6->sin6_addr) &&
            !IN6_IS_ADDR_LINKLOCAL(&addr_ipv6->sin6_addr) &&
            !IN6_IS_ADDR_MC_LINKLOCAL(&addr_ipv6->sin6_addr)))) {
        int rc = getnameinfo(addr, addr->sa_family == AF_INET6 ?
                             sizeof(sockaddr_in6) : sizeof(sockaddr_in),
                             host, sizeof(host), nullptr, 0, 0);
        if (rc == 0) {
          TRACE("getnameinfo() successful, hostname='%s'", host);
          if (strncmp(short_host_name.c_str(), host, short_host_name.size()) == 0 &&
              host[short_host_name.size()] == '.') {
            fqdn = host;
            break;
          }
        } else {
          TRACE("getnameinfo() failed with error %d", rc);
        }
      }
    }
    freeifaddrs(ifa);
  } else {
    LOG_ER("getifaddrs() failed, errno=%d", errno);
  }
  return fqdn;
}

std::string Conf::GetNodeName(const std::string& short_host_name) {
  std::string node_name;
  std::ifstream str;
  str.width(255);
  try {
    str.open(PKGSYSCONFDIR "/node_name");
    str >> node_name;
  } catch (std::ifstream::failure) {
    node_name.clear();
  }
  return (str.fail() || node_name.empty()) ? short_host_name : node_name;
}

std::string Conf::GetShortHostName() {
  char short_host_name[HOST_NAME_MAX + 1];
  if (gethostname(short_host_name, sizeof(short_host_name)) == 0) {
    short_host_name[sizeof(short_host_name) - 1] = '\0';
  } else {
    LOG_ER("gethostname() failed, errno=%d", errno);
    short_host_name[0] = '\0';
  }
  return short_host_name;
}

}  // namespace base
