#!/bin/bash

export TET_CASE=-1
export TET_LIST_NUMBER=1
export TET_ITERATION=1
export DISPLAY=127.0.0.1:0.0

#vip_wait value 0 - getchar(); 1 - nowait; other - wait(no);
export VIP_WAIT=1
export SWITCHOVER=0

if [ "$SWITCHOVER" == "1" ] ; then
    export VIP_WAIT=120
fi

#source /opt/opensaf/tetware/lib_path.sh

HOST=`hostname`

#if [ $TARGET_ARCH == i386 ] || [ $TARGET_ARCH == i386-64 ]; then
    export VIP_APP_NAME1=PC_1
    export VIP_APP_NAME2=PC_2
    export VIP_IP1=192.168.66.1
    export VIP_CURRENT_IP1=172.1.1.5
    export VIP_INTF1=eth1
    export VIP_IP2=192.168.66.2
    export VIP_CURRENT_IP2=172.1.1.6
    export VIP_INTF2=eth2
#fi

 #xterm -T IFA_$HOST -e gdb $TET_BASE_DIR/ifsv/ifsv_vip_$TARGET_ARCH.exe
 $TET_BASE_DIR/ifsv/ifsv_vip_$TARGET_ARCH.exe
