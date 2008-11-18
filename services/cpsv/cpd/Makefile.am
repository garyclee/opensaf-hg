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

noinst_LTLIBRARIES = libcpd.la

MIBLIB_GENERATED_FILES = \
	$(top_builddir)/services/cpsv/cpd/miblib_gen/sackptcheckpointentry_mib.c \
	$(top_builddir)/services/cpsv/cpd/miblib_gen/safckptscalarobject_mib.c \
	$(top_builddir)/services/cpsv/cpd/miblib_gen/sackptnodereplicalocentry_mib.c

libcpd_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_CPD=1 -DNCS_CPSV_LOG=1 \
	-I $(top_srcdir)/services/cpsv/inc \
	-I $(top_builddir)/services/cpsv/inc/snmp_inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

libcpd_la_LDFLAGS = -static

libcpd_la_SOURCES = \
	cpd_amf.c \
	cpd_ckptentry.c \
	cpd_ckptscalar.c \
	cpd_db.c \
	cpd_evt.c \
	cpd_init.c \
	cpd_log.c \
	cpd_mbcsv.c \
	cpd_mds.c \
	cpd_mib.c \
	cpd_proc.c \
	cpd_red.c \
	cpd_reploc.c \
	cpd_sbevt.c \
	cpd_tmr.c

nodist_libcpd_la_SOURCES = $(MIBLIB_GENERATED_FILES)

$(MIBLIB_GENERATED_FILES): miblib

miblib:
	for file in $(MIBLIB_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBSDIR); \
			export PATH=$(top_srcdir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmiblib \
			-d $(top_builddir)/services \
			SAF-CKPT-SVC-v7_5; \
		fi \
	done

CLEANFILES = \
	$(MIBLIB_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB

clean-local:
	-rm -rf $(top_builddir)/services/cpsv/cpd/miblib_gen
