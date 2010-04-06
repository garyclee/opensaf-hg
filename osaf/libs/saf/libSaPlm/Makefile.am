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
# Author(s): Emerson 
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

LIB_VERSION = 0:0:0

SUBDIRS = pkgconfig

lib_LTLIBRARIES = libSaPlm.la

libSaPlm_la_SOURCES =

libSaPlm_la_CPPFLAGS = \
	$(AM_CPPFLAGS)

libSaPlm_la_LDFLAGS = -version-info $(LIB_VERSION)

libSaPlm_la_LIBADD = \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/agents/saf/plma/libplma.la \
	$(top_builddir)/osaf/libs/common/plmsv/libplmsv_common.la
