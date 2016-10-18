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

TESTS = libbase_test

check_PROGRAMS = $(TESTS)

libbase_test_CXXFLAGS =$(AM_CXXFLAGS)

libbase_test_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/core/cplusplus \
	-I$(GTEST_DIR)/include

libbase_test_LDFLAGS = \
	-pthread -lrt \
	$(top_builddir)/osaf/libs/core/cplusplus/base/libbase_la-getenv.o \
	$(top_builddir)/osaf/libs/core/cplusplus/base/libbase_la-log_message.o \
	$(top_builddir)/osaf/libs/core/cplusplus/base/libbase_la-process.o

libbase_test_SOURCES = \
	getenv_test.cc \
	log_message_test.cc \
	mock_logtrace.cc \
	mock_osaf_abort.cc \
	mock_osafassert.cc \
	time_add_test.cc \
	time_compare_test.cc \
	time_convert_test.cc \
	time_subtract_test.cc

libbase_test_LDADD = \
	$(GTEST_DIR)/lib/libgtest.la \
	$(GTEST_DIR)/lib/libgtest_main.la
