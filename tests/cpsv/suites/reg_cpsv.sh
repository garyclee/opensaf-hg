#!/bin/bash

export TET_CPSV_INST_NUM=0  
export TET_CPSV_TEST_CASE_NUM=-1
export TET_CPSV_TEST_LIST=1
export TET_CPSV_NUM_ITER=1
export TET_CPSV_ALL_REPLICAS_CKPT=all_replicas_ckpt_name
export TET_CPSV_ACTIVE_REPLICA_CKPT=active_replica_ckpt_name
export TET_CPSV_WEAK_REPLICA_CKPT=weak_replica_ckpt_name
export TET_CPSV_COLLOCATED_CKPT=collocated_ckpt_name
export TET_CPSV_ASYNC_ALL_REPLICAS_CKPT=async_all_replicas_ckpt_name
export TET_CPSV_ASYNC_ACTIVE_REPLICA_CKPT=async_active_replica_ckpt_name
export TET_CPSV_ALL_COLLOCATED_CKPT=all_collocated_ckpt_name
export TET_CPSV_WEAK_COLLOCATED_CKPT=weak_collocated_ckpt_name
export TET_CPSV_MULTI_VECTOR_CKPT=multi_vector_ckpt_name
export TET_CPSV_ALL_REPLICAS_CKPT_L=all_replicas_lckpt_name
export TET_CPSV_ACTIVE_REPLICA_CKPT_L=active_replica_lckpt_name
export TET_CPSV_WEAK_REPLICA_CKPT_L=weak_replica_lckpt_name
export TET_CPSV_COLLOCATED_CKPT_L=collocated_lckpt_name


#source /opt/opensaf/tetware/lib_path.sh
#xterm -T CPA_APP_$TET_NODE_ID -e  $TET_BASE_DIR/cpsv/cpsv_app_$TARGET_ARCH.exe 
$TET_BASE_DIR/cpsv/cpsv_app_$TARGET_ARCH.exe 
