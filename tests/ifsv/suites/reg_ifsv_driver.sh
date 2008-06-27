#!/bin/bash

export TET_CASE=-1
export TET_LIST_NUMBER=-1
export TET_ITERATION=1
export EDS_ON=1

export RED_FLAG=0
#source /opt/opensaf/tetware/lib_path.sh

 #xterm -T IFA-DRIVER_$TET_NODE_ID -ls -sb -geometry 80x80 -e gdb $TET_BASE_DIR/ifsv/ifsv_driver_$TARGET_ARCH.exe
 $TET_BASE_DIR/ifsv/ifsv_driver_$TARGET_ARCH.exe
 pkill ifsv_demo.out
 /opt/opensaf/bin/ifsv_demo.out 2&
