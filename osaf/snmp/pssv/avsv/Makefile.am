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

SUBDIRS = avm

pkglib_LTLIBRARIES = libavsv_pssv.la

PSSV_GENERATED_FILES = \
	$(top_builddir)/osaf/snmp/pssv/avsv/avsv_pssv_init.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/avsv_pssv_init.h \
	$(top_builddir)/osaf/snmp/pssv/avsv/avsvscalars_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/ncs_avsv_mib_pssv_mib.h \
	$(top_builddir)/osaf/snmp/pssv/avsv/ncsndtableentry_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/ncssgtableentry_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/ncssitableentry_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/ncssutableentry_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/saamfcompcstypesupportedtableentry_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/saamfcomptableentry_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/saamfcsinamevaluetableentry_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/saamfcsitableentry_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/saamfcstypeparamentry_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/saamfhealthchecktableentry_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/saamfnodetableentry_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/saamfscalars_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/saamfsgtableentry_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/saamfsitableentry_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/saamfsisideptableentry_pssv_mib.c  \
	$(top_builddir)/osaf/snmp/pssv/avsv/saamfsuspersirankentry_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/saamfsutableentry_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/saclmnodetableentry_pssv_mib.c \
	$(top_builddir)/osaf/snmp/pssv/avsv/saf_amf_mib_pssv_mib.h \
	$(top_builddir)/osaf/snmp/pssv/avsv/saf_cluster_mib_pssv_mib.h \
	$(top_builddir)/osaf/snmp/pssv/avsv/safclmscalarobject_pssv_mib.c

libavsv_pssv_la_CPPFLAGS = \
	-DNCS_PSR=1 -DPSR_LOG=1 -DNCS_PSS_RED=1 -DNCS_PSS_DEBUG=0 -DNCS_MBCSV=1 \
	-DMBCSV_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_builddir)/osaf/snmp/mapi/include \
	-I$(top_srcdir)/osaf/libs/common/avsv/include \
	-I$(top_srcdir)/osaf/services/saf/avsv/avd/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

nodist_libavsv_pssv_la_SOURCES = $(PSSV_GENERATED_FILES)

libavsv_pssv_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@

$(PSSV_GENERATED_FILES): pssv

pssv:
	@(if [ ! -f pssv.gen ]; then \
		export SMIPATH=$(top_srcdir)/osaf/snmp/mibs:$(MIBDIRS); \
		export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
		smidump -R $(top_srcdir)/OpenSAFCopyRights \
		-f ncspssv \
		-d $(top_builddir)/osaf \
		-o avsv SAF-AMF-MIB SAF-CLM-MIB NCS-AVSV-MIB; \
		touch pssv.gen; \
	fi)

CLEANFILES = \
	$(PSSV_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB \
	pssv.gen
