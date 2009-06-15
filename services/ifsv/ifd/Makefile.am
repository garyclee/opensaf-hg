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

noinst_LTLIBRARIES = libifd.la

MIBLIB_GENERATED_FILES = \
	$(top_builddir)/services/ifsv/ifd/ifd_miblib_gen/ifmibobjects_mib.c \
	$(top_builddir)/services/ifsv/ifd/ifd_miblib_gen/ncsifsvifipentry_mib.c \
	$(top_builddir)/services/ifsv/ifd/ifd_miblib_gen/ncsifsvifxentry_mib.c \
	$(top_builddir)/services/ifsv/ifd/ifd_miblib_gen/intf_mib.c \
	$(top_builddir)/services/ifsv/ifd/ifd_miblib_gen/ncsifsvifmapentry_mib.c \
	$(top_builddir)/services/ifsv/ifd/ifd_miblib_gen/ncsifsvipentry_mib.c \
	$(top_builddir)/services/ifsv/ifd/ifsv_ir_miblib_gen/ncsifsvbindifentry_mib.c \
	$(top_builddir)/services/ifsv/ifd/vip_miblib_gen/ncsvipentry_mib.c

libifd_la_CPPFLAGS = \
	-DNCS_IFD=1 -DNCS_MAB=1 -DNCS_IFSV_LOG=1 -DNCS_VIP=1 -DVIP_HALS_SUPPORT=1 \
	-DNCS_IFSV_BOND_INTF=1 -DNCS_IFSV_IPXS=1 \
	$(AM_CPPFLAGS) \
	-I$(top_builddir)/services/ifsv/inc/ifsv_snmp_inc \
	-I$(top_srcdir)/services/ifsv/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/common/inc

libifd_la_LDFLAGS = -static

libifd_la_SOURCES = \
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

nodist_libifd_la_SOURCES = $(MIBLIB_GENERATED_FILES)

$(MIBLIB_GENERATED_FILES): miblib

miblib:
	@for file in $(MIBLIB_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBDIRS); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmiblib \
			-d $(top_builddir)/services \
			NCS-IPXS-MIB; \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBDIRS); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmiblib \
			-d $(top_builddir)/services \
			NCS-IFSV-MIB; \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBDIRS); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmiblib \
			-d $(top_builddir)/services \
			NCS-IFSV-BIND-MIB; \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBDIRS); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmiblib \
			-d $(top_builddir)/services \
			NCS-VIP-MIB; \
		fi \
	done

CLEANFILES = \
	$(MIBLIB_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB

clean-local:
	-rm -rf $(top_builddir)/services/ifsv/ifd/ifd_miblib_gen
	-rm -rf $(top_builddir)/services/ifsv/ifd/ifsv_ir_miblib_gen
	-rm -rf $(top_builddir)/services/ifsv/ifd/vip_miblib_gen
