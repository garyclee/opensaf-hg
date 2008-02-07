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

XTERM=""
OUT="/etc/opt/opensaf/avsv_demo"
COMP_PID_MAP_FILE="/etc/opt/opensaf/avsv_demo_comp_pid_map"
 
echo "Executing Component Instantiate Script"
echo "CompName: $SA_AMF_COMPONENT_NAME"

# Instantiate the component
exec $XTERM $OUT 2>&1  >> /var/opt/opensaf/stdouts/avsv_demo.log &

# Get the pid
PID=$!
echo $PID

# Store the CompName PID mapping (CompName:PID format)
echo "$SA_AMF_COMPONENT_NAME:$PID" >> $COMP_PID_MAP_FILE

exit 0
