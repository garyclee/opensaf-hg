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

noinst_LTLIBRARIES = libdts.la

MIBLIB_GENERATED_FILES = \
	$(top_builddir)/services/dtsv/dts/dts_miblib_gen/ncsdtsvnodelogpolicyentry_mib.c \
	$(top_builddir)/services/dtsv/dts/dts_miblib_gen/ncsdtsvservicelogpolicyentry_mib.c \
	$(top_builddir)/services/dtsv/dts/dts_miblib_gen/ncsdtsvscalars_mib.c

libdts_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_DTS=1 -DDTS_LOG=1 -DNCS_DTA=1 -DDTS_FLOW=1 \
	-I $(top_srcdir)/services/dtsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/include \
	-I $(top_srcdir)/services/common/inc

libdts_la_LDFLAGS = -static -Wl,--export-dynamic

libdts_la_SOURCES = \
	dts_amf.c \
	dts_api.c \
	dts_cbop.c \
	dts_chkop.c \
	dts_ckpt_dec.c \
	dts_ckpt_edu.c \
	dts_ckpt_enc.c \
	dts_ckpt_updt.c \
	dts_def.c \
	dts_dl_api.c \
	dts_glpolc.c \
	dts_log.c \
	dts_mds.c \
	dts_mib.c \
	dts_nodepolc.c \
	dts_pvt.c \
	dts_seq.c \
	dts_svcpolc.c \
	dts_tgt.c \
	sysf_log.c

nodist_libdts_la_SOURCES = $(MIBLIB_GENERATED_FILES)

libdts_la_LIBADD = $(top_builddir)/services/rde/rda/librda.la

$(MIBLIB_GENERATED_FILES): miblib

miblib:
	@for file in $(MIBLIB_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBDIRS); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmiblib \
			-d $(top_builddir)/services \
			NCS-DTSV-MIB; \
		fi \
	done

CLEANFILES = \
	$(MIBLIB_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB

clean-local:
	-rm -rf $(top_builddir)/services/dtsv/dts/dts_miblib_gen
