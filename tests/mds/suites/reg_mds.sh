#!/bin/bash

export TET_CASE=-1
export TET_ITERATION=1
export TET_DEST_SUITE=1
export TET_SERV_SUITE=1

export LEAP_THREADS_NON_RT=0
export SA_AMF_COMPONENT_NAME="safComp=CompT_VDS,safSu=SuT_NCS_SCXB,safNode=Node1_SCXB"
export ACTIVE_CB_IP=127.0.0.1

#source /opt/opensaf/tetware/lib_path.sh

 #xterm -T MDS_APIs  -geometry 185x65 -e gdb  $TET_BASE_DIR/mds/mds_sender_$TARGET_ARCH.exe
  $TET_BASE_DIR/mds/mds_sender_$TARGET_ARCH.exe

