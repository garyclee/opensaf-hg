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

MAINTAINERCLEANFILES = Makefile.in

LIB_VERSION = 0:0:0

lib_LTLIBRARIES = libSaClm.la

libSaClm_la_SOURCES =

libSaClm_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_CLA=1 -DNCS_AVSV_LOG=1

libSaClm_la_LDFLAGS = -version-info $(LIB_VERSION)

libSaClm_la_LIBADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/services/avsv/common/libavsv_common.la \
	$(top_builddir)/services/avsv/cla/libcla.la
