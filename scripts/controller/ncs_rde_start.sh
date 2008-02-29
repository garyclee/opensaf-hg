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

#
# Role Distribution Entity AMF instantiate script
#
PATH=$PATH:/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:/opt/opensaf/controller/bin
PIDPATH=/var/run
PIDFILE=rde.pid
DAEMON=ncs_rde
DESC="Role Distribution Entity Daemon"
BINPATH=/opt/opensaf/controller/bin
RDE_SCRIPT=/opt/opensaf/controller/scripts/rde_script

#
#To start ncs_rde from gdb
#RDE_SCRIPT=xterm -e /usr/bin/gdb /opt/opensaf/controller/bin/ncs_rde
#

RDE_HA_COMP_NAMED_PIPE=/tmp/rde_ha_comp_named_pipe
RDE_HA_ENV_HEALTHCHECK_KEY="BAD8"

echo "Executing RDE HA init-script..."
echo "comp name = $SA_AMF_COMPONENT_NAME"
echo "healthcheck key = $RDE_HA_ENV_HEALTHCHECK_KEY"
if [ ":$NCS_STDOUTS_PATH" == ":" ]
then
    export NCS_STDOUTS_PATH=/var/opt/opensaf/stdouts
fi
echo "NCS_STDOUTS_PATH=$NCS_STDOUTS_PATH"

#
# Set environment variables
#
function rde_set_env_var
{

	# Check for environment variable for shelf number.
	#if [ "$SHELF_NUMBER" = "" ] ; then
        #        SHELF_NUMBER=`cat /etc/opt/opensaf/shelf_id`
        #        export SHELF_NUMBER
	#fi

	# Check for environment variable for slot number.
	if [ "$SLOT_NUMBER" = "" ] ; then
		SLOT_NUMBER=`cat /etc/opt/opensaf/slot_id`
                export SLOT_NUMBER
	fi

	# Check for environment variable for site number.
	if [ "$SITE_NUMBER" = "" ] ; then
		export SITE_NUMBER=0x0F
	fi

	# Check for environment variable for site number.
	if [ "$PAYLOAD_BLADES" = "" ] ; then
		export PAYLOAD_BLADES="3.$SITE_NUMBER 4.$SITE_NUMBER 5.$SITE_NUMBER 6.$SITE_NUMBER 7.$SITE_NUMBER 8.$SITE_NUMBER 9.$SITE_NUMBER 10.$SITE_NUMBER 11.$SITE_NUMBER 12.$SITE_NUMBER 13.$SITE_NUMBER 14.$SITE_NUMBER 15.$SITE_NUMBER 16.$SITE_NUMBER"
	fi
	
}


#
# Create the named pipe if not exist RDE_HA_COMP_NAMED_PIPE
#
if test -p $RDE_HA_COMP_NAMED_PIPE
then
   echo "$RDE_HA_COMP_NAMED_PIPE is already existing"
else
   mkfifo -m 0666 $RDE_HA_COMP_NAMED_PIPE # mode 0666
fi

#
# push the component name and HC key into the named pipe (running in the backgroud)
# Do not ouput tailing new line (-n) and enable escape sequences (-e)
#

#
# check for the PID availability
#
if test -f $PIDPATH/$PIDFILE
then
   l_pid=`cat $PIDPATH/$PIDFILE`
fi

if [ "$l_pid" ]
then 
   echo "ncs_rde daemon is already running"
   echo -n -e "$SA_AMF_COMPONENT_NAME|$RDE_HA_ENV_HEALTHCHECK_KEY" > $RDE_HA_COMP_NAMED_PIPE &
else 
   echo "Starting $DESC: $BINPATH/$DAEMON";		

#
# Set env
#
   rde_set_env_var
   /opt/opensaf/controller/bin/ncs_rde>$NCS_STDOUTS_PATH/ncs_rde.log 2>&1 &
   sleep 1s
   if test -f $PIDPATH/$PIDFILE
   then
      l_pid=`cat $PIDPATH/$PIDFILE`
   fi

   if [ "$l_pid" ]
   then 
      echo "ncs_rde daemon restarted successfully with PID $l_pid"
   echo -n -e "$SA_AMF_COMPONENT_NAME|$RDE_HA_ENV_HEALTHCHECK_KEY" > $RDE_HA_COMP_NAMED_PIPE &
   else
      echo "Sorry ncs_rde daemon start failed, startup script failed"
   fi
fi

exit 0
