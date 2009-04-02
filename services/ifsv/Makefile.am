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

CONTROLLER_SUBDIRS = clicmds ifd logstr

SUBDIRS = ifnd ifsv_ip_installer $(CONTROLLER_SUBDIRS)

noinst_HEADERS = \
	inc/ifa_db.h \
	inc/ifa_dl_api.h \
	inc/ifa.h \
	inc/ifa_mds.h \
	inc/ifd_dl_api.h \
	inc/ifd.h \
	inc/ifd_ipxs.h \
	inc/ifd_ir.h \
	inc/ifd_mds.h \
	inc/ifd_mib.h \
	inc/ifdproc.h \
	inc/ifd_red.h \
	inc/ifd_saf.h \
	inc/ifnd_dl_api.h \
	inc/ifnd.h \
	inc/ifnd_ipxs.h \
	inc/ifnd_mds.h \
	inc/ifnd_mib.h \
	inc/ifndproc.h \
	inc/ifnd_red.h \
	inc/ifnd_saf.h \
	inc/ifsv_cb.h \
	inc/ifsv_cli.h \
	inc/ifsv_db.h \
	inc/ifsvd_cb.h \
	inc/ifsvdevt.h \
	inc/ifsv_drv.h \
	inc/ifsv_evt.h \
	inc/ifsv.h \
	inc/ifsvifap.h \
	inc/ifsvinit.h \
	inc/ifsvipxs.h \
	inc/ifsv_log.h \
	inc/ifsv_logstr.h \
	inc/ifsv_mds.h \
	inc/ifsv_mem.h \
	inc/ifsv_mib.h \
	inc/ifsv_mib_inc.h \
	inc/ifsvproc.h \
	inc/ifsv_red_common.h \
	inc/ifsv_saf.h \
	inc/ifsv_tmr.h \
	inc/ifsv_vip_mem.h \
	inc/ipxs_cip.h \
	inc/ipxs_db.h \
	inc/ipxs_evt.h \
	inc/ipxs_mem.h \
	inc/vip_cli.h \
	inc/vip_db.h \
	inc/vip.h \
	inc/vip_mib.h \
	inc/vip_tbl.h

pkglib_PROGRAMS = ncs_ifd ncs_ifnd

ncs_ifd_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_IFD=1 -DNCS_VIP=1 -DVIP_HALS_SUPPORT=1 -DNCS_IFSV_BOND_INTF=1 -DNCS_IFSV_LOG=1 \
	-I $(top_srcdir)/services/ifsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/vds/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/include

ncs_ifd_SOURCES = \
	$(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_ifd_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/services/mbcsv/src/libmbca.la \
	$(top_builddir)/services/ifsv/ifd/libifd.la \
	$(top_builddir)/services/ifsv/common/libifsv_common.la

ncs_ifnd_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_IFND=1 -DNCS_VIP=1 -DVIP_HALS_SUPPORT=1 -DNCS_IFSV_BOND_INTF=1 -DNCS_IFSV_LOG=1 \
	-I $(top_srcdir)/services/ifsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/vds/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/include

ncs_ifnd_SOURCES = \
	$(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_ifnd_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/services/ifsv/ifnd/libifnd.la \
	$(top_builddir)/services/ifsv/common/libifsv_common.la
