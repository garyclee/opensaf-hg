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

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = immoitest

immoitest_CPPFLAGS = \
	-I $(top_srcdir)/include \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/tests/immsv/common \
	-I $(top_srcdir)/tests/unit_test_fw/inc

immoitest_SOURCES = \
	$(top_srcdir)/tests/unit_test_fw/src/util.c \
	$(top_srcdir)/tests/unit_test_fw/src/utest.c \
	$(top_srcdir)/tests/immsv/common/immtest.c \
	$(top_srcdir)/tests/immsv/implementer/test_saImmOiInitialize_2.c \
	$(top_srcdir)/tests/immsv/implementer/test_saImmOiSelectionObjectGet.c \
	$(top_srcdir)/tests/immsv/implementer/test_saImmOiDispatch.c \
	$(top_srcdir)/tests/immsv/implementer/test_saImmOiFinalize.c \
	$(top_srcdir)/tests/immsv/implementer/test_saImmOiImplementerSet.c \
	$(top_srcdir)/tests/immsv/implementer/test_saImmOiImplementerClear.c \
	$(top_srcdir)/tests/immsv/implementer/test_saImmOiClassImplementerSet.c \
	$(top_srcdir)/tests/immsv/implementer/test_saImmOiClassImplementerRelease.c \
	$(top_srcdir)/tests/immsv/implementer/test_saImmOiObjectImplementerSet.c \
	$(top_srcdir)/tests/immsv/implementer/test_saImmOiObjectImplementerRelease.c \
	$(top_srcdir)/tests/immsv/implementer/test_saImmOiRtObjectCreate_2.c \
	$(top_srcdir)/tests/immsv/implementer/test_saImmOiRtObjectDelete.c \
	$(top_srcdir)/tests/immsv/implementer/test_saImmOiRtObjectUpdate_2.c \
	$(top_srcdir)/tests/immsv/implementer/test_SaImmOiRtAttrUpdateCallbackT.c \
	$(top_srcdir)/tests/immsv/implementer/test_SaImmOiCcb.c \
	$(top_srcdir)/tests/immsv/implementer/test_SaImmOiAdminOperation.c

immoitest_LDADD = \
	$(top_builddir)/lib/lib_SaImm/libSaImmOi.la \
	$(top_builddir)/lib/lib_SaImm/libSaImmOm.la
