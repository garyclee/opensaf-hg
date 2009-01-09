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

lib_LTLIBRARIES = libSaEvt.la

libSaEvt_la_SOURCES = 

libSaEvt_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_EDA=1 -DNCS_EDSV_LOG=1

libSaEvt_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@

libSaEvt_la_LIBADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/services/common/libsaf_common.la \
	$(top_builddir)/services/edsv/common/libedsv_common.la \
	$(top_builddir)/services/edsv/eda/libeda.la
