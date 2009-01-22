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

noinst_LTLIBRARIES = libavnd_ctrl.la libavnd_pld.la

MIBLIB_GENERATED_FILES = \
	$(top_builddir)/services/avsv/avnd/avnd_miblib_gen/ncsscomptableentry_mib.c \
	$(top_builddir)/services/avsv/avnd/avnd_miblib_gen/ncssndtableentry_mib.c \
	$(top_builddir)/services/avsv/avnd/avnd_miblib_gen/ncsssutableentry_mib.c \
	$(top_builddir)/services/avsv/avnd/avnd_miblib_gen/saamfscompcsitableentry_mib.c

COMMON_SRC = \
	avnd.c \
	avnd_cam.c \
	avnd_cbq.c \
	avnd_chc.c \
	avnd_clc.c \
	avnd_clm.c \
	avnd_clmdb.c \
	avnd_comp.c \
	avnd_compdb.c \
	avnd_cpm.c \
	avnd_di.c \
	avnd_err.c \
	avnd_evt.c \
	avnd_evtlog.c \
	avnd_hcdb.c \
	avnd_hpi.c \
	avnd_log.c \
	avnd_mab.c \
	avnd_mds.c \
	avnd_pg.c \
	avnd_pgdb.c \
	avnd_proc.c \
	avnd_sidb.c \
	avnd_srm.c \
	avnd_su.c \
	avnd_sudb.c \
	avnd_susm.c \
	avnd_term.c \
	avnd_tmr.c \
	avnd_trap.c \
	avnd_util.c \
	avnd_verify.c \
	avnd_avnd.c \
	avnd_proxy.c \
	avnd_proxydb.c 

COMMON_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_AVND=1 -DNCS_AVSV_LOG=1 \
	-I $(top_srcdir)/services/avsv/inc \
	-I $(top_builddir)/services/avsv/inc/avsv_snmp_inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

libavnd_ctrl_la_CPPFLAGS = \
	$(COMMON_CPPFLAGS) \
	-DNCS_AVND_MBCSV_CKPT=1

libavnd_ctrl_la_LDFLAGS = -static

libavnd_ctrl_la_SOURCES = \
	$(COMMON_SRC) \
	avnd_mbcsv.c \
	avnd_ckpt_dec.c \
	avnd_ckpt_edu.c \
	avnd_ckpt_enc.c \
	avnd_ckpt_updt.c 

nodist_libavnd_ctrl_la_SOURCES = $(MIBLIB_GENERATED_FILES)

# For payload 

libavnd_pld_la_CPPFLAGS = $(COMMON_CPPFLAGS) 

libavnd_pld_la_LDFLAGS = -static

libavnd_pld_la_SOURCES = $(COMMON_SRC)

nodist_libavnd_pld_la_SOURCES = $(MIBLIB_GENERATED_FILES)

$(MIBLIB_GENERATED_FILES): miblib

miblib:
	@for file in $(MIBLIB_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBSDIR); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmiblib \
			-d $(top_builddir)/services \
			SAF-AMF-MIB SAF-CLM-MIB NCS-AVSV-MIB; \
		fi \
	done

CLEANFILES = \
	$(MIBLIB_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB

clean-local:
	-rm -rf $(top_builddir)/services/avsv/avnd/avnd_miblib_gen
