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
# /opt/TIPC/tipc.ko
#

#Constants
MASK_LAST_3NIBBLES=4096
MASK_LAST_1NIBBLE=16
SHIFT4=4

echo " Executing TIPC Link reset script"

if [ "$#" -lt "1" ]
   then
     echo " Usage: $0 <Node_id>"
     exit 0
fi

NODE_ID_READ=0x$1

#echo "$NODE_ID_READ"
TIPC_NODEID=`/opt/TIPC/tipc-config -a|cut -d"." -f3|cut -d">" -f1`
SELF_TIPC_NODE_ID=1.1.$TIPC_NODEID
#echo "$SELF_TIPC_NODE_ID"

        VAL_MSB_NIBBLE=$(($NODE_ID_READ % $MASK_LAST_1NIBBLE))
        VAL1=$(($NODE_ID_READ >> $SHIFT4))
        VAL_3NIBBLES_AFTER_SHIFT4=$(($VAL1 % $MASK_LAST_3NIBBLES))

        if [ "$VAL_3NIBBLES_AFTER_SHIFT4" -gt "256" ]
          then
              echo "Node id or Physical Slot id out of range"
              echo "Quitting......"
              exit 0
        fi

        PEER_TIPC_NODEID=1.1.$(($VAL_MSB_NIBBLE + $VAL_3NIBBLES_AFTER_SHIFT4))
        #echo "$PEER_TIPC_NODEID"

        /opt/TIPC/tipc-config -lt=`/opt/TIPC/tipc-config -l | grep "$PEER_TIPC_NODEID" | cut -d: -f1-3`/500
