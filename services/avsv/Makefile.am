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

CONTROLLER_SUBDIRS = avd avm logstr clicmds pssv_integ snmp_integ

SUBDIRS = avnd $(CONTROLLER_SUBDIRS)

noinst_HEADERS = \
   inc/ava_cb.h \
   inc/ava_def.h \
   inc/ava_dl_api.h \
   inc/ava.h \
   inc/ava_hdl.h \
   inc/ava_log.h \
   inc/ava_logstr.h \
   inc/ava_mds.h \
   inc/ava_mem.h \
   inc/avd_avm.h \
   inc/avd_avnd.h \
   inc/avd_bam.h \
   inc/avd_cb.h \
   inc/avd_ckp.h \
   inc/avd_ckpt_edu.h \
   inc/avd_ckpt_msg.h \
   inc/avd_ckpt_updt.h \
   inc/avd_comp.h \
   inc/avd_csi.h \
   inc/avd_dblog.h \
   inc/avd_def.h \
   inc/avd_dl_api.h \
   inc/avd_evt.h \
   inc/avd_fail.h \
   inc/avd.h \
   inc/avd_hb.h \
   inc/avd_hlt.h \
   inc/avd_logstr.h \
   inc/avd_mds.h \
   inc/avd_mem.h \
   inc/avd_mibtbl.h \
   inc/avd_msg.h \
   inc/avd_pg.h \
   inc/avd_proc.h \
   inc/avd_sg.h \
   inc/avd_si.h \
   inc/avd_su.h \
   inc/avd_susi.h \
   inc/avd_tmr.h \
   inc/avd_trap.h \
   inc/avnd_cb.h \
   inc/avnd_clm.h \
   inc/avnd_comp.h \
   inc/avnd_def.h \
   inc/avnd_defs.h \
   inc/avnd_di.h \
   inc/avnd_dl_api.h \
   inc/avnd_err.h \
   inc/avnd_evt.h \
   inc/avnd.h \
   inc/avnd_hc.h \
   inc/avnd_log.h \
   inc/avnd_logstr.h \
   inc/avnd_mds.h \
   inc/avnd_mem.h \
   inc/avnd_mib.h \
   inc/avnd_pg.h \
   inc/avnd_proc.h \
   inc/avnd_srm.h \
   inc/avnd_su.h \
   inc/avnd_tmr.h \
   inc/avnd_trap.h \
   inc/avnd_util.h \
   inc/avsv_amfparam.h \
   inc/avsv_clmparam.h \
   inc/avsv_d2nedu.h \
   inc/avsv_d2nmem.h \
   inc/avsv_d2nmsg.h \
   inc/avsv_defs.h \
   inc/avsv_eduutil.h \
   inc/avsv.h \
   inc/avsv_ipc.h \
   inc/avsv_log.h \
   inc/avsv_logstr.h \
   inc/avsv_n2avaedu.h \
   inc/avsv_n2avamem.h \
   inc/avsv_n2avamsg.h \
   inc/avsv_n2claedu.h \
   inc/avsv_n2clamem.h \
   inc/avsv_n2clamsg.h \
   inc/avsv_util.h \
   inc/cla_cb.h \
   inc/cla_def.h \
   inc/cla_dl_api.h \
   inc/cla.h \
   inc/cla_hdl.h \
   inc/cla_log.h \
   inc/cla_logstr.h \
   inc/cla_mds.h \
   inc/cla_mem.h \
   inc/avd_si_dep.h \
   inc/avnd_ckpt_edu.h \
   inc/avnd_ckpt_msg.h \
   inc/avnd_ckpt_updt.h \
   inc/avnd_mbcsv.h \
   inc/avnd_proxy.h \
   inc/avsv_nd2ndmsg.h 

pkglib_PROGRAMS = ncs_scap ncs_pcap

ncs_scap_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_MAC=1 -DNCS_BAM=1 -DNCS_AVD=1 -DNCS_AVND=1 -DNCS_AVM=1 -DNCS_SRMA=1 \
	-I $(top_srcdir)/services/avsv/inc \
	-I $(top_srcdir)/services/avsv/avm/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/vds/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/bam/inc \
	-I $(top_srcdir)/services/srmsv/inc \
	-I $(top_srcdir)/include

ncs_scap_CFLAGS = @HPI_CFLAGS@

ncs_scap_SOURCES = \
	$(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_scap_LDFLAGS = @XERCESC_LIB@

ncs_scap_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaEvt/libSaEvt.la \
	$(top_builddir)/services/common/libsaf_common.la \
	$(top_builddir)/services/avsv/common/libavsv_common.la \
	$(top_builddir)/services/mab/mac/libmaa.la \
	$(top_builddir)/services/srmsv/srma/libsrma.la \
	$(top_builddir)/services/mbcsv/src/libmbca.la \
	$(top_builddir)/services/hisv/hpl/libhpl.la \
	$(top_builddir)/services/avsv/avd/libavd.la \
	$(top_builddir)/services/avsv/avnd/libavnd_ctrl.la \
	$(top_builddir)/services/avsv/avm/libavm.la \
	$(top_builddir)/services/bam/libbam.la

ncs_pcap_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_AVND=1 -DNCS_SRMA=1 \
	-I $(top_srcdir)/services/avsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/vds/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/srmsv/inc \
	-I $(top_srcdir)/include

ncs_pcap_SOURCES = \
	$(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_pcap_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaEvt/libSaEvt.la \
	$(top_builddir)/services/common/libsaf_common.la \
	$(top_builddir)/services/avsv/common/libavsv_common.la \
	$(top_builddir)/services/srmsv/srma/libsrma.la \
	$(top_builddir)/services/avsv/avnd/libavnd_pld.la
