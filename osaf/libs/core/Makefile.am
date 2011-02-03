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
# Author(s): Wind River Systems
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = include common leap mbcsv mds

LIB_VERSION = 0:2:0

lib_LTLIBRARIES = libopensaf_core.la

libopensaf_core_la_SOURCES =

libopensaf_core_la_CPPFLAGS = \
	$(AM_CPPFLAGS)

libopensaf_core_la_LDFLAGS = \
	-lpthread -ldl -lrt \
	-version-number $(LIB_VERSION)

libopensaf_core_la_LIBADD = \
	$(top_builddir)/osaf/libs/core/common/libopensaf_common.la \
	$(top_builddir)/osaf/libs/core/leap/libleap.la \
	$(top_builddir)/osaf/libs/core/mbcsv/libmbca.la \
	$(top_builddir)/osaf/libs/core/mds/libmds.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/dta/libdta.la
