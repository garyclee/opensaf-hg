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


# This file will only configure the TIPC in network mode and it is assumed that TIPC 
#       and tipc-config executable is present in the "/opt/TIPC" directory.     

# constants
MASK_LAST_3NIBBLES=4096
MASK_LAST_1NIBBLE=16
SHIFT4=4
NIDFIFO=/tmp/nodeinit.fifo
NID_MAGIC=AAB49DAA
NID_SVC_ID=TIPC
NID_TIPC_INSTALL_SUCCESS=1
NID_TIPC_LOAD_MODULE_ERR=2

MODULE=tipc
TIPC_D=/opt/TIPC
CHASSIS_ID_FILE=/etc/opt/opensaf/chassis_id
SLOT_ID_FILE=/etc/opt/opensaf/slot_id
# IMPORTANT : This script receives command in the form :
#/opt/opensaf/controller/scripts/ncs_tipc.sh start intf_name[eth0] core_id[1234] TIPC

# Get the Chassis Id and Slot Id from /etc/opt/opensaf/chassis_id and /etc/opt/opensaf/slot_id

if ! test -f $CHASSIS_ID_FILE; then
   echo "$CHASSIS_ID_FILE doesnt exists, exiting ...." 
   exit 0
fi
CHASSIS_ID=`cat $CHASSIS_ID_FILE`
if [ "$CHASSIS_ID" -gt "16" ] || [ "$CHASSIS_ID" -lt "1" ]  
    then 
      echo "CHASSIS ID Should be in the range of 1 to 16"
      echo "Quitting......"
      exit 0
fi

if ! test -f $SLOT_ID_FILE; then
   echo "$SLOT_ID_FILE doesnt exists, exiting ...." 
   exit 0
fi
SLOT_ID=`cat $SLOT_ID_FILE`
if [ "$SLOT_ID" -gt "16" ] || [ "$SLOT_ID" -lt "1" ]  
    then 
      echo "SLOT ID Should be in the range of 1 to 16"
      echo "Quitting......"
      exit 0
fi

if [ "$#" -lt "1" ] ||  [ "$#" -gt "4" ]
   then
     echo " If you want to use TIPC in Non Networking mode"
     echo " Usage: $0 start &"
     echo " If you want to use TIPC in Networking mode"
     echo " Usage: $0 start <ETH_Interface_nam > <number[1-9999]> &"
     echo " Example: if u want to set ethernet interface as eth0 and number as 1000"
     echo " $0 start eth0 1000 &"
      exit 0
fi

if ! test -d $TIPC_D ; then
   echo "/opt/TIPC Directory doesnt exists, exiting ...." 
   exit 0
fi

if ! test -f $TIPC_D/tipc-config  ; then
   echo "/opt/TIPC/tipc-config File doesnt exists, exiting ...." 
   exit 0
fi


if ! test -f $TIPC_D/tipc.ko  ; then
   echo "/opt/TIPC/tipc.ko File doesnt exists, exiting ...." 
   exit 0
fi

ETH_NAME=$2
CORE_ID=$3

cd /opt/TIPC/
chmod 777 *

if [ $# -gt 1 ] ; then 
    if [ "$CORE_ID" -gt "9999" ] || [ "$CORE_ID" -lt "1" ]  
      then 
        echo "CORE ID Should be in the range of 1 to 9999"
        echo "Quitting......"
        exit 0
    fi
fi    

lsmod | grep $MODULE
   if ! [ $? -eq 0 ] ; then
      echo "Inserting TIPC mdoule..."
      
      insmod tipc.ko
      
      ret_val=$?
      if [ $ret_val -ne 0 ] ; then 
         #echo "Unable to load the TIPC module, Please check the module format, Exitting...."
         echo " TIPC Module not insmoded " > /var/log/messages
         echo "$NID_MAGIC:$NID_SVC_ID:$NID_TIPC_LOAD_MODULE_ERR" > $NIDFIFO
         exit 1
         #exit 0
      fi

      ./tipc-config -max_nodes=2000 
      ret_val=$?
      if [ $ret_val -ne 0 ] ; then 
         echo "Unable to set the Max_nodes to 2000, Exitting ....."
         exit 0
      fi

   else
      echo "TIPC module already present , Please remove the TIPC module to configure and rerun the script again"
      echo "Quitting......"
      exit 0
   fi


printf "00%02x%02x0f\n" $CHASSIS_ID $SLOT_ID > /etc/opt/opensaf/node_id

NODE_ID_READ=`cat /etc/opt/opensaf/node_id`

VAL_MSB_NIBBLE=$((0x$NODE_ID_READ % $MASK_LAST_1NIBBLE)) 
VAL1=$((0x$NODE_ID_READ >> $SHIFT4))
VAL_3NIBBLES_AFTER_SHIFT4=$(($VAL1 % $MASK_LAST_3NIBBLES))

    if [ "$VAL_3NIBBLES_AFTER_SHIFT4" -gt "256" ]  
      then 
        echo "Node id or Physical Slot id out of range"
        echo "Quitting......"
        exit 0
    fi

TIPC_NODEID=$(($VAL_MSB_NIBBLE + $VAL_3NIBBLES_AFTER_SHIFT4))



if [ $# -eq 1 ] ; then 

   echo "Configuring TIPC in Non-Networking Mode..."
   ################ Address config and check #########
   ./tipc-config -a=1.1.$TIPC_NODEID 
   ret_z1=$?
   if [ $ret_z1 -ne 0 ] ; then 
       echo "Unable to Configure TIPC address, Please try again, exiting" 
       echo "Removing TIPC module ...."
       rmmod tipc
       exit 0
   fi
else
   echo "Configuring TIPC in Networking Mode..."
   ################ Address config and check #########
   ./tipc-config -netid=$CORE_ID -a=1.1.$TIPC_NODEID  
   ret_z2=$?
   if [ $ret_z2 -ne 0 ] ; then 
       echo "Unable to Configure TIPC address, Please try again, exiting" 
       echo "Removing TIPC module ...."
       rmmod tipc
       exit 0
   fi
   ################ Interface config and check #########
   ./tipc-config -be=eth:$ETH_NAME 
   ret_z3=$?
   if [ $ret_z3 -ne 0 ] ; then 
       echo "Unable to Configure TIPC bearer interface, Please try again, exiting" 
       echo "Removing TIPC module ...."
       rmmod tipc
       exit 0
   fi
fi   


echo " TIPC Module is Present and Configured Success in network mode " 
echo "$NID_MAGIC:$NID_SVC_ID:$NID_TIPC_INSTALL_SUCCESS" > $NIDFIFO

echo "Running Permanent loop to clean MDS Logs..."
while true
do
    for MDS_LOG_FILE in `ls /var/opt/opensaf/mdslog/ncsmds_n*.log 2>/dev/null`
    do
       #echo Checking $MDS_LOG_FILE 
       FILESIZE=`du -sk $MDS_LOG_FILE 2>/dev/null |cut -d"	" -f1`
       #echo  $FILESIZE
       if [ 5000 -lt 0$FILESIZE ]
       then 
           #echo  Moving
           rm -f $MDS_LOG_FILE.old 
           mv -f $MDS_LOG_FILE      $MDS_LOG_FILE.old 
       fi
    done
    sleep 15
done
      

