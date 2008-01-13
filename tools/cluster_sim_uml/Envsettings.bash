# User defined, edit to suit your environment

OPENSAF_VERSION=1.0
OPENSAF_BUILD=4

export HOST_TUPLE=i686-redhat-linux-gnu
export ARCH=`echo $HOST_TUPLE | cut -d"-" -f1`
export OPENSAF_HOME=`pwd`/../..
export XERCES_LIB=/usr/local/lib
export NETSNMP_PREFIX=/usr

export KVER=2.6.18.6
export KURL=http://www.se.kernel.org/pub/linux/kernel/v2.6/linux-$KVER.tar.bz2
export BBVER=1.3.2
export BBURL=http://busybox.net/downloads/busybox-$BBVER.tar.bz2
export TIPCUTILSVER=1.0.4
export TIPCUTILSURL=http://prdownloads.sourceforge.net/tipc/tipcutils-$TIPCUTILSVER.tar.gz
export UMLUTILSVER=20040406
export UMLUTILSURL=http://prdownloads.sourceforge.net/user-mode-linux/uml_utilities_$UMLUTILSVER.tar.bz2

# proxy settings
#export http_proxy=http://username:password@host:port/

########################################
# Do not edit
export OPENSAF_CONTROLLER_RPM=$OPENSAF_HOME/rpms/opensaf_controller-$OPENSAF_VERSION-$OPENSAF_BUILD.$ARCH.rpm
export OPENSAF_PAYLOAD_RPM=$OPENSAF_HOME/rpms/opensaf_payload-$OPENSAF_VERSION-$OPENSAF_BUILD.$ARCH.rpm
export OPENSAF_INCLUDE=$OPENSAF_HOME/include
export OPENSAF_LIB=$OPENSAF_HOME/targets/$HOST_TUPLE/lib
alias opensaf="cd $PWD; ./opensaf.sh"
alias uml_mconsole=$PWD/uml/bin/uml_mconsole
echo '"opensaf" - Manage the UML cluster'
echo '"uml_mconsole - Manage one UML kernel'
