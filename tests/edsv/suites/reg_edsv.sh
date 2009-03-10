#!/bin/bash

export TET_CASE=-1
export TET_ITERATION=1
export TET_LIST_NUMBER=-1
export ENABLE_REDUNDANCY=0

export TET_CHANNEL_NAME=channel
export TET_PUBLISHER_NAME=Jack
export TET_EVENTDATA_SIZE=20
export TET_SUBSCRIPTION_ID=4

#source /opt/opensaf/tetware/lib_path.sh

 #xterm -T EDA_$TET_NODE_ID -geometry 102x82 -e gdb  $TET_BASE_DIR/edsv/eda_ut_$TARGET_ARCH.exe
 $TET_BASE_DIR/edsv/eda_ut_$TARGET_ARCH.exe
