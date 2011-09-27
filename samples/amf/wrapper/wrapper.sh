#!/bin/sh
#
#      -*- OpenSAF  -*-
#
# (C) Copyright 2011 The OpenSAF Foundation
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
# Author(s): Ericsson
#

name=$(basename $0)
progdir="/usr/local/sbin"
prog="wrapper"

if [ -z $SA_AMF_COMPONENT_NAME ]; then
	logger -st $name "not AMF component context"
	exit 199
fi

binary=$progdir/$prog

if ! [ -x $binary ]; then
	logger -st $name  "$binary not executable"
	exit 198
fi

# Source LSB functions library
. /lib/lsb/init-functions

piddir="/tmp"
pidfile="$piddir/${SA_AMF_COMPONENT_NAME}.pid"
export WRAPPERPIDFILE=$pidfile

RETVAL=0

instantiate()
{
	args="$*"
	start_daemon -p $pidfile $binary $args
	RETVAL=$?
	if [ $RETVAL -ne 0 ]; then
		logger -st $name "Starting $binary failed"
	fi
	return $RETVAL
}

cleanup()
{
	killproc -p $pidfile $binary
	RETVAL=$?
	if [ $RETVAL -ne 0 ]; then
		logger -st $name "killproc $binary failed"
	fi

	# Cleanup the "wrapped" component
	logger -st $name "Executing $STOPSCRIPT"
	$STOPSCRIPT
	RETVAL=$?
	if [ $RETVAL -ne 0 ]; then
		logger -st $name "Executing $STOPSCRIPT failed"
	fi

	return $RETVAL
}

status()
{
	pidofproc -p $pidfile $binary
	RETVAL=$?
	return $RETVAL
}

CMD=$1
case $CMD in
	instantiate)
		shift
		instantiate $*
		RETVAL=$?
		;;
	cleanup)
		cleanup $*
		RETVAL=$?
		;;
	status)
		status
		RETVAL=$?
		;;
	*)
		echo "Usage: $0 {instantiate|cleanup|status}"
		RETVAL=2
esac

exit $RETVAL

