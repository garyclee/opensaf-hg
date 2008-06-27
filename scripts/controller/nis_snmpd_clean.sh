#!/bin/bash 
#
#      -*- OpenSAF  -*-
#
# (C) Copyright 2008 The OpenSAF Foundation
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
# under the GNU Lesser General Public License Version 2.1, February 1999.
# The complete license can be accessed from the following location:
# http://opensource.org/licenses/lgpl-license.php
# See the Copying file included with the OpenSAF distribution for full
# licensing terms.
#
# Author(s): Emerson Network Power
#
#

# get role from RDA.
# if role is active then call snmpd start script

ROLE_NAME=$1

if [ "$ROLE_NAME" = "ACTIVE" ]; then
   # Execute the componentization script of snmpd
   SNMP_HOME="/etc/init.d"
   echo "Executing snmpd-clean-script..."
   echo "Giving Command $SNMP_HOME/snmpd stop"
fi

