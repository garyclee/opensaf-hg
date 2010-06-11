#! /bin/bash

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
# The parameters in this file are inputs to run_osaf_tests.sh.
# Supply a file like this for the -e option envfile, or
# omit it and use the default settings.

#
# Where OpenSAF is installed:
#
OPENSAF_ROOT=${PWD%%tests*}
OPENSAF_CONF=/etc/opensaf
OPENSAF_VAR=/var/lib/opensaf

#
# Where test results go:
#
OPENSAF_TET_RESULT=$PWD/results

#
# Where test log saved:
#
OPENSAF_TET_LOG=/tmp/tccdlog

#
# The architecture of the machine where the tests are run;
# used by the tests to find executables with names of the
# form "*_${TARGET_ARCH}.exe"
#
#TARGET_ARCH=

#
# Where tetware objects are installed:
#
TET_ROOT=/usr/local/tet

#
# Where opensaf tests are installed:
#
TET_BASE_DIR=$PWD
export PATH=$PATH:${TET_ROOT}/bin

