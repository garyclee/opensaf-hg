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
# Author(s): Wind River Systems
#            Ericsson AB
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

noinst_HEADERS = \
	ntfa.h

noinst_LTLIBRARIES = libntfa.la

libntfa_la_CPPFLAGS = \
	-DSA_EXTENDED_NAME_SOURCE \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/ntfsv/include

libntfa_la_SOURCES = \
	ntfa_api.c \
	ntfa_util.c \
	ntfa_mds.c

libntfa_la_LDFLAGS = -static
