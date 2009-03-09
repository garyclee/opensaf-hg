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

pkglib_LTLIBRARIES = libifsv_entp_subagt.la

SUBAGT_GENERATED_FILES = \
	$(top_builddir)/services/ifsv/entp_snmp_integ/ifsv_entp_subagt_init.c \
	$(top_builddir)/services/ifsv/entp_snmp_integ/ifsv_entp_subagt_init.h \
	$(top_builddir)/services/ifsv/entp_snmp_integ/ncs_ifsv_mib_agt_stub.c \
	$(top_builddir)/services/ifsv/entp_snmp_integ/ncs_ifsv_mib_agt_stub.h

libifsv_entp_subagt_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DBBS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=0 \
	-I $(top_srcdir)/services/ifsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/include

nodist_libifsv_entp_subagt_la_SOURCES = $(SUBAGT_GENERATED_FILES)

libifsv_entp_subagt_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@

$(SUBAGT_GENERATED_FILES): subagt

subagt:
	@for file in $(SUBAGT_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBDIRS); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f synncsnetsnmp_new \
			-d $(top_builddir)/services \
			-o ifsv_entp NCS-IFSV-MIB; \
		fi \
	done

CLEANFILES = \
	$(SUBAGT_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB
