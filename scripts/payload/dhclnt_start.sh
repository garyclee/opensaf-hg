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

PID_PATH="/tmp/pid/"
XTERM=""
CAT="cat"
ECHO="echo -e"
DBGFILE="/var/opt/opensaf/stdouts/monc_debug_log"
DHCLIENT="/sbin/dhclient -nw"
EXTN=".pid"
INTERFACES="bond0"

echo EXECUTING DHCLIENT START at `date` >> /dhc

compName=$SA_AMF_COMPONENT_NAME
if [ "$compName" = "" ]; then
    $ECHO "NO ENVIRONMENT VARIABLE OF DHCLNT COMPONENT NAME SET" >> $DBGFILE
    exit -1;
fi

# CHECK FOR THE EXISTENCE OF THE DHCLIENT COMPONENT
#pidval=`pidof [ /sbin/dhclient ]`
pidval=`ps -eaf|awk "{if (\\$8==\"/sbin/dhclient\" && \\$3==\"1\") print \\$2}"`;

# IN ABSENCE OF THE DHCLNTD COMPONENT, START IT
if [ "$pidval" = "" ]; then
   $ECHO "Executing dhclient-init-script...";
   $XTERM ${DHCLIENT} ${INTERFACES} &
   sleep 5
fi


# GET THE PID OF THE DHCLNTD COMPONENT
#pidval=`pidof [ /sbin/dhclient ]`
pidval=`ps -eaf|awk "{if (\\$8==\"/sbin/dhclient\") print \\$2}"`;

# IN ABSENCE OF THE MONITORING COMPONENT, EXIT WITH ERROR STATUS
if [ "$pidval" = "" ]; then
   $ECHO "UNABLE TO START THE DHCLNT COMPONENT....QUITTING...\n" >> $DBGFILE
        echo UNABLE TO COMPLETE EXECUTING DHCLIENT START at `date` >> /dhc
   exit -1;
fi

$CAT  /dev/null > $PID_PATH$compName$EXTN;
$ECHO $pidval > $PID_PATH$compName$EXTN;
echo COMPLETED EXECUTING DHCLIENT START at `date` >> /dhc
$ECHO "DHCLIENT START SUCCESS" >> $DBGFILE
exit 0

