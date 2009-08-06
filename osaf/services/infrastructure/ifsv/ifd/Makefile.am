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

pkglib_PROGRAMS = ncs_ifd

ncs_ifd_CPPFLAGS = \
	-DNCS_IFD=1 -DNCS_MAB=1 -DNCS_IFSV_LOG=1 -DNCS_VIP=1 -DVIP_HALS_SUPPORT=1 \
	-DNCS_IFSV_BOND_INTF=1 -DNCS_IFSV_IPXS=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/ifsv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/vds/include \
	-I$(top_builddir)/osaf/snmp/mapi/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

ncs_ifd_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c \
	ifd_ir_mib.c \
	ifsvifap.c \
	ifd_mbcsv.c \
	ipxs_cip.c \
	ifsv_cifmap.c \
	ifd_ir.c \
	vip_reg_oac.c \
	ifd_vipproc.c \
	ifd_api.c \
	ifd_evt.c \
	ifd_mds.c \
	ifd_mib.c \
	ifd_red.c \
	ifd_saf.c \
	ifd_cgbl.c \
	vip_mib.c \
	ifd_ipxs.c \
	ifsv_cxif.c \
	ifdproc.c

ncs_ifd_LDADD = \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la \
	$(top_builddir)/osaf/services/infrastructure/ifsv/common/libifsv_common.la \
	$(top_builddir)/osaf/snmp/miblib/ifsv/ifd/libifd_miblib.la
