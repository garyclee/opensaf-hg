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


#XTERM="/usr/bin/X11/xterm -e /usr/bin/gdb"    # set this if debugging, etc are required
XTERM=""

EXE_HOME="/opt/opensaf/controller/bin"

export PSEUDO_DRBD_ENV_HEALTH_CHECK_KEY="A5A5"

echo "Executing Pseudo DRBD instantiate script..";

# The following environment variables are useful when running in simulation mode,
# where multiple nodes are simulated on the same Linux desktop
if [ ":$NCS_STDOUTS_PATH" == ":" ]
then
    export NCS_STDOUTS_PATH=/var/opt/opensaf/stdouts
fi
echo "NCS_STDOUTS_PATH=$NCS_STDOUTS_PATH"

$XTERM $EXE_HOME/ncs_pdrbd >> $NCS_STDOUTS_PATH/ncs_pdrbd.log 2>&1 &

exit 0
