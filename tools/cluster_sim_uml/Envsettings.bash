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
# The commands in this file sets up the environment for creating the UML
# environment.
#
# Edit to suit your environment. Default settings should work for most users.
#
# XERCES_PREFIX and NETSNMP_PREFIX can be set in the environment to point to an
# alternative installation directory.
# Set the variables to the directory above where the lib and bin directories are
# located. See the default setting below.

export OPENSAF_HOME=$PWD/../..
opensaf_version=$(grep 'Version:' ${OPENSAF_HOME}/rpms/opensaf_controller_rpm.spec | cut -d: -f2 | tr -d ' ')
opensaf_release=$(grep 'Release:' ${OPENSAF_HOME}/rpms/opensaf_controller_rpm.spec | cut -d: -f2 | tr -d ' ')

host_tuple=$(${OPENSAF_HOME}/config.guess)
arch=$(arch)

# the directory above lib please
xerces_prefix=${XERCES_PREFIX:-"/usr"}
export NETSNMP_PREFIX=${NETSNMP_PREFIX:-"/usr"}

export KVER=2.6.24.3
export KURL=${KURL:-"http://www.kernel.org/pub/linux/kernel/v2.6/linux-$KVER.tar.bz2"}
export BBVER=1.9.1
export BBURL=http://busybox.net/downloads/busybox-$BBVER.tar.bz2
export TIPCUTILSVER=1.0.4
export TIPCUTILSURL=http://prdownloads.sourceforge.net/tipc/tipcutils-$TIPCUTILSVER.tar.gz
export UMLUTILSVER=20070815
export UMLUTILSURL=http://user-mode-linux.sourceforge.net/uml_utilities_20070815.tar.bz2

# proxy settings
#export http_proxy=http://username:password@host:port/

########################################
# Do not edit
export OPENSAF_CONTROLLER_RPM=$OPENSAF_HOME/rpms/opensaf_controller-$opensaf_version-$opensaf_release.$arch.rpm
export OPENSAF_PAYLOAD_RPM=$OPENSAF_HOME/rpms/opensaf_payload-$opensaf_version-$opensaf_release.$arch.rpm
export OPENSAF_INCLUDE=$OPENSAF_HOME/include

case $arch in
    i686 ) export LIB_SUFFIX=lib ;;
    x86_64 ) export LIB_SUFFIX=lib64 ;;
    * )	echo "error: unknown architecture"
	exit 1 ;;
esac

export OPENSAF_LIB=$OPENSAF_HOME/targets/$host_tuple/$LIB_SUFFIX
export NETSNMP_LIB=$NETSNMP_PREFIX/$LIB_SUFFIX
export XERCES_LIB=$xerces_prefix/$LIB_SUFFIX

alias opensaf="cd $PWD; ./opensaf.sh"
alias uml_mconsole=$PWD/uml/bin/uml_mconsole

echo '"opensaf" - Manage the UML cluster'
echo '"uml_mconsole - Manage one UML kernel'
