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

die() {
    echo "ERROR: $*"
    exit 1
}

test -n "$1" || die "No root dir argument specified"
root=$1
hellohome=$(dirname $0)

echo "building aishello"

make -C $hellohome/aishello

echo "installing aishello"

mkdir -p $root/opt/aishello/etc/
mkdir -p $root/opt/aishello/bin/

cp -d $hellohome/cstart $root/opt/aishello/etc/
cp -d $hellohome/aishello/aishello $root/opt/aishello/bin/

echo "aishello installed in $root/opt/aishello"

