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
# Author(s): Ericsson AB
#
#

MAINTAINERCLEANFILES = Makefile.in

OILIB_VERSION=0:1:1
OMLIB_VERSION=0:1:1

EXTRA_DIST = libSaImmOi.version libSaImmOm.version

lib_LTLIBRARIES = libSaImmOi.la libSaImmOm.la

libSaImmOi_la_SOURCES = 

libSaImmOi_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@

libSaImmOi_la_LDFLAGS = -Wl,-version-script=$(top_srcdir)/lib/lib_SaImm/libSaImmOi.version \
			-version-number $(OILIB_VERSION)

libSaImmOi_la_LIBADD = $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/services/common/libsaf_common.la \
	$(top_builddir)/services/immsv/common/libimmsv_common.la \
	$(top_builddir)/services/immsv/imma/libimmaOi.la

libSaImmOm_la_SOURCES = 

libSaImmOm_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@

libSaImmOm_la_LDFLAGS = -Wl,-version-script=$(top_srcdir)/lib/lib_SaImm/libSaImmOm.version \
			-version-number $(OMLIB_VERSION)

libSaImmOm_la_LIBADD = $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/services/common/libsaf_common.la \
	$(top_builddir)/services/immsv/common/libimmsv_common.la \
	$(top_builddir)/services/immsv/imma/libimmaOm.la
