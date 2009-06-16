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

SUBDIRS = logstr pssv_integ snmp_integ

noinst_LTLIBRARIES = libavm.la

noinst_HEADERS = \
	inc/avm_avd_api.h \
	inc/avm_ckp.h \
	inc/avm_ckpt_msg.h \
	inc/avm_db.h \
	inc/avm_defs.h \
	inc/avm_dl_api.h \
	inc/avm_fsm.h \
	inc/avm.h \
	inc/avm_logstr.h \
	inc/avm_mem.h \
	inc/avm_util.h \
	inc/avm_cb.h \
	inc/avm_ckpt_edu.h \
	inc/avm_ckpt_updt.h \
	inc/avm_dblog.h \
	inc/avm_dhcp_conf.h \
	inc/avm_evt.h \
	inc/avm_fund.h \
	inc/avm_hpi_util.h \
	inc/avm_mds.h \
	inc/avm_tmr.h

MIBLIB_GENERATED_FILES = \
	$(top_builddir)/services/avsv/avm/avm_miblib_gen/ncsavmentdeploytableentry_mib.c \
	$(top_builddir)/services/avsv/avm/avm_miblib_gen/ncsavmentfaultdomaintableentry_mib.c \
	$(top_builddir)/services/avsv/avm/avm_miblib_gen/ncsavmentupgradetableentry_mib.c \
	$(top_builddir)/services/avsv/avm/avm_miblib_gen/ncsavmscalars_mib.c

libavm_la_CPPFLAGS = \
	-DNCS_AVM=1 -DNCS_AVM_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_builddir)/services/avsv/inc/avsv_snmp_inc \
	-I$(top_builddir)/services/avsv/avm/inc/avm_snmp_inc \
	-I$(top_srcdir)/services/avsv/inc \
	-I$(top_srcdir)/services/avsv/avm/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/fm/fma/inc \
	-I$(top_srcdir)/services/hisv/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/common/inc

libavm_la_CFLAGS = $(AM_CFLAGS) @HPI_CFLAGS@

libavm_la_LDFLAGS = -static

libavm_la_LIBADD = \
	$(top_srcdir)/services/rde/rda/librda.la \
	$(top_srcdir)/services/fm/fma/libfma.la

libavm_la_SOURCES = \
	avm_avd.c \
	avm_chkop.c \
	avm_ckpt_dec.c \
	avm_ckpt_edu.c \
	avm_ckpt_enc.c \
	avm_ckpt_updt.c \
	avm_db.c \
	avm_dblog.c \
	avm_dhcp_util.c \
	avm_eda.c \
	avm_ent.c \
	avm_evt.c \
	avm_fsm.c \
	avm_fund.c \
	avm_hpi_util.c \
	avm_init.c \
	avm_mab.c \
	avm_mds.c \
	avm_proc.c \
	avm_rda.c \
	avm_fma.c \
	avm_role_evt.c \
	avm_scalar.c \
	avm_tmr.c \
	avm_traps.c \
	avm_upgd_traps.c \
	avm_util.c

nodist_libavm_la_SOURCES = $(MIBLIB_GENERATED_FILES)


$(MIBLIB_GENERATED_FILES): miblib

miblib:
	@for file in $(MIBLIB_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBDIRS); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmiblib \
			-d $(top_builddir)/services \
			NCS-AVM-MIB; \
		fi \
	done

CLEANFILES = \
	$(MIBLIB_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB

clean-local:
	-rm -rf $(top_builddir)/services/avsv/avm/avm_miblib_gen
