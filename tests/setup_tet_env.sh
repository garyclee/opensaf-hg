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


#
# edit_passwd_file
#
# Make tet user equivalent to root by changing uid/gid to 0.
# useradd created an entry something like this:
# tet:x:501:9005::/home/tet:/bin/bash
#
edit_passwd_file()
{
    /bin/ed /etc/passwd <<EOF > /dev/null 2>&1
/^tet:/
c
tet:x:0:0::/home/tet:/bin/bash
.
w
EOF
}


#
# create_tet_user_env
#
# Create /home/tet/.bashrc for tet user.
#
create_tet_user_env()
{
    BASHRC=/home/tet/.bashrc
    BASHRC_TMP=/tmp/.bashrc_$$
    # assuming the contents are correct if the file exists
    if [ -f ${BASHRC} ]; then
        cat ${BASHRC} |grep -v "^export PATH" |grep -v "^export TET_ROOT" |grep -v "^export TET_TMP_DIR" > ${BASHRC_TMP} 
        cp -f ${BASHRC_TMP} ${BASHRC}
        rm -f ${BASHRC_TMP}
    fi
        
    echo "export PATH=\$PATH:$TET_ROOT/bin" >> ${BASHRC}
    echo "export TET_ROOT=$TET_ROOT" >> ${BASHRC}
    echo "export TET_TMP_DIR=/tmp" >> ${BASHRC}

    source ${BASHRC}
}


#
# create_tet_user_groups
#
# Create the user and groups required for TET.
#
create_tet_user_groups () {

    # start clean; remove previous tet user/groups
    /usr/sbin/userdel tet 2>/dev/null
    /usr/sbin/groupdel testers 2>/dev/null
    /usr/sbin/groupdel tet 2>/dev/null
 
    # add groups/user
    /usr/sbin/groupadd testers
    /usr/sbin/groupadd tet
    /usr/sbin/useradd -g tet -G testers -p tet -d /home/tet tet

    # Now manually change uid/gid in /etc/passwd to 0 because
    # useradd doesn't allow creation of another entry with uid=0.
    # (make tet user equivalent to root)
    edit_passwd_file

    # create .bashrc for tet user
    create_tet_user_env
}


#
# create_tet_systems_files
#
# Takes a hostname as its only argument.
# See TETWARE src/tet3/systems and src/tet3/systems.equiv files for
# more information.
#
create_tet_systems_files()
{
   HNAME=$1

   # "000" is the local host
   echo "000 $HNAME" > $TET_ROOT/systems

   # $HNAME is permitted to log on to the tccd daemon
   echo "$HNAME" > $TET_ROOT/systems.equiv
}


#
# make_xinetd_file
#
# Create /etc/xinetd.d/tcc for starting up tccd.
#
make_xinetd_file ()
{
   echo "Making /etc/xinetd.d/tcc file"
   rm -f /etc/xinetd.d/tcc*

   # default: on
   # description: The telnet server serves telnet sessions;
   # it uses unencrypted username/password pairs for authentication.
    cat > /etc/xinetd.d/tcc <<EOF
service tcc
{
	disable		= no
	flags		= REUSE
	log_on_failure	+= USERID
	log_on_success	+= USERID PID
	log_type	= FILE /var/log/tccd.log
	protocol	= tcp
	server		= $TET_ROOT/bin/in.tccd
	socket_type	= stream
	user		= tet
	wait		= no
}

EOF
}


#
# update_etc_services
#
# Update /etc/services by first removing any
# pre-existing tcc entries, then adding the new tcc entry.
#
update_etc_services()
{
   SERVICES_TEMP=/tmp/services.$$
   TCC_LINE="tcc		3333/tcp   # TETware TCCD"

   echo "Updating the /etc/services file"
   # grab all lines that don't begin with tcc followed by space/tab
   cat /etc/services | grep -v "^tcc[ 	]" > ${SERVICES_TEMP}
   # add the new entry
   echo "$TCC_LINE" >> ${SERVICES_TEMP}
   # replace the existing services file
   # cp so as not to destroy symlink
   cp -f ${SERVICES_TEMP} /etc/services
   rm -f ${SERVICES_TEMP}
   echo "Done updating /etc/services file"
}


#
# update_inetd_conf
#
# Update /etc/inetd.conf by first removing any
# pre-existing tcc entries, then adding the new tcc entry.
#
update_inetd_conf()
{
   INETDCONF=/etc/inetd.conf
   INETDCONF_TEMP=/tmp/inetd_conf.$$
   INETD_LINE="tcc            stream   tcp     nowait  tet     $TET_ROOT/bin/in.tccd" 

   echo "Updating the $INETDCONF file"
   # grab all lines that don't begin with tcc followed by space/tab
   cat $INETDCONF | grep -v "^tcc[ 	]" > $INETDCONF_TEMP
   # add the new entry
   echo "$INETD_LINE" >> $INETDCONF_TEMP
   # replace the existing services file
   # cp so as not to destroy symlink
   cp -f $INETDCONF_TEMP $INETDCONF
   rm -f $INETDCONF_TEMP
   echo "Done updating $INETDCONF file"
}


