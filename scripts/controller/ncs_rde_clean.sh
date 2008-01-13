#!/bin/bash
#
#           -*- OpenSAF  -*-
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

#
# Role Distribution Entity AMF cleanup script
#

PIDPATH=/var/run
PIDFILE=rde.pid
DAEMON=ncs_rde
DESC="Role Distribution Entity Daemon"
BINPATH=/opt/opensaf/controller/bin


RDE_HA_COMP_NAMED_PIPE=/tmp/rde_ha_comp_named_pipe

#check if the PIDFILE exists and get pid
if test -f $PIDPATH/$PIDFILE
then
   l_pid=`cat $PIDPATH/$PIDFILE`
fi

#stop rde daemon
if [ "$l_pid" ]
   then 
      echo "Stopping $DESC: $BINPATH/$DAEMON";
       `kill -9 $l_pid`
      echo "rde_lfm daemon is stopped sucessfully! $l_pid"
   else 
      echo "rde_lfm  deamon is not running"
fi

#remove the RDE PID file
if test -f $PIDPATH/$PIDFILE
then
   rm -f $PIDPATH/$PIDFILE
fi

#Delete RDE_HA_COMP_NAMED_PIPE
if test -p $RDE_HA_COMP_NAMED_PIPE
then
   rm -f $RDE_HA_COMP_NAMED_PIPE
fi

exit 0

