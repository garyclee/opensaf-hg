#      -*- OpenSAF  -*-
#
# (C) Copyright 2015 The OpenSAF Foundation
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

TESTS = testamfd

check_PROGRAMS = testamfd

testamfd_CXXFLAGS =$(AM_CXXFLAGS)

testamfd_CPPFLAGS = \
	-DSA_CLM_B01=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/amf/include \
	-I$(top_srcdir)/osaf/services/saf/amf/amfd/include \
	-I$(top_srcdir)/osaf/libs/common/immsv/include \
	-I$(top_srcdir)/osaf/tools/saflog/include \
	-I$(GTEST_DIR)/include

testamfd_LDFLAGS = \
	-lpthread

testamfd_SOURCES = \
	test_amfdb.cc

testamfd_LDADD = \
	$(GTEST_DIR)/lib/libgtest.la \
	$(GTEST_DIR)/lib/libgtest_main.la \
	$(top_builddir)/osaf/tools/saflog/src/libsaflog.la \
	$(top_builddir)/osaf/tools/safimm/src/libimmutil.la \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/common/amf/libamf_common.la \
	$(top_builddir)/osaf/libs/saf/libSaNtf/libSaNtf.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOi.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOm.la \
	$(top_builddir)/osaf/libs/saf/libSaClm/libSaClm.la \
	$(top_builddir)/osaf/libs/saf/libSaLog/libSaLog.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/rda/librda.la
