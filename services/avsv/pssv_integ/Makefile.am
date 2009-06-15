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

pkglib_LTLIBRARIES = libavsv_pssv.la

PSSV_GENERATED_FILES = \
	$(top_builddir)/services/avsv/pssv_integ/avsv_pssv_init.c \
	$(top_builddir)/services/avsv/pssv_integ/avsv_pssv_init.h \
	$(top_builddir)/services/avsv/pssv_integ/avsvscalars_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/ncs_avsv_mib_pssv_mib.h \
	$(top_builddir)/services/avsv/pssv_integ/ncsndtableentry_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/ncssgtableentry_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/ncssitableentry_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/ncssutableentry_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/saamfcompcstypesupportedtableentry_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/saamfcomptableentry_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/saamfcsinamevaluetableentry_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/saamfcsitableentry_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/saamfcstypeparamentry_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/saamfhealthchecktableentry_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/saamfnodetableentry_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/saamfscalars_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/saamfsgtableentry_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/saamfsitableentry_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/saamfsisideptableentry_pssv_mib.c  \
	$(top_builddir)/services/avsv/pssv_integ/saamfsuspersirankentry_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/saamfsutableentry_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/saclmnodetableentry_pssv_mib.c \
	$(top_builddir)/services/avsv/pssv_integ/saf_amf_mib_pssv_mib.h \
	$(top_builddir)/services/avsv/pssv_integ/saf_cluster_mib_pssv_mib.h \
	$(top_builddir)/services/avsv/pssv_integ/safclmscalarobject_pssv_mib.c

libavsv_pssv_la_CPPFLAGS = \
	-DNCS_PSR=1 -DPSR_LOG=1 -DNCS_PSS_RED=1 -DNCS_PSS_DEBUG=0 -DNCS_MBCSV=1 -DMBCSV_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_builddir)/services/avsv/inc/avsv_snmp_inc \
	-I$(top_srcdir)/services/avsv/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/common/inc

nodist_libavsv_pssv_la_SOURCES = $(PSSV_GENERATED_FILES)

libavsv_pssv_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@

$(PSSV_GENERATED_FILES): pssv

pssv:
	@for file in $(PSSV_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBDIRS); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncspssv \
			-d $(top_builddir)/services \
			-o avsv SAF-AMF-MIB SAF-CLM-MIB NCS-AVSV-MIB; \
		fi \
	done

CLEANFILES = \
	$(PSSV_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB
