#      -*- OpenSAF  -*-
#
# (C) Copyright 2009 The OpenSAF Foundation
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
# Author(s): Ericsson AB
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = logtest

logtest_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/common/inc \
	-I$(top_srcdir)/tests/immsv/common \
	-I$(top_srcdir)/tests/unit_test_fw/inc

logtest_SOURCES = \
	$(top_srcdir)/tests/unit_test_fw/src/utest.c \
	$(top_srcdir)/tests/unit_test_fw/src/util.c \
	$(top_srcdir)/tests/logsv/src/logtest.c \
	$(top_srcdir)/tests/logsv/src/tet_saLogInitialize.c \
	$(top_srcdir)/tests/logsv/src/tet_saLogSelectionObjectGet.c \
	$(top_srcdir)/tests/logsv/src/tet_saLogDispatch.c \
	$(top_srcdir)/tests/logsv/src/tet_saLogFinalize.c \
	$(top_srcdir)/tests/logsv/src/tet_saLogStreamOpen_2.c \
	$(top_srcdir)/tests/logsv/src/tet_saLogStreamOpenAsync_2.c \
	$(top_srcdir)/tests/logsv/src/tet_saLogStreamOpenCallbackT.c \
	$(top_srcdir)/tests/logsv/src/tet_saLogWriteLog.c \
	$(top_srcdir)/tests/logsv/src/tet_saLogWriteLogAsync.c \
	$(top_srcdir)/tests/logsv/src/tet_saLogWriteLogCallbackT.c \
	$(top_srcdir)/tests/logsv/src/tet_saLogFilterSetCallbackT.c \
	$(top_srcdir)/tests/logsv/src/tet_saLogStreamClose.c \
	$(top_srcdir)/tests/logsv/src/tet_saLogLimitGet.c \
	$(top_srcdir)/tests/logsv/src/tet_LogOiOps.c

logtest_LDADD = \
	$(top_builddir)/lib/lib_SaLog/libSaLog.la
