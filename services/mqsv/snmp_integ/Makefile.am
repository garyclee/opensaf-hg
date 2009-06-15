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

pkglib_LTLIBRARIES = libmqsv_subagt.la

SUBAGT_GENERATED_FILES = \
	$(top_builddir)/services/mqsv/snmp_integ/mqsv_subagt_init.c \
	$(top_builddir)/services/mqsv/snmp_integ/mqsv_subagt_init.h \
	$(top_builddir)/services/mqsv/snmp_integ/saf_msg_mib_agt_stub.c \
	$(top_builddir)/services/mqsv/snmp_integ/saf_msg_mib_agt_stub.h

libmqsv_subagt_la_CPPFLAGS = \
	-DNCS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=1 -DNCS_AVA=1 -DNCS_AVSV_LOG=1 \
	-DNCS_EDA=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/mqsv/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/common/inc \
	-I$(top_srcdir)/services/mab/inc

nodist_libmqsv_subagt_la_SOURCES = $(SUBAGT_GENERATED_FILES)

libmqsv_subagt_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@

$(SUBAGT_GENERATED_FILES): subagt

subagt:
	@for file in $(SUBAGT_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBDIRS); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f synncsnetsnmp_new \
			-d $(top_builddir)/services \
			-o mqsv SAF-MSG-MIB; \
		fi \
	done

CLEANFILES = \
	$(SUBAGT_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB
