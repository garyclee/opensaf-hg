#!/bin/bash
source ../setup.sh

if [ $# -le 0 ]; then
   echo "Invalid Input"
   echo "./compile_maa_switch.sh (linux/linux-64) ";
   exit 1;
fi

if [ $# -le 1 ]; then
   if [ "$1" != "linux" ] && [ "$1" != "linux-64" ]; then
      echo "Invalid Input"
      echo "./compile_maa_switch.sh (linux/linux-64)";
      exit 1;
   fi
fi

if [ "$1" == "linux" ] && [ "$1" == "linux-64" ]; then
   if [ $# -ne 1 ]; then
      echo "Invalid Input"
      echo "./compile_maa_switch.sh (linux/linux-64)";
      exit 1
   fi
fi


TOOLKITDIR=$OPENSAF_HOME
INCLUDEDIR=$OPENSAF_HOME/include

case "$1" in
   linux)
      LIBDIR=$OPENSAF_HOME/targets/$HOST_ALIAS/lib;
      MAA_SWITCH=maa-switch_1405.c;
      LIBMAA_SWITCH=libmaa_switch_$1.so;;

   linux-64)
      if [ ":$TARGET_HOST" == ":" ]; then
      if [ `uname -m` != x86_64 ]; then
         echo "Cannot compile ARCH - linux-64 from ARCH - `uname -m`";
         exit 1;
      fi
      fi
      LIBDIR=$OPENSAF_HOME/targets/$HOST_ALIAS/lib64;
      MAA_SWITCH=maa-switch_1405.c;
      LIBMAA_SWITCH=libmaa_switch_$1.so;;

   *)
      echo "Invalid target please specify (linux/linux-64)";
      exit 1;
      ;;
esac
echo "lib  path is $LIBDIR"
$COMPILER -fPIC $CFLAGS -g  -shared -o $LIBMAA_SWITCH -DNCS_SAF=1 -I$INCLUDEDIR -L$LIBDIR -lncs_core -lmaa -lpthread  -ldl -lm -lc -lrt $MAA_SWITCH

