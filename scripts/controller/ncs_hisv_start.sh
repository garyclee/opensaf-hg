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


#XTERM="xterm -e /usr/bin/gdb"    # Set this if debugging(etc...) required
XTERM=""

export HCD_HEALTH_KEY_DEFAULT_SET="F6C7"
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/opensaf/controller/lib

echo "Executing HISv-init-script...";
###  The following environment variables are useful when running in 
###  in SIMULATION mode, where multiple NODEs are simulated on the
###  on the same linux Desktop
if [ ":$NCS_STDOUTS_PATH" == ":" ]
then
    export NCS_STDOUTS_PATH=/var/opt/opensaf/stdouts
fi
echo "NCS_STDOUTS_PATH=$NCS_STDOUTS_PATH"
$XTERM /opt/opensaf/controller/bin/ncs_hisv  >> $NCS_STDOUTS_PATH/ncs_hisv.log 2>&1 &
exit 0


