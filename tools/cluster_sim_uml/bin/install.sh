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

top=`pwd`
bin=$top/bin
archive=$top/archive
scripts=$archive/scripts

die() {
    echo "ERROR: $*"
    rm -rf $root
    exit 1
}

test -x ./bin/install.sh ||\
    die "This script must be executed in the [`pwd/..`] directory"

# Check that we can find everything needed
test -e $top/tipcutils-$TIPCUTILSVER/tipc-config || die "cannot find tipc-config"
test -e $top/uml/linux-$KVER/net/tipc/tipc.ko || die "cannot find tipc kernel module"
test -e $OPENSAF_CONTROLLER_RPM || "cannot find rpm: $OPENSAF_CONTROLLER_RPM"
test -e $OPENSAF_PAYLOAD_RPM || "cannot find rpm: $OPENSAF_PAYLOAD_RPM"
test -e ${NETSNMP_LIB}/libnetsnmp.so.*.?.? || die "cannot find net-snmp libraries in: ${NETSNMP_LIB}"
test -e $XERCES_LIB/libxerces-c.so || die "Cannot find xerces libraries in: ${XERCES_LIB}"

create_controller_root()
{
    local root=$top/root.controller
    local opensafconfig=$root/etc/opt/opensaf/controller

    echo "Creating root file system for controller..."

    if ! test -d $root; then
        mkdir $root
        cd $root
	    mkdir -p usr/$LIB_SUFFIX
	    cp -a ${NETSNMP_LIB}/libnetsnmp*.so* usr/$LIB_SUFFIX
	    mkdir -p usr/sbin
	    cp $NETSNMP_PREFIX/sbin/snmp* usr/sbin
	    mkdir -p usr/bin
	    cp $NETSNMP_PREFIX/bin/snmp* usr/bin
	    mkdir -p usr/share/snmp/mibs
	    cp $NETSNMP_PREFIX/share/snmp/mibs/* usr/share/snmp/mibs

        # Get libraries that snmpd depends on
        mkdir -p $LIB_SUFFIX
        cp -L `ldd usr/sbin/snmpd | tr -s ' \t' '\n' | grep /$LIB_SUFFIX | \
            grep -v usr | sed -e 's,/tls,,'` $root/$LIB_SUFFIX
        cp -L `ldd usr/sbin/snmpd | tr -s ' \t' '\n' | grep /usr/$LIB_SUFFIX | \
            sed -e 's,/tls,,'` $root/usr/$LIB_SUFFIX
#        mkdir -p $root/usr/lib/perl5/5.8.8/x86_64-linux-thread-multi/CORE
#        cp /usr/lib/perl5/5.8.8/x86_64-linux-thread-multi/CORE/libperl.so \
#            $root/usr/lib/perl5/5.8.8/x86_64-linux-thread-multi/CORE

        # Unpack opensaf rpm build result
        rpm2cpio $OPENSAF_CONTROLLER_RPM | cpio -dim >& /dev/null
        rm -rf $root/opt/opensaf/include

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

        cd - >& /dev/null

        # Required by e.g. the snmpd script
        mkdir -p $root/etc/snmp
        cp $archive/snmp*d.conf $root/etc/snmp
        mkdir -p $root/sbin
        mkdir -p $root/etc/default
        
        mkdir -p $root/etc/init.d
        cp $archive/scripts/controller/snmpd $root/etc/init.d
        chmod +x $root/etc/init.d/snmpd
        
        cp $scripts/controller/*.rc $root/etc/init.d
        cp $scripts/controller/profile $root/etc/
        
        # Copy needed extra programs
        mkdir -p $root/bin
        cp /bin/bash $root/bin
        mkdir -p $root/$LIB_SUFFIX
        
        # Get libraries that bash depends on
        cp -L `ldd /bin/bash | tr -s ' \t' '\n' | grep /$LIB_SUFFIX | sed -e 's,/tls,,'` $root/$LIB_SUFFIX

        # Get libraries that opensaf depends on
        cp /$LIB_SUFFIX/librt.so.* $root/$LIB_SUFFIX
        cp /$LIB_SUFFIX/libgcc_s.so.* $root/$LIB_SUFFIX
        cp -a /usr/$LIB_SUFFIX/libstdc++.* $root/$LIB_SUFFIX

        mkdir -p $root/usr/bin
        mkdir -p $root/usr/share/snmp/mibs
        mkdir -p $root/var/opt/opensaf/stdouts

        # CLI unique group names
        cp $archive/group $root/etc

        cp -f $archive/rde* $root/etc/opt/opensaf/
        cp -a $XERCES_LIB/libxerces-c.so* $root/$LIB_SUFFIX

        # Get libraries that xerces depends on
        cp -L `ldd $XERCES_LIB/libxerces-c.so | tr -s ' \t' '\n' | grep /usr/$LIB_SUFFIX | sed -e 's,/tls,,'` $root/usr/$LIB_SUFFIX

        $top/Hello/install.sh $root
        cp -d $top/Hello/AppConfig.xml $root/etc/opt/opensaf

        mkdir -p $root/opt/TIPC
        cp $top/tipcutils-$TIPCUTILSVER/tipc-config $root/opt/TIPC
        cp $top/uml/linux-$KVER/net/tipc/tipc.ko $root/opt/TIPC

        # Copy local timezone settings
        test -f /etc/localtime && cp /etc/localtime $root/etc

    else
        echo "Using existing [./root], remove it to enforce re-build"
    fi

    echo "Creating [$root/root.cpio] ..."
    $bin/mklndircpio $root $root/root.cpio /hostfs/root.controller >& /dev/null

}

create_payload_root()
{
    local root=$top/root.payload
    local opensafconfig=$root/etc/opt/opensaf/payload

    echo "Creating root file system for payload..."

    if ! test -d $root; then
        mkdir $root
        cd $root
        rpm2cpio $OPENSAF_PAYLOAD_RPM | cpio -dim >& /dev/null
        rm -rf $root/opt/opensaf/include
        sed -e s/INITTIMEOUT=1000/INITTIMEOUT=10000/g $root/opt/opensaf/payload/scripts/nis_pld > \
            $root/opt/opensaf/payload/scripts/nis_pld.new
        mv $root/opt/opensaf/payload/scripts/nis_pld.new $root/opt/opensaf/payload/scripts/nis_pld
        chmod +x $root/opt/opensaf/payload/scripts/nis_pld
        cd - >& /dev/null

        # rpm post install
        mkdir -p $root/var/opt/opensaf/mdslog
        mkdir -p $root/var/opt/opensaf/nidlog

        mkdir -p $root/etc/init.d
        cp $scripts/payload/*.rc $root/etc/init.d
        cp $scripts/payload/profile $root/etc/

        # Copy needed extra programs
        mkdir -p $root/bin
        cp /bin/bash $root/bin
        mkdir -p $root/$LIB_SUFFIX

        # Get libraries that bash depends on
        cp -L `ldd /bin/bash | tr -s ' \t' '\n' | grep /$LIB_SUFFIX | sed -e 's,/tls,,'` $root/$LIB_SUFFIX

        # Get libraries that opensaf depends on
        cp /$LIB_SUFFIX/librt.so.* $root/$LIB_SUFFIX
        cp /$LIB_SUFFIX/libgcc_s.so.* $root/$LIB_SUFFIX
        cp -a /usr/$LIB_SUFFIX/libstdc++.* $root/$LIB_SUFFIX

        mkdir -p $root/usr/bin
        mkdir -p $root/var/opt/opensaf/stdouts

        $top/Hello/install.sh $root

        mkdir -p $root/opt/TIPC
        cp -L $top/tipcutils-$TIPCUTILSVER/tipc-config $root/opt/TIPC
        cp $top/uml/linux-$KVER/net/tipc/tipc.ko $root/opt/TIPC

        # Copy local timezone settings
        test -f /etc/localtime && cp /etc/localtime $root/etc

    else
        echo "Using existing [./root], remove it to enforce re-build"
    fi

    echo "Creating [$root/root.cpio] ..."
    $bin/mklndircpio $root $root/root.cpio /hostfs/root.payload >& /dev/null
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
