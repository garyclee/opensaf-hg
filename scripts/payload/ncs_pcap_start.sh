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

#XTERM="xterm -T OpenSAF-PCAP -e "
#XTERM="xterm -T OpenSAF-PCAP -e /usr/bin/gdb "
# Set this if debugging required. If you start in gdb,
# NOTE: Ensure you pass $2 -> ROLE & $3 -> NID_SVC_ID as cmd. line arguments,
# which NID passes to this (ncs_pcap_start) script.
# i.e. from gdb Typically, the cmd would be  "run ROLE=1\2 NID_SVC_ID=21"

DAEMON=ncs_pcap
PIDPATH=/var/run
BINPATH=/opt/opensaf/payload/bin
PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:$BINPATH
NID_PCAP_DAEMON_STARTED=1
NID_PCAP_DAEMON_NOT_FND=2
NID_PCAP_DAEMON_START_FAILED=3


echo "Executing PCAP init-script..."

if [ ":$NCS_STDOUTS_PATH" == ":" ]
then
    export NCS_STDOUTS_PATH=/var/opt/opensaf/stdouts
fi
echo "NCS_STDOUTS_PATH=$NCS_STDOUTS_PATH"


#Function to start/spawn a service.
start()
{

    echo "Starting PCAP..."
    #Check if daemon is installed.
    if [ ! -x $BINPATH/$DAEMON ]; then
       echo -n -e "Unable to find daemon: $BINPATH/$DAEMON     \n"
       echo "$NID_MAGIC:$SERVICE_CODE:$NID_PCAP_DAEMON_NOT_FND" > $NIDFIFO
       exit 1
    fi
    killall ncs_pcap
    $XTERM /opt/opensaf/payload/bin/ncs_pcap $* >$NCS_STDOUTS_PATH/ncs_pcap.log 2>&1 &
    echo "Starting $DESC: $BINPATH/$DAEMON";
   
    if [ $? -ne 0 ] ; then
	echo -n -e "Failed to start $DAEMON.    \n"
	echo "$NID_MAGIC:$SERVICE_CODE:$NID_PCAP_DAEMON_START_FAILED" > $NIDFIFO
	exit 0
     else
         echo "Started $DESC: $BINPATH/$DAEMON";
     fi

} # End start()


case "$1" in
   start)
        shift
        start $*
        # Don't Report Status to NID. PCAP will notify NID.
	echo "."
        ;;
esac;

exit 0;
