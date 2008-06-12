#!/bin/bash

if [ $# -ne 1 ];
then 
 echo "USAGE: compile_exe.sh <linux/linux64>"
 exit 1;
fi

export HJ_HOME=/opt/opensaf/tetware/

case $1 in

"linux")
   $COMPILER $CFLAGS -m32 -DNCS_SRMA_EXE=1 -o $TET_COMP_DIR/srmsv/gcsrmsv_ex_i386.exe $TET_COMP_DIR/srmsv/src/srmsv_exe.c;
   ;;

"linux-64")
   if [ ":$TARGET_HOST" == ":" ]; then 
   if [ `uname -m` != x86_64 ]; then 
      exit 1;
   fi
   fi
   $COMPILER $CFLAGS -DNCS_SRMA_EXE=1 -o $TET_COMP_DIR/srmsv/gcsrmsv_ex_i386-64.exe $TET_COMP_DIR/srmsv/src/srmsv_exe.c;; 

*) echo "check your input";
   exit 1;;

esac
