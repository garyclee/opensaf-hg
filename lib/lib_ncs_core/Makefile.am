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

LIB_VERSION = 0:1:0

lib_LTLIBRARIES = libncs_core.la 

libncs_core_la_SOURCES =

libncs_core_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@

libncs_core_la_LDFLAGS = -lpthread -lc -ldl -lm -lrt \
                         -version-info $(LIB_VERSION)

libncs_core_la_LIBADD = \
	$(top_builddir)/services/leap/libleap.la \
	$(top_builddir)/services/common/libncs_common.la \
	$(top_builddir)/services/mds/libmds.la \
	$(top_builddir)/services/vds/vda/libvda.la \
	$(top_builddir)/services/mab/oac/liboaa.la \
	$(top_builddir)/services/dtsv/dta/libdta.la
