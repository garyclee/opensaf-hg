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

pkglib_LTLIBRARIES = libifsv_bind_subagt.la

SUBAGT_GENERATED_FILES = \
	$(top_builddir)/osaf/snmp/subagt/services/ifsv/entp_ifsv_ir/ifsv_ir_subagt_init.c \
	$(top_builddir)/osaf/snmp/subagt/services/ifsv/entp_ifsv_ir/ifsv_ir_subagt_init.h \
	$(top_builddir)/osaf/snmp/subagt/services/ifsv/entp_ifsv_ir/ncs_ifsv_bind_mib_agt_stub.c \
	$(top_builddir)/osaf/snmp/subagt/services/ifsv/entp_ifsv_ir/ncs_ifsv_bind_mib_agt_stub.h

libifsv_bind_subagt_la_CPPFLAGS = \
	-DBBS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=0 \
	$(AM_CPPFLAGS)

nodist_libifsv_bind_subagt_la_SOURCES = $(SUBAGT_GENERATED_FILES)

libifsv_bind_subagt_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@

$(SUBAGT_GENERATED_FILES): subagt

subagt:
	@(if [ ! -f subagt.gen ]; then \
		export SMIPATH=$(top_srcdir)/osaf/snmp/mibs:$(MIBDIRS); \
		export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
		smidump -R $(top_srcdir)/OpenSAFCopyRights \
		-f synncsnetsnmp_new \
		-d $(top_builddir)/osaf \
		-o ifsv_ir NCS-IFSV-BIND-MIB; \
		touch subagt.gen; \
	fi)

CLEANFILES = \
	$(SUBAGT_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB \
	subagt.gen
