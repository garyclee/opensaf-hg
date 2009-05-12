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
# opensaf/tests/makefile
# top-level GNU Make makefile for building opensaf tests
#

#
# Usage examples:
#
# gmake
# gmake all
# gmake PLATFORM=linux64
# gmake PLATFORM=solaris
# gmake TARGET=all
# gmake [ avsv | cpsv | edsv | glsv | ifsv | logsv | mbcsv | mds | mqsv | srmsv ]
# gmake TARGET=clean
# gmake TARGET=clobber
# gmake tetsuites
# override defaults:
# gmake TARGET_ARCH=x86_64-redhat-linux-gnu
#       OPENSAF_HOME=/mydir/stuff/opensaf-1.1.0
#	OPEN_TWARE_HOME=/mydir/stuff/tetware
#	LIB_BASEDIR=/opensaf/rh4
#	PLATFORM=linux-64
#

#
# OPENSAF_HOME is the base directory of the opensaf source,
# which contains source files for both the product and tests.
# The build of tests depends on header files from OPENSAF_HOME.
# If not already set in the environment, defaults to the value below.
#
mydir := $(shell pwd)
OPENSAF_HOME ?= $(mydir)/..

include Makefile.common

#
# add logsv to this list when it's ready
#
SERVICES_DIRS	:= \
	cpsv \
	edsv \
	glsv \
	ifsv \
	mbcsv \
	mds \
	mqsv \
	srmsv

# These are the files required to run the OpenSAF tests
TET_SUITE_FILES := \
	tests/*/*.exe \
	tests/*/suites/* \
	tests/cpsv/src/tet_cpa.c \
	tests/build_env.sh \
	tests/osaf_test_env.sh \
	tests/run_osaf_tests.sh \
	tests/setup_tet_env.sh

.PHONY: all clean clobber tetsuites \
	$(SERVICES_DIRS)

TETSUITES := tet_suites_$(TARGET_ARCH)
	
all clean clobber: $(SERVICES_DIRS)

tetsuites: all
    ifeq "$(OS)" "Linux"
	cd ..; $(TAR) -zcf tests/$(TETSUITES).tgz $(TET_SUITE_FILES)
    endif
    ifeq "$(OS)" "SunOS"
	cd ..; $(TAR) -cf tests/$(TETSUITES) $(TET_SUITE_FILES); $(GZIP) -S .tgz tests/$(TETSUITES)
    endif

$(SERVICES_DIRS):
	$(MAKE) --directory=$@ $(TARGET)
