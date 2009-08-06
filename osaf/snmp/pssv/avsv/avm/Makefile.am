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

pkglib_LTLIBRARIES = libavm_pssv.la

PSSV_GENERATED_FILES = \
	$(top_builddir)/osaf/snmp/pssv/avsv/avm/avm_pssv_init.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/avm/avm_pssv_init.h \
	$(top_builddir)/osaf/snmp/pssv/avsv/avm/ncs_avm_mib_pssv_mib.h \
	$(top_builddir)/osaf/snmp/pssv/avsv/avm/ncsavmentdeploytableentry_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/avm/ncsavmentupgradetableentry_pssv_mib.c

libavm_pssv_la_CPPFLAGS = \
	-DNCS_PSR=1 -DPSR_LOG=1 -DNCS_PSS_RED=1 -DNCS_PSS_DEBUG=0 \
	$(AM_CPPFLAGS) \
	-I$(top_builddir)/osaf/snmp/mapi/include \
	-I$(top_srcdir)/osaf/services/saf/avsv/avm/include \
	-I$(top_srcdir)/osaf/services/infrastructure/hisv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

libavm_pssv_la_CFLAGS = $(AM_CFLAGS) @HPI_CFLAGS@

nodist_libavm_pssv_la_SOURCES = $(PSSV_GENERATED_FILES)

libavm_pssv_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@

$(PSSV_GENERATED_FILES): pssv

pssv:
	@(if [ ! -f pssv.gen ]; then \
		export SMIPATH=$(top_srcdir)/osaf/snmp/mibs:$(MIBDIRS); \
		export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
		smidump -R $(top_srcdir)/OpenSAFCopyRights \
		-f ncspssv \
		-d $(top_builddir)/osaf \
		-o avm NCS-AVM-MIB; \
		touch pssv.gen; \
	fi)

CLEANFILES = \
	$(PSSV_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB \
	pssv.gen
