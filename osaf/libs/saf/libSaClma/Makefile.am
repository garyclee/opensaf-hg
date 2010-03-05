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
# Author(s):  Emerson Network Power
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

LIB_VERSION = 0:0:0

SUBDIRS = pkgconfig

lib_LTLIBRARIES = libSaClma.la

libSaClma_la_SOURCES =

libSaClma_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DNCS_CLA=1 -DNCS_AVSV_LOG=1

libSaClma_la_LDFLAGS = -version-info $(LIB_VERSION)

libSaClma_la_LIBADD = \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/agents/saf/clma/libclma.la \
	$(top_builddir)/osaf/libs/common/clmsv/libclmsv_common.la
