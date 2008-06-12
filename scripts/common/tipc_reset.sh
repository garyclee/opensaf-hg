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

# Constants
MASK_LAST_3NIBBLES=4096
MASK_LAST_1NIBBLE=16
SHIFT4=4

# Make sure tipc-config is available, either in path or in default location
tipc_config=`which tipc-config 2> /dev/null`
test $? -ne 0 && tipc_config="/opt/TIPC/tipc-config"
if ! [ -x ${tipc_config} ]; then
    echo "error: tipc-config is not available"
    exit 1
fi

echo " Executing TIPC Link reset script"

if [ "$#" -lt "1" ]; then
    echo " Usage: $0 <Node_id>"
    exit 1
fi

NODE_ID_READ=0x$1
VAL_MSB_NIBBLE=$(($NODE_ID_READ % $MASK_LAST_1NIBBLE))
VAL1=$(($NODE_ID_READ >> $SHIFT4))
VAL_3NIBBLES_AFTER_SHIFT4=$(($VAL1 % $MASK_LAST_3NIBBLES))

if [ "$VAL_3NIBBLES_AFTER_SHIFT4" -gt "256" ]; then
    echo "Node id or Physical Slot id out of range"
    echo "Quitting......"
    exit 1
fi

PEER_TIPC_NODEID=1.1.$(($VAL_MSB_NIBBLE + $VAL_3NIBBLES_AFTER_SHIFT4))

${tipc_config} -lt=`${tipc_config} -l | grep "$PEER_TIPC_NODEID" | cut -d: -f1-3`/500

exit $?
