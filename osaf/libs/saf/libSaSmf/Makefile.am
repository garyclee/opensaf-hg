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
# Author(s): GoAhead Software
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = libSaSmf.map

LIB_VERSION = 0:0:0

SUBDIRS = pkgconfig

lib_LTLIBRARIES = libSaSmf.la

libSaSmf_la_SOURCES =

libSaSmf_la_CPPFLAGS = \
	$(AM_CPPFLAGS)

if HAVE_LD_VERSION_SCRIPT
libSaSmf_la_LDFLAGS = \
	-Wl,-version-script=$(top_srcdir)/osaf/libs/saf/libSaSmf/libSaSmf.map \
	-version-number $(LIB_VERSION)
else
libSaSmf_la_LDFLAGS = \
	-version-number $(LIB_VERSION)
endif

libSaSmf_la_LIBADD = \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/agents/saf/smfa/libsmfa.la \
	$(top_builddir)/osaf/libs/common/smfsv/libsmfsv_common.la
