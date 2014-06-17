#      -*- OpenSAF  -*-
#
# (C) Copyright 2010 The OpenSAF Foundation
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
# Author(s): Emerson Network Power
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = clmtest

noinst_HEADERS = \
	clmtest.h

clmtest_CPPFLAGS = \
	-DSA_CLM_B01=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/tests/unit_test_fw/inc

clmtest_SOURCES = \
	clmtest.c \
	tet_saClmInitialize.c \
	tet_saClmClusterNodeGetAsync.c \
	tet_saClmClusterTrack.c \
	tet_saClmClusterTrackStop.c \
	tet_saClmClusterNotificationFree.c \
	tet_saClmClusterNodeGet.c \
	tet_saClmResponse.c \
	tet_ClmOiOps.c \
	tet_saClmDispatch.c \
	tet_saClmSelectionObjectGet.c \
	tet_saClmFinalize.c

clmtest_LDADD = \
	$(top_builddir)/osaf/libs/saf/libSaClm/libSaClm.la \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/tests/unit_test_fw/src/libutest.la
