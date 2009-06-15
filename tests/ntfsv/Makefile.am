#      -*- OpenSAF  -*-
#
# (C) Copyright 2009 The OpenSAF Foundation
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under High-Availability Operating 
# Environment Software License Version 1.4.
# Complete License can be accesseble from below location.
# http://www.opensaf.org/license 
# See the Copying file included with the OpenSAF distribution for
# full licensing terms.
#
# Author(s): Ericsson AB
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = ntftest

ntftest_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/common/inc \
	-I$(top_srcdir)/tests/immsv/common \
	-I$(top_srcdir)/tests/unit_test_fw/inc

ntftest_SOURCES = \
	$(top_srcdir)/services/common/src/logtrace.c \
	$(top_srcdir)/tests/ntfsv/src/tet_ntf_main.c \
	$(top_srcdir)/tests/ntfsv/src/sa_error.c \
	$(top_srcdir)/tests/ntfsv/src/tet_ntf_common.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfInitialize.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfSelectionObjectGet.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfDispatch.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfFinalize.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfNotificationCallbackT.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfNotificationDiscardedCallbackT.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfMiscellaneousNotificationAllocate.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfAlarmNotificationAllocate.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfAttributeChangeNotificationAllocate.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfStateChangeNotificationAllocate.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfSecurityAlarmNotificationAllocate.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfNotificationSend.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfNotificationSubscribe.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfStateChangeNotificationFilterAllocate.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfSecurityAlarmNotificationFilterAllocate.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfObjectCreateDeleteNotificationFilterAllocate.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfAttributeChangeNotificationFilterAllocate.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfAlarmNotificationFilterAllocate.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfNotificationUnsubscribe.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfObjectCreateDeleteNotificationAllocate.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfNotificationReadInitialize.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfNotificationReadFinalize.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfNotificationReadNext.c \
	$(top_srcdir)/tests/ntfsv/src/tet_NotificationContentVerification.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfArrayValAllocate.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfPtrValAllocate.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfArrayValGet.c \
	$(top_srcdir)/tests/ntfsv/src/tet_saNtfPtrValGet.c \
	$(top_srcdir)/tests/ntfsv/src/test.c \
	$(top_srcdir)/tests/ntfsv/src/util.c

ntftest_LDADD = \
	$(top_builddir)/lib/lib_SaNtf/libSaNtf.la