#
# configure_tccd
#
# Update /etc/services;
# create tet systems, systems.equiv files;
# configure [x]inetd to start the TCC daemon.
#
configure_tccd()
{
   create_tet_systems_files $HOSTNAME
   update_etc_services

   # configure Solaris system
   if [ "$OS" = "SunOS" ]; then
      # XXX to be implemented
      # configure tccd in Service Management Facility using
      # inetadm(1M); also see smf(5)
      return
   fi

   # configure Linux system
   # existence of xinetd directory indicates this system uses XINETD
   if [ -d /etc/xinetd.d ]; then
      echo "This system uses XINETD."
      make_xinetd_file
      echo "Restarting the xinetd daemon"
      /etc/init.d/xinetd restart
      echo "xinetd daemon restarted"
   else
      echo "This system uses INETD."
      update_inetd_conf
      echo "Restarting the inetd daemon"
      /etc/init.d/inetd restart
      echo "inetd daemon restarted"
   fi
   echo "Done setting up tccd in [x]inetd"
}


#
# CLEANHOSTS
#
# This perl command removes the legacy tetware config
# comments and IP addresses from /etc/hosts.
#================ Tetware automation cfg =============
#<Ip-addr>        <blade>" >> /tmp/setup
#=====================================================
# entries here...
#=====================================================
#
CLEANHOSTS='
use File::Copy;
$fileName = "/etc/hosts";
$outFileName = "/etc/hosts.new";
$startPattern = "#================ Tetware automation cfg =============";
$endPattern1  = "#<Ip-addr>";
$endPattern   = "#=====================================================";

open (INFILE, $fileName) or die "Cannot open $fileName: $!\n";
open (OUTFILE, ">$outFileName") or die "Cannot open $outFileName: $!\n";

while (<INFILE>) {
        next if (/$startPattern/ ... /$endPattern1/);
        next if (/$endPattern/  ... /$endPattern/);
        print  OUTFILE $_;
}
close (INFILE);
close (OUTFILE);

move ($outFileName, $fileName);
'
# the single quote above is the end of the perl command CLEANHOSTS


#
# update_etc_hosts
#
# Not sure what is really required here, but the previous
# version of this script removed the loopback lines from
# /etc/hosts, then appended the following:
# 127.0.0.1 HOSTNAME localhost
# XXX Problem: our /etc/hosts file says:
# XXX # Do not remove the following line, or various programs
# XXX # that require network functionality will fail.
# XXX 127.0.0.1               v20z-1330-5-int1 localhost.localdomain localhost
# XXX The CLEANHOSTS cmd will remove this line.
#
update_etc_hosts()
{
   LOOPBACK=127.0.0.1

   # remove legacy tet configuration from /etc/hosts
   perl -e "{$CLEANHOSTS}"

   # remove ^$LOOPBACK lines from /etc/hosts
   grep -v "^${LOOPBACK}" /etc/hosts > /tmp/hosts.$$
   cat >> /tmp/hosts.$$ <<EOF
#======= lines added for OpenSAF/tetware tests: =======
$LOOPBACK $HOSTNAME localhost
#======= end of lines added for OpenSAF/tetware tests =======
EOF

   # cp so as not to destroy symlink
   cp -f /tmp/hosts.$$ /etc/hosts
   rm -f /tmp/hosts.$$
}


configure_tetware()
{ 
   create_tet_user_groups
   configure_tccd
# temporarily commented-out until we know requirements
   update_etc_hosts
}


################################
# main program
################################
# This script requires root privilege; it modifies or creates:
#  /etc/passwd, /etc/group, /etc/shadow, /etc/gshadow
#  /etc/inetd.conf, /etc/xinetd.d/tcc, /etc/services
#  /home/tet/.bashrc, /home/tet/systems, /home/tet/systems.equiv
#  /etc/hosts

if [ $# != 1 ]; then
   echo "Usage: $0 TET_ROOT"
   echo "where TET_ROOT is the root of the Tetware files"
   exit 1
fi
TET_ROOT=$1

/usr/bin/id | grep '^uid=0(' >/dev/null 2>&1
if [ $? -ne 0 ]; then
   echo "must be root to run this script"
   exit 1
fi

# Prerequisite: Tetware is installed in $TET_ROOT.
# test for existence of directory/symlink/link
if [ ! -f $TET_ROOT  -a  ! -d $TET_ROOT ]; then
   echo "$TET_ROOT doesn't exist."
   echo "Install Tetware in $TET_ROOT, then re-run this script"
   exit 1
fi

if [ "$HOSTNAME" = "" ]; then
    HOSTNAME=`hostname`
fi

OS=`uname -s`

configure_tetware

if [ ! -f /usr/local/tet ]; then
    ln -s ${TET_ROOT} /usr/local/tet
fi 
