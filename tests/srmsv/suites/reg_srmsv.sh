#!/bin/bash

export TET_SRMSV_TEST_CASE_NUM=-1
export TET_SRMSV_TEST_LIST=1
export TET_SRMSV_REM_NODE_ID=131343
export TET_SRMSV_NUM_ITER=1
export TET_SRMSV_NUM_SYS=1
export TET_SRMSV_PID=10 /*pid is needed to monitor for the process exit */

#source /opt/opensaf/tetware/lib_path.sh
#xterm -T SRMA_APP_$TET_NODE_ID -e /usr/bin/gdb $TET_BASE_DIR/srmsv/srmsv_$TARGET_ARCH.exe 
$TET_BASE_DIR/srmsv/srmsv_$TARGET_ARCH.exe 
