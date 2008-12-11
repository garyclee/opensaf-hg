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

pkglib_LTLIBRARIES = libifsv_clicef.la

MAPI_GENERATED_FILES = \
	$(top_builddir)/services/ifsv/inc/ifsv_snmp_inc/ifsv_entp_mapi.h \
	$(top_builddir)/services/ifsv/inc/ifsv_snmp_inc/ipxs_mapi.h \
	$(top_builddir)/services/ifsv/inc/ifsv_snmp_inc/ifsv_ir_mapi.h \
	$(top_builddir)/services/ifsv/inc/ifsv_snmp_inc/vip_mapi.h

BUILT_SOURCES = $(MAPI_GENERATED_FILES)

libifsv_clicef_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_IFSV_BOND_INTF=1 -DNCS_MAB=1 -DNCS_SAF=1 -DNCS_IFSV_LOG=1 -DNCS_VIP=1 -DNCS_CLI=1 -DNCSCLI_FILE=1 -DNCSCLI_LOG=1 -DNCS_IFSV_IPXS=1 \
	-I $(top_srcdir)/services/ifsv/inc \
	-I $(top_builddir)/services/ifsv/inc/ifsv_snmp_inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/cli/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

libifsv_clicef_la_SOURCES = \
	ifsv_cli.c \
	vip_cli.c

libifsv_clicef_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@

$(MAPI_GENERATED_FILES): mapi

mapi:
	for file in $(MAPI_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBSDIR); \
			export PATH=$(top_srcdir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmapi \
			-d $(top_builddir)/services \
			-o ifsv_entp NCS-IFSV-MIB; \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBSDIR); \
			export PATH=$(top_srcdir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmapi \
			-d $(top_builddir)/services \
			-o ipxs NCS-IPXS-MIB; \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBSDIR); \
			export PATH=$(top_srcdir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmapi \
			-d $(top_builddir)/services \
			-o ifsv_ir NCS-IFSV-BIND-MIB; \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBSDIR); \
			export PATH=$(top_srcdir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmapi \
			-d $(top_builddir)/services \
			-o vip NCS-VIP-MIB; \
		fi \
	done

CLEANFILES = \
	$(MAPI_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB
