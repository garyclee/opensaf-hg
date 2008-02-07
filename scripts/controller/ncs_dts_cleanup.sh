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
PIDPATH=/var/run
PIDFILE=ncs_dts.pid

#XTERM=""
echo "Cleanup DTS..."

if test -f $PIDPATH/$PIDFILE
then
    l_pid=`cat $PIDPATH/$PIDFILE`
    echo $l_pid
fi

if [ "$l_pid" ]
then
    echo "AMF error code: $NCS_ENV_COMPONENT_ERROR_SRC"
   sleep 3
   if [ $NCS_ENV_COMPONENT_ERROR_SRC -ne 0 ]
   then
      `kill -5 $l_pid`
   else
      `kill -9 $l_pid`
   fi
else
   echo "DTS process already killed"
fi
#killall ncs_dts
rm -f $PIDPATH/$PIDFILE

exit 0
