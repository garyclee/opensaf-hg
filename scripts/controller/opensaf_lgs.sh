#!/bin/sh 
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
# Author(s): Ericsson AB
#

mkdir -p /var/opt/opensaf/saflog
command=opensaf_saflogd
lgsd=/opt/opensaf/controller/bin/$command
pidfile=/var/run/$command.pid
stdoutspath=${STDOUTSPATH:-"/var/opt/opensaf/stdouts"}
source /etc/opt/opensaf/saflog.conf
mkdir -p $LOG_SYSTEM_PATH_NAME
mkdir -p $LOG_NOTIFICATION_PATH_NAME
mkdir -p $LOG_ALARM_PATH_NAME

test -x $lgsd || exit 1

case $1 in
    start)
        export LOGSV_TRACE_PATHNAME=$stdoutspath/$command
        # uncomment next line if you want to enable tracing from start of log server
        #export LOGSV_TRACE_CATEGORIES=ALL
        $lgsd > $stdoutspath/$command 2>&1 &
        ;;
    clean)
        killall -9 $command
        rm -f $pidfile
        ;;
    dump)
        # Dump internal data structures to console
        kill -USR2 `cat $pidfile` 
        ;;
    *)
        echo "Usage: $0 {start|clean|dump}"
        exit 1
esac

exit 0
