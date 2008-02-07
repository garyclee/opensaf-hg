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
PIDFILE=ncs_snmp_subagt.pid

if test -f $PIDPATH/$PIDFILE
then
   l_pid=`cat $PIDPATH/$PIDFILE`
   echo $l_pid
fi
if [ "$l_pid" ]
   then `kill -HUP $l_pid`
fi

exit 0

