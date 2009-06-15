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

SUBDIRS = logstr

noinst_LTLIBRARIES = libpsr.la

MIBLIB_GENERATED_FILES = \
	$(top_builddir)/services/mab/psr/pss_miblib_gen/ncspssvprofiletableentry_mib.c \
	$(top_builddir)/services/mab/psr/pss_miblib_gen/ncspssvscalars_mib.c

libpsr_la_CPPFLAGS = \
	-DNCS_PSR=1 -DNCS_MAB=1 -DMAB_LOG=1 -DPSR_LOG=1 -DNCS_PSS_RED=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/bam/inc \
	-I$(top_srcdir)/services/common/inc

libpsr_la_LDFLAGS = -static -Wl,--export-dynamic

libpsr_la_SOURCES = \
	psr_amf.c \
	psr_api.c \
	psr_ext.c \
	psr_init.c \
	psr_log.c \
	psr_mds.c \
	psr_mib.c \
	psr_pvt.c \
	pss_mbcsv.c \
	psr_rfmt.c \
	$(top_srcdir)/services/mab/mas_pss_common/app_amf.c

nodist_libpsr_la_SOURCES = $(MIBLIB_GENERATED_FILES)

libpsr_la_LIBADD = $(top_builddir)/services/rde/rda/librda.la

$(MIBLIB_GENERATED_FILES): miblib

miblib:
	@for file in $(MIBLIB_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBDIRS); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmiblib \
			-d $(top_builddir)/services \
			NCS-PSR-MIB; \
		fi \
	done

CLEANFILES = \
	$(MIBLIB_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB

clean-local:
	-rm -rf $(top_builddir)/services/mab/psr/pss_miblib_gen
