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
{
export LTSNMPD_FILE="snmp_agent"
#LTSNMPD_FILE=0
lcl_process="lt-snmpd";

ps -aux | while read LINE
do
if [ "`echo $LINE |awk -F" " '{print $12}' - `" = $lcl_process ] ; then
echo $LINE | awk -F" " '{print $2}' > $LTSNMPD_FILE
echo "found ..."
fi

done

}
