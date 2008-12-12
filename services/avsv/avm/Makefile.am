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
	$(top_builddir)/services/avsv/avm/src/avm_miblib_gen/ncsavmentdeploytableentry_mib.c \
	$(top_builddir)/services/avsv/avm/src/avm_miblib_gen/ncsavmentfaultdomaintableentry_mib.c \
	$(top_builddir)/services/avsv/avm/src/avm_miblib_gen/ncsavmentupgradetableentry_mib.c \
	$(top_builddir)/services/avsv/avm/src/avm_miblib_gen/ncsavmscalars_mib.c

libavm_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	@HPI_DEF@ \
	-DNCS_AVM=1 -DNCS_AVM_LOG=1 \
	-I $(top_srcdir)/services/avsv/inc \
	-I $(top_builddir)/services/avsv/inc/avsv_snmp_inc \
	-I $(top_srcdir)/services/avsv/avm/inc \
	-I $(top_builddir)/services/avsv/avm/inc/avm_snmp_inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/hisv/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

libavm_la_CFLAGS = @HPI_CFLAGS@

libavm_la_LDFLAGS = -static

libavm_la_SOURCES = \
	src/avm_avd.c \
	src/avm_chkop.c \
	src/avm_ckpt_dec.c \
	src/avm_ckpt_edu.c \
	src/avm_ckpt_enc.c \
	src/avm_ckpt_updt.c \
	src/avm_db.c \
	src/avm_dblog.c \
	src/avm_dhcp_util.c \
	src/avm_eda.c \
	src/avm_ent.c \
	src/avm_evt.c \
	src/avm_fsm.c \
	src/avm_fund.c \
	src/avm_hpi_util.c \
	src/avm_init.c \
	src/avm_lfm.c \
	src/avm_mab.c \
	src/avm_mds.c \
	src/avm_proc.c \
	src/avm_rda.c \
	src/avm_role_evt.c \
	src/avm_scalar.c \
	src/avm_tmr.c \
	src/avm_traps.c \
	src/avm_upgd_traps.c \
	src/avm_util.c

nodist_libavm_la_SOURCES = $(MIBLIB_GENERATED_FILES)

if MOT_ATCA

libavm_la_LIBADD = \
	$(top_builddir)/services/rde/rda/librda.la \
	$(top_builddir)/services/avsv/avm/liblfm_avm_intf.la

else

libavm_la_LIBADD = \
	$(top_builddir)/services/rde/rda/librda.la

endif

if MOT_ATCA

lib_LTLIBRARIES = liblfm_avm_intf.la

liblfm_avm_intf_la_CPPFLAGS =
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_AVM=1 -DNCS_AVM_LOG=1 \
	-I $(top_srcdir)/services/avsv/inc \
	-I $(top_builddir)/services/avsv/inc/avsv_snmp_inc \
	-I $(top_srcdir)/services/avsv/avm/inc \
	-I $(top_builddir)/services/avsv/avm/inc/avm_snmp_inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/hisv/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

liblfm_avm_intf_la_SOURCES = \
	src/lfm_avm_intf.c

liblfm_avm_intf_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@

endif

$(MIBLIB_GENERATED_FILES): miblib

miblib:
	@for file in $(MIBLIB_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBSDIR); \
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
	-rm -rf $(top_builddir)/services/avsv/avm/src/avm_miblib_gen
