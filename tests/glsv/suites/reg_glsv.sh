#!/bin/bash

# GLSV INPUTS
export TET_GLSV_INST_NUM=1
export TET_GLSV_TET_LIST_INDEX=1
export TET_GLSV_TEST_CASE_NUM=-1
export TET_GLSV_NUM_ITER=1
export TET_GLSV_RES_NAME1=
export TET_GLSV_RES_NAME2=
export TET_GLSV_RES_NAME3=

# GLSV REDUNDANCY INPUTS
export TET_GLSV_RED_FLAG=
export TET_GLSV_WAIT_TIME=

export GLA_NODE_ID_1=$TET_NODE_ID
export GLA_NODE_ID_2=
export GLA_NODE_ID_3=

#source /opt/opensaf/tetware/lib_path.sh

#xterm -T GLA_$TET_NODE_ID -e gdb $TET_BASE_DIR/glsv/glsv_a_$TARGET_ARCH.exe
$TET_BASE_DIR/glsv/glsv_a_$TARGET_ARCH.exe
