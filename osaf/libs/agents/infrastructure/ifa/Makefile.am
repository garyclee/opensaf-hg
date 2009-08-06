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

lib_LTLIBRARIES = libifa.la

libifa_la_CPPFLAGS = \
	-DNCS_IFA=1 -DNCS_IFSV_LOG=1 -DNCS_VIP=1 -DNCS_IFSV_IPXS=1 \
	-DVIP_HALS_SUPPORT=1 -DNCS_IFSV_BOND_INTF=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/ifsv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include \
	-I$(top_builddir)/osaf/snmp/mapi/include

libifa_la_SOURCES = \
	ifa_proc.c \
	ifa_api.c \
	ifa_mds.c \
	ifa_vipproc.c \
	ipxs_aproc.c

libifa_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@

clean-local:
	-rm -rf $(top_builddir)/services/ifsv/inc/ifsv_snmp_inc
