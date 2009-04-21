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
#

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = ntfsend ntfsubscribe

noinst_HEADERS = \
	inc/ntfclient.h

ntfsend_CPPFLAGS = \
	-I $(top_srcdir)/services/ntfsv/utils/inc \
	-I $(top_srcdir)/services/ntfsv/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include


ntfsend_SOURCES = \
	ntfsend.c

ntfsend_LDADD = $(top_builddir)/lib/lib_SaNtf/libSaNtf.la

ntfsubscribe_CPPFLAGS = \
	-I $(top_srcdir)/services/ntfsv/utils/inc \
	-I $(top_srcdir)/services/ntfsv/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include


ntfsubscribe_SOURCES = \
	ntfsubscribe.c

ntfsubscribe_LDADD = $(top_builddir)/lib/lib_SaNtf/libSaNtf.la

