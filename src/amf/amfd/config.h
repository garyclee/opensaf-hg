/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
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
 * Author(s): Ericsson AB
 *
 */

#ifndef AMFD_CONFIG_H_
#define AMFD_CONFIG_H_

#include "osaf/saf/saAis.h"

class Configuration {
  public:
    Configuration();
    ~Configuration();
    SaAisErrorT get_config(void);
    bool restrict_auto_repair_enabled();
    void restrict_auto_repair(bool enable);
  private:
    bool restrict_auto_repair_;
    Configuration(const Configuration&) = delete;
    Configuration& operator=(const Configuration&) = delete;
};

extern Configuration *configuration;

#endif
