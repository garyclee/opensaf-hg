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

lib_LTLIBRARIES = libifa.la

MAPI_GENERATED_FILES = \
	$(top_builddir)/services/ifsv/inc/ifsv_snmp_inc/ifsv_mapi.h

BUILT_SOURCES = $(MAPI_GENERATED_FILES)

libifa_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_IFA=1 -DNCS_IFSV_LOG=1 -DNCS_VIP=1 -DNCS_IFSV_IPXS=1 -DVIP_HALS_SUPPORT=1 -DNCS_IFSV_BOND_INTF=1 \
	-I $(top_srcdir)/services/ifsv/inc \
	-I $(top_builddir)/services/ifsv/inc/ifsv_snmp_inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/include

libifa_la_SOURCES = \
	ifa_proc.c \
	ifa_api.c \
	ifa_mds.c \
	ifa_vipproc.c \
	ipxs_aproc.c

libifa_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@

$(MAPI_GENERATED_FILES): mapi

mapi:
	for file in $(MAPI_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBSDIR); \
			export PATH=$(top_srcdir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmapi \
			-d $(top_builddir)/services \
			-o ifsv IFMIB; \
		fi \
	done

CLEANFILES = \
	$(MAPI_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB
