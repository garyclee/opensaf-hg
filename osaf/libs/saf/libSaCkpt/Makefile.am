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

EXTRA_DIST = libSaCkpt.map

LIB_VERSION = 1:0:2	

SUBDIRS = pkgconfig

lib_LTLIBRARIES = libSaCkpt.la

libSaCkpt_la_SOURCES =

libSaCkpt_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DNCS_CPA=1 -DNCS_CPSV_LOG=1

if HAVE_LD_VERSION_SCRIPT
libSaCkpt_la_LDFLAGS = \
	-Wl,-version-script=$(top_srcdir)/osaf/libs/saf/libSaCkpt/libSaCkpt.map \
	-version-number $(LIB_VERSION)
else
libSaCkpt_la_LDFLAGS = \
	-version-number $(LIB_VERSION)
endif

libSaCkpt_la_LIBADD = \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/common/ais/libais_common.la \
	$(top_builddir)/osaf/libs/agents/saf/cpa/libcpa.la \
	$(top_builddir)/osaf/libs/common/cpsv/libcpsv_common.la
