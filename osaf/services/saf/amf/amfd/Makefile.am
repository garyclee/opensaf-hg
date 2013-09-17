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
	app.cc \
	apptype.cc \
	chkop.cc \
	cluster.cc \
	clm.cc \
	comp.cc \
	compcstype.cc \
	comptype.cc \
	csiattr.cc \
	csi.cc \
	cstype.cc \
	ctcstype.cc \
	dmsg.cc \
	hlt.cc \
	hlttype.cc \
	imm.cc \
	main.cc \
	mbcsv_dec.cc \
	mbcsv_edu.cc \
	mbcsv_enc.cc \
	mbcsv_updt.cc \
	mds.cc \
	ndfsm.cc \
	ndmsg.cc \
	ndproc.cc \
	node.cc \
	nodegroup.cc \
	nodeswbundle.cc \
	ntf.cc \
	pg.cc \
	role.cc \
	sg_2n_fsm.cc \
	sg.cc \
	sg_nored_fsm.cc \
	sg_npm_fsm.cc \
	sg_nwayact_fsm.cc \
	sg_nway_fsm.cc \
	sgproc.cc \
	sgtype.cc \
	siass.cc \
	si.cc \
	si_dep.cc \
	su.cc \
	sirankedsu.cc \
	sutcomptype.cc \
	sutype.cc \
	svctype.cc \
	svctypecstypes.cc \
	timer.cc \
	util.cc

osafamfd_LDADD = \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/common/amf/libamf_common.la \
	$(top_builddir)/osaf/libs/saf/libSaNtf/libSaNtf.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOi.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOm.la \
	$(top_builddir)/osaf/libs/saf/libSaClm/libSaClm.la \
	$(top_builddir)/osaf/libs/saf/libSaLog/libSaLog.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/rda/librda.la

