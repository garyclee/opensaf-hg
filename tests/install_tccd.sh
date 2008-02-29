#!/bin/bash

# Script to automatically install the TCC daemon
#
# Some systems use XINETD and some INETD.  This
# script accommodates both                      
#
INETD_LINE="tcc            stream   tcp     nowait  tet     /usr/local/tet/bin/in.tccd" 
SERVICES_TEMP=/tmp/services.$$
HERE=`pwd`

make_xinetd_file () {

# default: on
# description: The telnet server serves telnet sessions; it uses \
#	unencrypted username/password pairs for authentication.
echo "service tcc "  >/etc/xinetd.d/tcc
echo "{" >>/etc/xinetd.d/tcc
echo "	disable		= no" >>/etc/xinetd.d/tcc
echo "	flags		= REUSE" >>/etc/xinetd.d/tcc
echo "	log_on_failure	+= USERID" >>/etc/xinetd.d/tcc
echo "	log_on_success	+= USERID PID" >>/etc/xinetd.d/tcc
echo "	log_type	= FILE /var/log/tccd.log" >>/etc/xinetd.d/tcc
echo "	protocol	= tcp" >>/etc/xinetd.d/tcc
echo "	server		= /usr/local/tet/bin/in.tccd" >>/etc/xinetd.d/tcc
echo "	socket_type	= stream        " >>/etc/xinetd.d/tcc
echo "	user		= tet" >>/etc/xinetd.d/tcc
echo "	wait		= no" >>/etc/xinetd.d/tcc
echo "}" >>/etc/xinetd.d/tcc
echo "" >>/etc/xinetd.d/tcc

}

ME=`/usr/bin/whoami`
if [ ${ME} != "root" ] 
then
    echo "Must run this script as root"
    echo "Exiting..."
    exit 1
fi

# This part is for XINETD systems
# Verify that xinetd directory exists
if [ -d /etc/xinetd.d ]
then
    echo "This system uses XINETD."
    echo "Making /etc/xinetd.d/tccd file"
    rm -f /etc/xinetd.d/tcc*
    make_xinetd_file
    echo "Installed daemon- "
    echo "Restarting the xinetd daemon"
    /etc/init.d/xinetd restart
    echo "xinetd daemon restarted"
else
# This part is for INETD systems
    echo "This system uses INETD."
    if [ ! -f /etc/inetd.conf ]
    then
        echo "Error- no inetd.conf file- nothing done"
        exit 1
    fi
    #Make temp copy without and current tccd
    cat /etc/inetd.conf | grep -v tccd >/tmp/inetd.conf
    echo  >>/tmp/inetd.conf
    echo "${INETD_LINE}" >>/tmp/inetd.conf
    cp /tmp/inetd.conf /etc/inetd.conf
    echo "Restarting the inetd daemon"
    /etc/init.d/inetd restart
    echo "inetd daemon restarted"
fi
    echo "Done setting up tccd in [x]inetd"


echo "Updating the /etc/services file"
sleep 2
cat /etc/services | grep -v tcc >${SERVICES_TEMP}
echo "tcc		3333/tcp   # TET war TCCD ">>${SERVICES_TEMP}
cp ${SERVICES_TEMP} /etc/services
rm -f ${SERVICES_TEMP}
echo "Done updating /etc/services file"












