#!/bin/bash
export OPENSAF_HOME=/home/opensaf/
export OPEN_TWARE_HOME=/home/opensourcetware/
export RESULTS_DIR=/opt/tware_result

# Setup Cross Compile options 
#CC_EXEC_PREFIX=/opt/tools/ppc_74xx-
#export CC_LIB_DIR=/opt/tools/target/devkit/lib
#ppc-mvl-linux-gnu
export CC_EXEC_PREFIX=

#If TARGET_HOST is not set , will be picked from ../config.guess 
# autotools style host value to pickup opensaf libraries.

export TARGET_HOST=
#i386-mvl-linux-gnu #autotools style value

export LD=$CC_EXEC_PREFIX"ld" 

export CFLAGS="-g  -Wall"
export COMPILER=$CC_EXEC_PREFIX"gcc"


