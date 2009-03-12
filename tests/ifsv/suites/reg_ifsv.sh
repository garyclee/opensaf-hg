#!/bin/bash

export TET_CASE=-1
export TET_LIST_NUMBER=-1
export TET_ITERATION=1
export EDS_ON=1

export MASTER_IFINDEX=2
export SLAVE_IFINDEX=3

export RED_FLAG=0
#source /opt/opensaf/tetware/lib_path.sh

 #xterm -T IFA_$TET_NODE_ID  -ls -sb -geometry 80x80 -e  gdb $TET_BASE_DIR/ifsv/ifsv_a_$TARGET_ARCH.exe
 $TET_BASE_DIR/ifsv/ifsv_a_$TARGET_ARCH.exe
