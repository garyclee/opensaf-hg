
#export DISPLAY=127.0.0.1:0.0

export TET_BASE_DIR=/opt/opensaf/tetware/
export TET_TMP_DIR=/tmp
export TET_MDS_IF_INDEX=1
export BUILD_TYPE=$1

#Populate Node_id Slot_id
DEC_ID=`cat /etc/opensaf/slot_id`
PC=`uname -r|grep mvlcge`;

export TARGET_ARCH=$TARGET_HOST

#Populate LD_LIBRARY_PATH
if [ -d /opt/opensaf/controller ]
then
   if [ "$BUILD_TYPE" == "linux" ] ; then
   export LD_LIBRARY_PATH=/usr/local/lib
   else
   export LD_LIBRARY_PATH=/usr/local/lib64
   fi
else
   if [ "$BUILD_TYPE" == "linux" ] ; then
   export LD_LIBRARY_PATH=/usr/local/lib
   else
   export LD_LIBRARY_PATH=/usr/local/lib64
   fi
fi 
echo $LD_LIBRARY_PATH  >>/etc/ld.so.conf
/sbin/ldconfig
export TET_SLOT_ID=$DEC_ID;
export TET_NODE_ID=$DEC_ID;
export TET_CLUSTER_ID=6007
export TET_SHELF_ID=2
