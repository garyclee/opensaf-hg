#!/bin/bash
#      -*- OpenSAF  -*-
#
# (C) Copyright 2008 The OpenSAF Foundation
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
# under the GNU Lesser General Public License Version 2.1, February 1999.
# The complete license can be accessed from the following location:
# http://opensource.org/licenses/lgpl-license.php
# See the Copying file included with the OpenSAF distribution for full
# licensing terms.
#
# Author(s): Emerson Network Power
#

TARGET_ARCH=$1
RPM_TARGET_ARCH=$1
RPM_TYPE=$2

LIB_SUFFIX=$3
export LIB_SUFFIX

RPM_SPEC_FILE=rpms/opensaf_"$RPM_TYPE"_rpm.spec

if [ "$RPM_TYPE" = "controller" ]
then 
        echo "############################################"
	echo "Building RPM Package for Controller Blade"
        echo "############################################"

	TAR_NAME="opensaf_controller.tgz"
elif [ "$RPM_TYPE" = "payload" ]
then
        echo "############################################"
	echo "Building RPM Package for Payload Blade"
        echo "############################################"
	TAR_NAME="opensaf_payload.tgz"
elif [ "$RPM_TYPE" = "64bit_agent_libs" ]
then
        echo "###################################################"
	echo "Building RPM Package for 64 bit agent libraries"
        echo "###################################################"
	TAR_NAME="opensaf_64bit_agent_libs.tgz"
else
	echo "Error: Wrong input provided"
	echo "Usage: ./create_opensaf_rpm.sh [host-tuple] [controller/payload]"
	exit 0
fi

RPM_TARGET_ARCH=`echo $TARGET_ARCH | cut -d"-" -f1`

if [ "$RPM_TARGET_ARCH" = "powerpc" ]
then
	RPM_TARGET_ARCH=ppc
fi

if [ "$RPM_TYPE" = "64bit_agent_libs" ]
then
##################################################
######    Set 64 bit library dependencies        #
##################################################
export LIBDIR=$PWD/opt/opensaf/payload/$LIB_SUFFIX
mkdir -p $LIBDIR

echo " Copying all 64bit agent  libraries "
cd targets/$TARGET_ARCH/$LIB_SUFFIX/
for i in `ls lib*so*`
do
        #echo "cp -d $i $LIBDIR/$i"
        cp -d $i $LIBDIR/$i
done

if [ $? != 0 ]
then
   echo "Error: OpenSAF 64-bit Libraries not found, Did you do a make install?"
   cd -
   rm -rf opt etc
   exit
fi

cd -

tar -zcvf $TAR_NAME opt
rm -rf opt

echo "Generated tar $TAR_NAME"

mkdir -p var/tmp/payload/RPMS
mkdir -p var/tmp/payload/SOURCES
mkdir -p var/tmp/payload/BUILD
mkdir -p var/tmp/payload/SRPMS
mkdir -p var/opensaf/payload

mv $TAR_NAME var/tmp/payload/SOURCES/

# 64 bit libraries need to get populated in /opt/opensaf/payload
# Set the RPM_TYPE as payload.
RPM_TYPE=payload

else 
##################################################
##### Set controller/payload rpm dependencies    #
##################################################

export INCDIR=$PWD/opt/opensaf/include
export LIBDIR=$PWD/opt/opensaf/$RPM_TYPE/$LIB_SUFFIX
#export LIBDIR
export BINDIR=$PWD/opt/opensaf/$RPM_TYPE/bin
export SCRIPTSDIR=$PWD/opt/opensaf/$RPM_TYPE/scripts
export CONFDIR=$PWD/etc/opt/opensaf

mkdir -p $BINDIR
mkdir -p $INCDIR
mkdir -p $LIBDIR
mkdir -p $SCRIPTSDIR
mkdir -p $CONFDIR

