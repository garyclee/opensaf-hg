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
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

LIB_VERSION = 0:0:0

lib_LTLIBRARIES = libSaAmf.la

libSaAmf_la_SOURCES =

libSaAmf_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_AVA=1 -DNCS_AVSV_LOG=1

libSaAmf_la_LDFLAGS = -lpthread -ldl -lrt \
	-version-info $(LIB_VERSION)

libSaAmf_la_LIBADD  = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/services/common/libsaf_common.la \
	$(top_builddir)/services/avsv/ava/libava.la \
	$(top_builddir)/services/avsv/common/libavsv_common.la
