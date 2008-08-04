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


EXE_HOME="/opt/opensaf/controller/bin"
PID_PATH="/tmp/pid/"
XTERM=""
CAT="cat"
ECHO="echo -e"
DBGFILE="/var/opt/opensaf/stdouts/monc_debug_log"
EXTN=".pid"
OPENHPID_PID_FILE=/var/run/openhpid.pid

compName=$SA_AMF_COMPONENT_NAME
if [ "$compName" = "" ]; then
    $ECHO "No Environment Variable of OPENHPI Component name set" >> $DBGFILE
    exit -1;
fi

# CHECK FOR THE EXISTENCE OF THE OPENHPID COMPONENT
pidval=`cat $OPENHPID_PID_FILE`
echo "PID Value $pidval "
$ECHO "Executing openhpid-init-script...";
for (( i=0; i<=5; i++ ))
do
$XTERM $EXE_HOME/openhpi restart;
sleep 2;

# GET THE PID OF THE OPENHPID COMPONENT
if [ -f $OPENHPID_PID_FILE ]; then
   pidval=`cat $OPENHPID_PID_FILE`
   if [ "$pidval" = "" ]; then
      $ECHO "Unable to start the OPENHPI component....Retrying...\n"
   else
      $CAT  /dev/null > $PID_PATH$compName$EXTN;
      $ECHO $pidval > $PID_PATH$compName$EXTN;
      $ECHO "OPENHPID Successfully Started" >> $DBGFILE
      exit 0
   fi
else
   $ECHO "Unable to start the OPENHPI component....Retrying...\n"
fi
done

$ECHO "Unable to start the OPENHPID component..Quitting...... \n"
exit -1
