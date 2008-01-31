#! /bin/bash

#      -*- OpenSAF  -*-
#
#  Copyright (c) 2007, Ericsson AB
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under High-Availability Operating 
# Environment Software License Version 1.4.
# Complete License can be accesseble from below location.
# http://www.opensaf.org/license 
# See the Copying file included with the OpenSAF distribution for
# full licensing terms.
#
# Author(s):
#   
#

TOP=`pwd`
bin=$TOP/bin
archive=$TOP/archive
scripts=$archive/scripts

die() {
    echo "ERROR: $*"
    exit 1
}

test -x ./bin/install.sh ||\
    die "This script must be executed in the [`pwd/..`] directory"

create_controller_root() {
    root=$TOP/root.controller
    opensafconfig=$root/etc/opt/opensaf/controller

    if ! test -d $root; then
        mkdir $root
        cd $root
	mkdir -p usr/lib
	cp -a $NETSNMP_PREFIX/lib/libnetsnmp*.so* usr/lib
	mkdir -p usr/sbin
	cp $NETSNMP_PREFIX/sbin/snmp* usr/sbin
	mkdir -p usr/bin
	cp $NETSNMP_PREFIX/bin/snmp* usr/bin
	mkdir -p usr/share/snmp/mibs
	cp $NETSNMP_PREFIX/share/snmp/mibs/* usr/share/snmp/mibs

    # Unpack opensaf rpm build result
        rpm2cpio $OPENSAF_CONTROLLER_RPM | cpio -dim

    # Do controller rpm spec post install
        mkdir -p $root/usr/share/snmp/mibs
	cp -a $OPENSAF_HOME/mibs/SAF* $root/usr/share/snmp/mibs
	cp -a $OPENSAF_HOME/mibs/NCS* $root/usr/share/snmp/mibs
	cp -a $OPENSAF_HOME/mibs/ECC-SMI $root/usr/share/snmp/mibs
        mkdir -p $root/usr/share/snmp/
        cp $root/etc/opt/opensaf/ncsSnmpSubagt.conf $root/usr/share/snmp/
        mkdir -p $root/var/opt/opensaf/mdslog
        mkdir -p $root/var/opt/opensaf/nidlog

    # Copy default config files
        cp -L $archive/NCSSystemBOM.xml $root/etc/opt/opensaf

        cd -

    # Required by e.g. the snmpd script
        mkdir -p $root/etc/snmp
        cp $archive/snmp*d.conf $root/etc/snmp
        mkdir -p $root/sbin
        cp /sbin/start-stop-daemon $root/sbin
        mkdir -p $root/etc/default

        mkdir -p $root/etc/init.d
        cp $archive/scripts/controller/snmpd $root/etc/init.d
        chmod +x $root/etc/init.d/snmpd

        cp $scripts/controller/*.rc $root/etc/init.d
        cp $scripts/controller/profile $root/etc/

    # Copy needed extra programs
        mkdir -p $root/bin
        cp /bin/bash $root/bin
        mkdir -p $root/lib

    # Get libraries that bash depends on
        cp /lib/libreadline.so.* $root/lib
        cp /lib/libhistory.so.* $root/lib
        cp /lib/libncurses.so.* $root/lib
        cp /lib/libdl.so.* $root/lib

    # Get libraries that opensaf depends on
        cp /lib/librt.so.* $root/lib
        cp /lib/libgcc_s.so.* $root/lib
        cp -a /usr/lib/libstdc++.* $root/lib

        mkdir -p $root/usr/bin
        mkdir -p $root/usr/share/snmp/mibs
        mkdir -p $root/var/opt/opensaf/stdouts

    # CLI unique group names
        cp $archive/group $root/etc

        cp -f $archive/rde* $root/etc/opt/opensaf/
        cp -a $XERCES_LIB/libxerces-c.so* $root/lib/

        $TOP/Hello/install.sh $root
        cp -d $TOP/Hello/AppConfig.xml $root/etc/opt/opensaf

        mkdir -p $root/opt/TIPC
        cp -L $TOP/tipcutils-$TIPCUTILSVER/tipc-config $root/opt/TIPC
        cp $TOP/uml/linux-$KVER/net/tipc/tipc.ko $root/opt/TIPC

    # Copy local timezone settings
        test -f /etc/localtime && cp /etc/localtime $root/etc

    else
        echo "Using existing [./root], remove it to enforce re-build"
    fi

    echo "Creating [$root/root.cpio] ..."
    $bin/mklndircpio $root $root/root.cpio /hostfs/root.controller

}

create_payload_root() {
    root=$TOP/root.payload
    opensafconfig=$root/etc/opt/opensaf/payload

    if ! test -d $root; then
        mkdir $root
        cd $root
        rpm2cpio $OPENSAF_PAYLOAD_RPM | cpio -dim
        cd -

    # rpm post install
        mkdir -p $root/var/opt/opensaf/mdslog
        mkdir -p $root/var/opt/opensaf/nidlog

        mkdir -p $root/etc/init.d
        cp $scripts/payload/*.rc $root/etc/init.d
        cp $scripts/payload/profile $root/etc/

    # Copy needed extra programs
        mkdir -p $root/bin
        cp /bin/bash $root/bin
        mkdir -p $root/lib

    # Get libraries that bash depends on
        cp /lib/libreadline.so.* $root/lib
        cp /lib/libhistory.so.* $root/lib
        cp /lib/libncurses.so.* $root/lib
        cp /lib/libdl.so.* $root/lib

    # Get libraries that opensaf depends on
        cp /lib/librt.so.* $root/lib
        cp /lib/libgcc_s.so.* $root/lib
        cp -a /usr/lib/libstdc++.* $root/lib

        mkdir -p $root/usr/bin
        mkdir -p $root/var/opt/opensaf/stdouts

        $TOP/Hello/install.sh $root

        mkdir -p $root/opt/TIPC
        cp -L $TOP/tipcutils-$TIPCUTILSVER/tipc-config $root/opt/TIPC
        cp $TOP/uml/linux-$KVER/net/tipc/tipc.ko $root/opt/TIPC

    # Copy local timezone settings
        test -f /etc/localtime && cp /etc/localtime $root/etc

    else
        echo "Using existing [./root], remove it to enforce re-build"
    fi

    echo "Creating [$root/root.cpio] ..."
    $bin/mklndircpio $root $root/root.cpio /hostfs/root.payload
}

case "$1" in
    controller)
	create_controller_root
	;;
    payload)
	create_payload_root
	;;
    *)
	create_controller_root
	create_payload_root
	;;
esac
exit 0
