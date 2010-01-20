#      OpenSAF
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
#            Ericsson AB
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = scripts

noinst_HEADERS = \
	inc/smfd_cb.h \
	inc/smfd_evt.h \
	inc/smfd_smfnd.h \
	inc/smfd.h \
	inc/SmfCampaignXmlParser.hh \
	inc/SmfUpgradeCampaign.hh \
	inc/SmfUpgradeProcedure.hh \
	inc/SmfUpgradeAction.hh \
	inc/SmfImmOperation.hh \
	inc/SmfCampaignThread.hh \
	inc/SmfProcedureThread.hh \
	inc/SmfCampaign.hh \
	inc/SmfCampState.hh \
	inc/SmfTargetTemplate.hh \
	inc/SmfProcState.hh \
	inc/SmfUpgradeStep.hh \
	inc/SmfStepState.hh \
	inc/SmfUpgradeMethod.hh \
	inc/SmfUtils.hh \
	inc/SmfCampaignWrapup.hh \
	inc/SmfCampaignInit.hh

pkglib_PROGRAMS = opensaf_smfd

opensaf_smfd_CXXFLAGS = $(AM_CXXFLAGS) @XML2_CFLAGS@

opensaf_smfd_CPPFLAGS = \
	-DNCS_MAIN_EXTERNAL \
	$(AM_CPPFLAGS) \
	-I $(top_srcdir)/osaf/libs/core/leap/include \
	-I $(top_srcdir)/osaf/libs/core/mds/include \
	-I $(top_srcdir)/osaf/libs/core/common/include \
	-I $(top_srcdir)/osaf/services/saf/smfsv/inc \
	-I $(top_srcdir)/osaf/services/saf/smfsv/smfd/inc \
	-I $(top_srcdir)/osaf/services/infrastructure/masv/include \
	-I $(top_srcdir)/osaf/libs/common/immsv/include

opensaf_smfd_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c \
	$(top_srcdir)/osaf/tools/safimm/src/immutil.c \
	smfd_amf.c \
	smfd_evt.c \
	smfd_main.c \
	smfd_smfnd.c \
	smfd_mds.c \
	SmfCampaignThread.cc \
	SmfProcedureThread.cc \
	SmfCampaign.cc \
	smfd_campaign_oi.cc \
	SmfCampaignXmlParser.cc \
	SmfUpgradeCampaign.cc \
	SmfUpgradeProcedure.cc \
	SmfCampState.cc \
	SmfProcState.cc \
	SmfImmOperation.cc \
	SmfTargetTemplate.cc \
	SmfUpgradeAction.cc \
	SmfUpgradeStep.cc \
	SmfStepState.cc \
	SmfUpgradeMethod.cc \
	SmfUtils.cc \
	SmfCampaignInit.cc \
	SmfCampaignWrapup.cc

opensaf_smfd_LDFLAGS = @XML2_LIBS@

opensaf_smfd_LDADD = \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la \
	$(top_builddir)/osaf/libs/saf/libSaClm/libSaClm.la \
	$(top_builddir)/osaf/libs/saf/libSaNtf/libSaNtf.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOi.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOm.la \
	$(top_builddir)/osaf/libs/common/avsv/libavsv_common.la \
	$(top_builddir)/osaf/libs/common/immsv/libimmsv_common.la \
	$(top_builddir)/osaf/libs/common/ntfsv/libntfsv_common.la \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/common/smfsv/libsmfsv_common.la
