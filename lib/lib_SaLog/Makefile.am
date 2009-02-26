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

MAINTAINERCLEANFILES = Makefile.in

LIB_VERSION=1:0:0

EXTRA_DIST = \
	libSaLog.version

lib_LTLIBRARIES = libSaLog.la

libSaLog_la_SOURCES = 

libSaLog_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@

libSaLog_la_LDFLAGS = -Wl,-version-script=$(top_srcdir)/lib/lib_SaLog/libSaLog.version \
                      -version-info $(LIB_VERSION)

libSaLog_la_LIBADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/services/common/libsaf_common.la \
	$(top_builddir)/services/logsv/lga/liblga.la
