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

TESTS = testamfa

check_PROGRAMS = testamfa

testamfa_CXXFLAGS =$(AM_CXXFLAGS)

testamfa_CPPFLAGS = \
	-I$(GTEST_DIR)/include

testamfa_LDFLAGS =

# Add test case cc files here
testamfa_SOURCES =

testamfa_LDADD = \
	$(GTEST_DIR)/lib/libgtest.la \
	$(GTEST_DIR)/lib/libgtest_main.la
