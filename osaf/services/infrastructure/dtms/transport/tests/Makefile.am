#      -*- OpenSAF  -*-
#
# (C) Copyright 2016 The OpenSAF Foundation
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

DEFAULT_INCLUDES =

TESTS = transport_test

check_PROGRAMS = $(TESTS)

transport_test_CXXFLAGS =$(AM_CXXFLAGS)

transport_test_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(GTEST_DIR)/include

transport_test_LDFLAGS = \
	-pthread -lrt \
	$(top_builddir)/osaf/libs/core/cplusplus/base/libbase_la-getenv.o \
	$(top_builddir)/osaf/services/infrastructure/dtms/transport/osaftransportd-transport_monitor.o

transport_test_SOURCES = \
	transport_monitor_test.cc \
	mock_logtrace.cc \
	mock_osaf_poll.cc

transport_test_LDADD = \
	$(GTEST_DIR)/lib/libgtest.la \
	$(GTEST_DIR)/lib/libgtest_main.la