echo "Copying all executable from ../targets/$TARGET_ARCH/$RPM_TYPE/"
cp targets/$TARGET_ARCH/$RPM_TYPE/* $BINDIR/

if [ $? != 0 ]
then
   echo "Error: OpenSAF Binaries not found, Did you do a make install?"
   rm -rf opt etc
   exit
fi


echo "Copying all public header files from ../include/"
cp include/* $INCDIR/

if [ $? != 0 ]
then
   echo "Error: OpenSAF public header files not available"
   rm -rf opt etc
   exit
fi


echo "Copying all libraries from ../targets/$TARGET_ARCH/$LIB_SUFFIX/"
cd targets/$TARGET_ARCH/$LIB_SUFFIX/
for i in `ls lib*so*`
do
	cp -d $i $LIBDIR/$i
done

if [ $? != 0 ]
then
   echo "Error: OpenSAF Libraries not found, Did you do a make install?"
   cd -
   rm -rf opt etc
   exit
fi
cd -

echo "Copying all scripts from  ../scripts/$RPM_TYPE"
cp scripts/$RPM_TYPE/* $SCRIPTSDIR
cp scripts/common/* $SCRIPTSDIR
cp tools/utilities/collect_logs_$RPM_TYPE.sh $SCRIPTSDIR/collect_logs.sh

if [ $? != 0 ]
then
   echo "Error: OpenSAF scripts not found. Please check the scripts directory."
   rm -rf opt etc
   exit
fi

echo "Copying all config files from ../config"
cp config/* $CONFDIR
cp config/.drbd_sync_state* $CONFDIR

if [ $? != 0 ]
then
   echo "Error: OpenSAF config files not found. Please check the config directory."
   rm -rf opt etc
   exit
fi

mv $CONFDIR/nodeinit.conf.$RPM_TYPE $CONFDIR/nodeinit.conf

tar -zcvf $TAR_NAME opt etc
rm -rf opt etc


echo "Generated tar $TAR_NAME"

mkdir -p var/tmp/$RPM_TYPE/RPMS
mkdir -p var/tmp/$RPM_TYPE/SOURCES
mkdir -p var/tmp/$RPM_TYPE/BUILD
mkdir -p var/tmp/$RPM_TYPE/SRPMS
mkdir -p var/opensaf/$RPM_TYPE

mv $TAR_NAME var/tmp/$RPM_TYPE/SOURCES/

fi ##### End, else (controller/payload related settings)  #########

if [ "$4" = "hw_mgmt" ] || [ "$5" = "hw_mgmt" ]
then
	sed '/hisv/s/#%/%/g' -i $RPM_SPEC_FILE
fi

if [ "$4" = "mot_atca" ] || [ "$5" = "mot_atca" ]
then
	sed '/lfm_avm_intf/s/#%/%/g' -i $RPM_SPEC_FILE
fi

echo "Creating RPM"

rpmbuild -bb  --target $RPM_TARGET_ARCH --define "LIB_SUFFIX $LIB_SUFFIX" --define "_topdir $PWD/var/tmp/$RPM_TYPE" --define "__spec_install_post :" --define "_unpackaged_files_terminate_build 0" --buildroot $PWD/var/opensaf/$RPM_TYPE $RPM_SPEC_FILE


#echo "Moving rpm from $PWD/var/tmp/$RPM_TYPE/RPMS/$RPM_TARGET_ARCH/  to  $PWD/rpm/"

if [ $? = 0 ]
then
mv var/tmp/$RPM_TYPE/RPMS/$RPM_TARGET_ARCH/*.rpm rpms/.
echo ""
echo "#######################################################"
echo "OpenSAF RPM creation successful..."
echo "The $2 RPM file is available in ./rpms directory"
echo "#######################################################"
else
rm -rf var
echo ""
echo "###########################################"
echo "OpenSAF $2 RPM creation failed..."
echo "The above errors have to be resolved."
echo "###########################################"
fi

rm -rf var

if [ "$4" = "hw_mgmt" ] || [ "$5" = "hw_mgmt" ]
then
	sed '/hisv/s/%/#%/g' -i $RPM_SPEC_FILE
fi

if [ "$4" = "mot_atca" ] || [ "$5" = "mot_atca" ]
then
sed '/lfm_avm_intf/s/%/#%/g' -i $RPM_SPEC_FILE
fi
