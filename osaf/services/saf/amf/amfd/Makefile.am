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

SUBDIRS = include scripts

osaf_execbindir = $(pkglibdir)
osaf_execbin_PROGRAMS = osafamfd

osafamfd_CXXFLAGS =$(AM_CXXFLAGS)

osafamfd_CPPFLAGS = \
	-DSA_CLM_B01=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/amf/include \
	-I$(top_srcdir)/osaf/services/saf/amf/amfd/include \
	-I$(top_srcdir)/osaf/libs/common/immsv/include \
	-I$(top_srcdir)/osaf/tools/saflog/include

osafamfd_SOURCES = \
	$(top_srcdir)/osaf/tools/safimm/src/immutil.c \
	$(top_srcdir)/osaf/tools/saflog/src/saflog.c \
	amfd_main.cc \
	avd_app.cc \
	avd_apptype.cc \
	avd.cc \
	avd_chkop.cc \
	avd_ckpt_dec.cc \
	avd_ckpt_edu.cc \
	avd_ckpt_enc.cc \
	avd_ckpt_updt.cc \
	avd_cluster.cc \
	avd_clm.cc \
	avd_comp.cc \
	avd_compcstype.cc \
	avd_comptype.cc \
	avd_csi.cc \
	avd_csiattr.cc \
	avd_cstype.cc \
	avd_ctcstype.cc \
	avd_dmsg.cc \
	avd_hlt.cc \
	avd_hlttype.cc \
	avd_imm.cc \
	avd_mds.cc \
	avd_ndfsm.cc \
	avd_ndmsg.cc \
	avd_ndproc.cc \
	avd_node.cc \
	avd_nodegroup.cc \
	avd_nodeswbundle.cc \
	avd_pg.cc \
	avd_proc.cc \
	avd_role.cc \
	avd_sg2Nfsm.cc \
	avd_sg.cc \
	avd_sgtype.cc \
	avd_sgNAcvRfsm.cc \
	avd_sgNoRfsm.cc \
	avd_sgNpMfsm.cc \
	avd_sgNWayfsm.cc \
	avd_sgproc.cc \
	avd_si.cc \
	avd_siDep.cc \
	avd_su.cc \
	avd_siass.cc \
	avd_sirankedsu.cc \
	avd_sutcomptype.cc \
	avd_sutype.cc \
	avd_svctype.cc \
	avd_svctypecstypes.cc \
	avd_tmr.cc \
	avd_ntf.cc \
	avd_util.cc

osafamfd_LDADD = \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/common/amf/libavsv_common.la \
	$(top_builddir)/osaf/libs/saf/libSaNtf/libSaNtf.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOi.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOm.la \
	$(top_builddir)/osaf/libs/saf/libSaClm/libSaClm.la \
	$(top_builddir)/osaf/libs/saf/libSaLog/libSaLog.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/rda/librda.la

