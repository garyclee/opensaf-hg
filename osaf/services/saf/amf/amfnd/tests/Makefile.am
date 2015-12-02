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

TESTS = testamfnd

check_PROGRAMS = testamfnd

testamfnd_CXXFLAGS =$(AM_CXXFLAGS)

testamfnd_CPPFLAGS = \
	-I$(GTEST_DIR)/include

testamfnd_LDFLAGS =

# Add test case cc files here
testamfnd_SOURCES =

testamfnd_LDADD = \
	$(GTEST_DIR)/lib/libgtest.la \
	$(GTEST_DIR)/lib/libgtest_main.la
