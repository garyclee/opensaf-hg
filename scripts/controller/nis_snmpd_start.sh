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

# get role from RDA.
# if role is active then call snmpd start script
# how to get the comp name ..?

NIDFIFO=/tmp/nodeinit.fifo
NID_MAGIC=AAB49DAA
SERVICE_CODE=SNMPD

NID_SNMPD_SUCCESS=1
NID_SNMPD_CREATE_FAILED=2
NID_SNMPD_VIP_CFG_FILE_NOT_FOUND=3
NID_SNMPD_VIP_ADDR_NOT_FOUND=4
NID_SNMPD_VIP_INTF_NOT_FOUND=5

export SNMPCONFPATH=/usr/share/snmp
export MIBDIRS=/usr/share/snmp/mibs

ROLE_NAME=$1
echo  "Inside nis snmpd script"
echo "ROLE_NAME = $ROLE_NAME"
pidval=`pidof snmpd`
echo "pidval = $pidval"

case "$ROLE_NAME" in
  ACTIVE)

        >/tmp/IP_ADDRESS
        >/tmp/INTERFACE
        ECHO="echo -e"
        SNMP_HOME="/etc/init.d"
        IPCMD_HOME="/bin"
        ARPING_HOME="/usr/sbin/"
        DEF_CONFIG_FILE_PATH="/etc/opt/opensaf"
        CONFIG_FILE="vipsample.conf"
        SNMP_IP=0

        getINTF () {
                templateFile=$1
             #echo "Line received in getTime function .. " $1
             echo $1 > tmp.log
             INTF=`awk '
             BEGIN {
                FS=":";
                }
                /SNMP/ {
                        print $4;
                }' tmp.log`
              if [ $INTF ]; then
                 echo $INTF > /tmp/INTERFACE
              fi
        }
        getIP () {
                templateFile=$1
             #echo "Line received in getTime function .. " $1
             echo $1 > tmp.log
             IP_ADDR=`awk '
             BEGIN {
                FS=":";
                }
                /SNMP/ {
                        print $2;
                }' tmp.log`
              if [ $IP_ADDR ]; then
                 echo $IP_ADDR > /tmp/IP_ADDRESS
              fi
        }
        FILENAME=$DEF_CONFIG_FILE_PATH/$CONFIG_FILE
        if [ -f $FILENAME ]; then
               echo "GOT THE CONFIG FILE "
        else
               echo "DIDN'T GET THE CONFIG FILE"
               echo "$NID_MAGIC:$SERVICE_CODE:$NID_SNMPD_VIP_CFG_FILE_NOT_FOUND" > $NIDFIFO
               exit -1
        fi
                                                                                
        #retrieve the IP address from config file
        echo "READING SNMP IP FROM THE CONFIG FILE"
        cat $FILENAME | while read LINE
        do
           getIP "$LINE"
           getINTF "$LINE"
        done
        SNMP_IP=`cat /tmp/IP_ADDRESS`
        SNMP_INTF=`cat /tmp/INTERFACE`

        echo "SNMP_IP is : $SNMP_IP"
        echo "SNMP_INTERFACE is: $SNMP_INTF"
        if [ "$SNMP_IP" = "" ]; then
           echo "$NID_MAGIC:$SERVICE_CODE:$NID_SNMPD_VIP_ADDR_NOT_FOUND" > $NIDFIFO
           exit -1
        fi
        if [ "$SNMP_INTF" = "" ]; then
           echo "$NID_MAGIC:$SERVICE_CODE:$NID_SNMPD_VIP_INTF_NOT_FOUND" > $NIDFIFO
           exit -1
        fi



        echo "IP ADDR INSTALLED FROM CMD"
        $IPCMD_HOME/ip addr add $SNMP_IP/24 dev $SNMP_INTF
        $ARPING_HOME/arping -U -c 3 -I $SNMP_INTF $SNMP_IP
        echo "Executing snmpd-init-script..."
        echo "Giving Command $SNMP_HOME/snmpd restart"
        $SNMP_HOME/snmpd restart;
        echo "1" > /tmp/snmp_status
       ;;
       *)
         echo "snmp start script will not be executed by NIS in this state"
       echo "1" > /tmp/snmp_status
       ;;
esac


