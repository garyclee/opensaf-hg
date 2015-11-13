#      -*- OpenSAF  -*-
#
# (C) Copyright 2015 The OpenSAF Foundation
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
# Author(s): Ericsson AB
#

include $(top_srcdir)/Makefile.common

TESTS = testamfd

check_PROGRAMS = testamfd

testamfd_CXXFLAGS =$(AM_CXXFLAGS)

testamfd_CPPFLAGS = \
	-DSA_CLM_B01=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/amf/include \
	-I$(top_srcdir)/osaf/services/saf/amf/amfd \
	-I$(top_srcdir)/osaf/services/saf/amf/amfd/include \
	-I$(top_srcdir)/osaf/libs/common/immsv/include \
	-I$(top_srcdir)/osaf/tools/saflog/include \
	-I$(GTEST_DIR)/include

testamfd_LDFLAGS = \
	-lpthread \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-app.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-apptype.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-chkop.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-ckpt_dec.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-ckpt_edu.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-ckpt_enc.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-ckpt_updt.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-clm.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-cluster.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-compcstype.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-comp.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-comptype.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-csiattr.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-csi.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-cstype.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-ctcstype.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-dmsg.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-hlt.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-hlttype.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-imm.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-mds.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-ndfsm.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-ndmsg.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-ndproc.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-nodegroup.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-node.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-nodeswbundle.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-ntf.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-pg.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-role.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-sg_2n_fsm.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-sg_nored_fsm.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-sg_npm_fsm.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-sg_nwayact_fsm.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-sg_nway_fsm.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-sg.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-sgproc.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-sgtype.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-siass.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-si_dep.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-si.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-sirankedsu.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-su.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-sutcomptype.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-sutype.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-svctypecstypes.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-svctype.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-timer.o \
	$(top_builddir)/osaf/services/saf/amf/amfd/osafamfd-util.o

testamfd_SOURCES = \
	test_amfdb.cc \
	test_ckpt_enc_dec.cc

testamfd_LDADD = \
	$(GTEST_DIR)/lib/libgtest.la \
	$(GTEST_DIR)/lib/libgtest_main.la \
	$(top_builddir)/osaf/tools/saflog/src/libsaflog.la \
	$(top_builddir)/osaf/tools/safimm/src/libimmutil.la \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/common/amf/libamf_common.la \
	$(top_builddir)/osaf/libs/saf/libSaNtf/libSaNtf.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOi.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOm.la \
	$(top_builddir)/osaf/libs/saf/libSaClm/libSaClm.la \
	$(top_builddir)/osaf/libs/saf/libSaLog/libSaLog.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/rda/librda.la
