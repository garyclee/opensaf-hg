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

#ifndef OPENSAF_OSAF_SERVICES_INFRASTRUCTURE_RDE_INCLUDE_ROLE_H_
#define OPENSAF_OSAF_SERVICES_INFRASTRUCTURE_RDE_INCLUDE_ROLE_H_

#include <time.h>
#include <stdint.h>
#include "rda_papi.h"
#include "mds_papi.h"
#include "base/macros.h"

namespace base {
class Process;
}

class Role {
  DELETE_COPY_AND_MOVE_OPERATORS(Role);
 public:
  explicit Role(NODE_ID own_node_id);
  void SetPeerState(PCS_RDA_ROLE node_role, NODE_ID node_id);
  timespec* Poll(timespec* ts);
  uint32_t SetRole(PCS_RDA_ROLE new_role);
  PCS_RDA_ROLE role() const;
  static const char* to_string(PCS_RDA_ROLE role);

 private:
  static const uint64_t kDefaultDiscoverPeerTimeout = 2000;
  static const uint64_t kDefaultPreActiveScriptTimeout = 5000;
  void ExecutePreActiveScript();
  void ResetElectionTimer();
  uint32_t UpdateMdsRegistration(PCS_RDA_ROLE new_role,
                                 PCS_RDA_ROLE old_role);

  PCS_RDA_ROLE role_;
  NODE_ID own_node_id_;
  base::Process* proc_;
  timespec election_end_time_;
  uint64_t discover_peer_timeout_;
  uint64_t pre_active_script_timeout_;
  static const char *const role_names_[];
  static const char *const pre_active_script_;
};

#endif /* OPENSAF_OSAF_SERVICES_INFRASTRUCTURE_RDE_INCLUDE_ROLE_H_ */

