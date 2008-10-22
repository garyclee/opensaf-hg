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
#XTERM="xterm -T OpenSAF-DTS -e "

#XTERM="xterm -T OpenSAF-DTS -e /usr/bin/gdb "
# Set this if debugging required.

DAEMON=ncs_dts
PIDPATH=/var/run
BINPATH=/opt/opensaf/controller/bin
PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:$BINPATH
PIDFILE=ncs_dts.pid
COMPNAMEFILE=/var/opt/opensaf/ncs_dts_comp_name
NIDFIFO=/tmp/nodeinit.fifo
NID_MAGIC=AAB49DAA
SERVICE_CODE=DTSV

export DTS_AMF_HELATH_CHECK_KEY="A9FD64E12C12"
NID_DTS_DAEMON_STARTED=1
NID_DTS_DAEMON_NOT_FND=2
NID_DTS_DAEMON_START_FAILED=3


echo "Executing DTSv init-script..."

if [ ":$NCS_STDOUTS_PATH" == ":" ]
then
    export NCS_STDOUTS_PATH=/var/opt/opensaf/stdouts
fi
echo "NCS_STDOUTS_PATH=$NCS_STDOUTS_PATH"


#Function to start/spawn a service.
start()
{

    echo "Starting DTSv..."
    #Check if daemon is installed.
    if [ ! -x $BINPATH/$DAEMON ]; then
       echo -n -e "Unable to find daemon: $BINPATH/$DAEMON     \n"
       echo "$NID_MAGIC:$SERVICE_CODE:$NID_DTS_DAEMON_NOT_FND" > $NIDFIFO
       exit 1
    fi
    killall ncs_dts
    rm -f $PIDPATH/$PIDFILE
    $XTERM /opt/opensaf/controller/bin/ncs_dts $* >$NCS_STDOUTS_PATH/ncs_dts.log 2>&1 &
    echo "Starting $DESC: $BINPATH/$DAEMON";		
   
    if [ $? -ne 0 ] ; then
	echo -n -e "Failed to start $DAEMON.    \n"
	echo "$NID_MAGIC:$SERVICE_CODE:$NID_DTS_DAEMON_START_FAILED" > $NIDFIFO
	exit 0
     else
         echo "Started $DESC: $BINPATH/$DAEMON";		
     fi

} # End start()


case "$1" in
   start)
        shift
        start $*
        ;;
   "")
        # AMF would call with no arguments
	echo $SA_AMF_COMPONENT_NAME

	#remove the existing $COMPNAMEFILE 
	rm -f $COMPNAMEFILE 

	#echo the component name into a temporary text file
	echo $SA_AMF_COMPONENT_NAME > $COMPNAMEFILE

	# check for the PID availability
	if test -f $PIDPATH/$PIDFILE
	then
	   l_pid=`cat $PIDPATH/$PIDFILE`
	fi
      
	if [ "$l_pid" ]
   	then
	   echo "Sending the SIGUSR1 to ncs_dts ..."
	   `kill -USR1 $l_pid`
	else
            echo "Re-Starting DTSv"
		start ;
		sleep 10s
		#Check if the process is running and send a signal
		if test -f $PIDPATH/$PIDFILE
		then
			l_pid=`cat $PIDPATH/$PIDFILE`
		fi

		echo $l_pid
		if [ "$l_pid" ]
		then `kill -USR1 $l_pid`
        	fi
	fi
     exit 0 # Report to AMF.
     ;;
esac;

