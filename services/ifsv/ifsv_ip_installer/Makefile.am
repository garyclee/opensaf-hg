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

bin_PROGRAMS = ncs_ifsv_ip_installer

ncs_ifsv_ip_installer_CPPFLAGS = \
	-DNCS_SAF=1 -DNCS_IFSV_BOND_INTF=1 \
	-I $(top_srcdir)/include

ncs_ifsv_ip_installer_SOURCES = \
	ifsv_app.h \
	ifsv_demo.h \
	drv_samp.c \
	ifsv_demo.c \
	ifsv_app.c

ncs_ifsv_ip_installer_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/services/ifsv/common/libifsv_common.la \
	$(top_builddir)/services/ifsv/ifa/libifa.la
