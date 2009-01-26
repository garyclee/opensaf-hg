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

noinst_LTLIBRARIES = libavd.la

MIBLIB_GENERATED_FILES = \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/avsvscalars_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/ncsndtableentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/ncssgtableentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/ncssitableentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/ncssutableentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/saamfcompcstypesupportedtableentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/saamfcomptableentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/saamfcsinamevaluetableentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/saamfcsitableentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/saamfcstypeparamentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/saamfhealthchecktableentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/saamfnodetableentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/saamfscalars_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/saamfsgsirankentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/saamfsgsurankentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/saamfsgtableentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/saamfsisideptableentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/saamfsitableentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/saamfsusitableentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/saamfsuspersirankentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/saamfsutableentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/saclmnodetableentry_mib.c \
	$(top_builddir)/services/avsv/avd/avd_miblib_gen/safclmscalarobject_mib.c

libavd_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_AVD=1 -DNCS_AVSV_LOG=1 \
	-I $(top_srcdir)/services/avsv/inc \
	-I $(top_builddir)/services/avsv/inc/avsv_snmp_inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

libavd_la_LDFLAGS = -static

libavd_la_SOURCES = \
	avd_avm.c \
	avd_avmmsg.c \
	avd_avnd.c \
	avd_bam.c \
	avd_bammsg.c \
	avd.c \
	avd_chkmsg.c \
	avd_chkop.c \
	avd_ckpt_dec.c \
	avd_ckpt_edu.c \
	avd_ckpt_enc.c \
	avd_ckpt_updt.c \
	avd_comp.c \
	avd_csi.c \
	avd_dblog.c \
	avd_dmsg.c \
	avd_evtlog.c \
	avd_hb.c \
	avd_hlt.c \
	avd_hpi.c \
	avd_mab.c \
	avd_mds.c \
	avd_mibtbl.c \
	avd_ndfsm.c \
	avd_ndmsg.c \
	avd_ndproc.c \
	avd_pg.c \
	avd_proc.c \
	avd_role.c \
	avd_sg2Nfsm.c \
	avd_sg.c \
	avd_sgNAcvRfsm.c \
	avd_sgNoRfsm.c \
	avd_sgNpMfsm.c \
	avd_sgNWayfsm.c \
	avd_sgproc.c \
	avd_si.c \
	avd_siDep.c \
	avd_su.c \
	avd_susi.c \
	avd_tmr.c \
	avd_trap.c \
	avd_utilmsg.c

nodist_libavd_la_SOURCES = $(MIBLIB_GENERATED_FILES)

$(MIBLIB_GENERATED_FILES): miblib

miblib:
	@for file in $(MIBLIB_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBSDIR); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmiblib \
			-d $(top_builddir)/services \
			IFMIB; \
		fi \
	done

CLEANFILES = \
	$(MIBLIB_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB

clean-local:
	-rm -rf $(top_builddir)/services/avsv/avd/avd_miblib_gen
