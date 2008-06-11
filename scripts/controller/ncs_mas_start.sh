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
#XTERM="xterm -T OpenSAF-MAS -e "

#XTERM="xterm -T OpenSAF-MAS -e /usr/bin/gdb "
# Set this if debugging required.

DAEMON=ncs_mas
PIDPATH=/var/run
BINPATH=/opt/opensaf/controller/bin
PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:$BINPATH
PIDFILE=ncs_mas.pid
COMPNAMEFILE=/var/opt/opensaf/ncs_mas_comp_name
NIDFIFO=/tmp/nodeinit.fifo
NID_MAGIC=AAB49DAA
SERVICE_CODE=MASV

NID_MAS_DAEMON_STARTED=1
NID_MAS_DAEMON_NOT_FND=2
NID_MAS_DAEMON_START_FAILED=3


echo "Executing MASv init-script..."

if [ ":$NCS_STDOUTS_PATH" == ":" ]
then
    export NCS_STDOUTS_PATH=/var/opt/opensaf/stdouts
fi
echo "NCS_STDOUTS_PATH=$NCS_STDOUTS_PATH"


#Function to start/spawn a service.
start()
{

    echo "Starting MASv..."
    #Check if daemon is installed.
    if [ ! -x $BINPATH/$DAEMON ]; then
       echo -n -e "Unable to find daemon: $BINPATH/$DAEMON     \n"
       echo "$NID_MAGIC:$SERVICE_CODE:$NID_MAS_DAEMON_NOT_FND" > $NIDFIFO
       exit 1
    fi
    killall ncs_mas
    $XTERM /opt/opensaf/controller/bin/ncs_mas >$NCS_STDOUTS_PATH/ncs_mas.log 2>&1 &
    echo "Starting $DESC: $BINPATH/$DAEMON";		
   
    if [ $? -ne 0 ] ; then
	echo -n -e "Failed to start $DAEMON.    \n"
	echo "$NID_MAGIC:$SERVICE_CODE:$NID_MAS_DAEMON_START_FAILED" > $NIDFIFO
	exit 0
     else
         echo "Started $DESC: $BINPATH/$DAEMON";		
     fi

} # End start()


case "$1" in
   start)
        start
        # Report Status to NID
        echo "$NID_MAGIC:$SERVICE_CODE:$NID_MAS_DAEMON_STARTED" > $NIDFIFO
	echo "."
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
	   echo "Sending the SIGUSR1 to ncs_mas ..."
	   `kill -USR1 $l_pid`
	else
            echo "Re-Starting MASv"
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

