#      -*- OpenSAF  -*-
#
# (C) Copyright 2010 The OpenSAF Foundation
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

EXTRA_DIST = libSaMsg.map

LIB_VERSION = 1:0:2

SUBDIRS = pkgconfig

lib_LTLIBRARIES = libSaMsg.la

libSaMsg_la_SOURCES =

libSaMsg_la_CPPFLAGS = \
	$(AM_CPPFLAGS)

if HAVE_LD_VERSION_SCRIPT
libSaMsg_la_LDFLAGS = \
	-Wl,-version-script=$(top_srcdir)/osaf/libs/saf/libSaMsg/libSaMsg.map \
	-version-number $(LIB_VERSION)
else
libSaMsg_la_LDFLAGS = \
	-version-number $(LIB_VERSION)
endif

libSaMsg_la_LIBADD = \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/agents/saf/mqa/libmqa.la \
	$(top_builddir)/osaf/libs/common/mqsv/libmqsv_common.la
