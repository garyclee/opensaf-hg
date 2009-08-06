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

lib_LTLIBRARIES = libifsv_common.la

libifsv_common_la_CPPFLAGS = \
	-DNCS_VIP=1 -DNCS_MAB=1 -DVIP_HALS_SUPPORT=1 -DNCS_IFSV_BOND_INTF=1 \
	-DNCS_IFSV_LOG=1 -DNCS_IFSV_IPXS=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/ifsv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include \
	-I$(top_builddir)/osaf/snmp/mapi/include

libifsv_common_la_SOURCES = \
	ifsv_vip_db.c \
	ifsv_cif.c \
	ifsv_drv.c \
	ifsv_log.c \
	ifsv_mds.c \
	ifsv_tmr.c \
	ifsv_db.c \
	ifsvipxs.c \
	ipxs_edu.c

libifsv_common_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@
