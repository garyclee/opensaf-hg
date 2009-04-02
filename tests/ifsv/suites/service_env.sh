#!/bin/bash

# set Node_id, Slot_id
DEC_ID=$(cat $OPENSAF_CONF/slot_id)
export TET_SLOT_ID=$DEC_ID
#export TET_NODE_ID=$DEC_ID
export TET_SHELF_ID=2

# VIP env parameters
export TET_CASE=-1
export TET_LIST_NUMBER=1
export TET_ITERATION=1
export DISPLAY=127.0.0.1:0.0

# vip_wait value 0 - getchar(); 1 - nowait; other - wait(no);
export VIP_WAIT=1
export SWITCHOVER=0

if [ "$SWITCHOVER" == "1" ] ; then
    export VIP_WAIT=120
fi

HOST=`hostname`

export VIP_APP_NAME1=PC_1
export VIP_APP_NAME2=PC_2
export VIP_IP1=${VIP_IP1:-192.168.6.1}
export VIP_CURRENT_IP1=10.70.141.100
export VIP_INTF1=${VIP_INTF1:-eth0}
export VIP_IP2=${VIP_IP2:-192.168.6.2}
export VIP_CURRENT_IP2=10.70.141.87
export VIP_INTF2=${VIP_INTF2:-eth1}



# IFSv env parameters 
export TET_CASE=-1
export TET_LIST_NUMBER=-1
export TET_ITERATION=1
export EDS_ON=1

export MASTER_IFINDEX=2
export SLAVE_IFINDEX=3

export RED_FLAG=0


# IFSv driver parameters
export TET_CASE=-1
export TET_LIST_NUMBER=-1
export TET_ITERATION=1
export EDS_ON=1

export RED_FLAG=0
