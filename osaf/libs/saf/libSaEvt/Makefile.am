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

EXTRA_DIST = libSaEvt.map

LIB_VERSION = 1:0:3

SUBDIRS = pkgconfig

lib_LTLIBRARIES = libSaEvt.la

libSaEvt_la_SOURCES =

libSaEvt_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DNCS_EDA=1 -DNCS_EDSV_LOG=1

if HAVE_LD_VERSION_SCRIPT
libSaEvt_la_LDFLAGS = \
	-Wl,-version-script=$(top_srcdir)/osaf/libs/saf/libSaEvt/libSaEvt.map \
	-version-number $(LIB_VERSION)
else
libSaEvt_la_LDFLAGS = \
	-version-number $(LIB_VERSION)
endif

libSaEvt_la_LIBADD = \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/agents/saf/eda/libeda.la \
	$(top_builddir)/osaf/libs/common/edsv/libedsv_common.la
