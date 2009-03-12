#!/bin/bash

TARGET_TYPE=$1
export TET_COMP_DIR=/opt/opensaf/tetware/
OPENSAF_DEV_TAR=/opt/opensaf/
TET_ROOT=/usr/local/tet
source setup.sh

if [ ":$TARGET_HOST" == ":" ]; then
HOST_ALIAS=`$SHELL ../config.guess`;
else
HOST_ALIAS=`$SHELL ../config.sub $TARGET_HOST`;
fi
export HOST_ALIAS

if [ $# -le 0 ]; then 
   echo "Invalid target please specify  <linux/linux-64>"
   exit 1;
fi

if [ $# -le 1 ]; then
   if [ "$1" != "linux" ] && [ "$1" != "linux-64" ]; then
      echo "Invalid target please specify  <linux/linux-64>"
      exit 1;
   fi
fi

if [ "$1" == "linux" ] || [ "$1" == "linux-64" ]; then
   if [ $# -ne 1 ]; then
      echo "Invalid target please specify  <linux/linux-64>"
      exit 1;
   fi
fi


if [ ! -f /usr/local/tet/bin/tcc ]; then
   echo "Tetware Not Installed";
   exit 1;
fi

if [ ! -d $OPENSAF_DEV_TAR ]; then
   echo "Dev Tar not installed";
   exit 1;
fi


case "$1" in
   linux)
      LIBDIR=$TARGET_LIB_PATH/lib;;
   linux-64)
      LIBDIR=$TARGET_LIB_PATH/lib64;;

   *)
      echo "Invalid target please specify <linux/linux-64>";
      exit 1;
      ;;
esac
export LIBDIR

cd $TET_COMP_DIR
echo "Cleaning the objs";
csh make_env.csh $TARGET_TYPE clean clean
#rm -rf $LIBDIR/libmaa_switch.so

#cd $TET_COMP_DIR/maa_switch
#if [ "$1" == "linux" ] || [ "$1" == "linux-64" ]; then
#   ./compile_maa_switch.sh $1
#   if [ $? == 0 ] && [ ! -f $TET_COMP_DIR/maa_switch/libmaa_switch_$1.so ]; then
#      echo "Error in compilation of maa-switch library";
#      exit 1;
#   fi
#    cp $TET_COMP_DIR/maa_switch/libmaa_switch_$1.so $LIBDIR/libmaa_switch.so
#    cp $TET_COMP_DIR/maa_switch/libmaa_switch_$1.so  $TET_COMP_DIR/maa_switch/libmaa_switch.so
#else
#   ./compile_maa_switch.sh $1 $2
#   if [ $? == 0 ] && [ ! -f $TET_COMP_DIR/maa_switch/libmaa_switch_$1_$2.so ]; then
#      echo "Error in compilation of maa-switch library";
#      exit 1;
#   fi
#   cp $TET_COMP_DIR/maa_switch/libmaa_switch_$1_$2.so $LIBDIR/libmaa_switch.so
#   cp $TET_COMP_DIR/maa_switch/libmaa_switch_$1_$2.so $TET_COMP_DIR/maa_switch/libmaa_switch.so
#fi


cd $TET_COMP_DIR

csh make_env.csh $TARGET_TYPE A edsv
csh make_env.csh $TARGET_TYPE A mqsv
csh make_env.csh $TARGET_TYPE A glsv
csh make_env.csh $TARGET_TYPE A cpsv
csh make_env.csh $TARGET_TYPE A ifsv
csh make_env.csh $TARGET_TYPE DRIVER ifsv
csh make_env.csh $TARGET_TYPE VIP ifsv
csh make_env.csh $TARGET_TYPE A srmsv
csh make_env.csh $TARGET_TYPE SNDR mds
csh make_env.csh $TARGET_TYPE RCVR mds
csh make_env.csh $TARGET_TYPE A mbcsv
csh make_env.csh $TARGET_TYPE A avsv

cd $TET_COMP_DIR/srmsv/src
./compile_exe.sh $TARGET_TYPE
cd -
