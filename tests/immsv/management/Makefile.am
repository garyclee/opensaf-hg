#      -*- OpenSAF  -*-
#
# (C) Copyright 2009 The OpenSAF Foundation
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

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = immomtest

immomtest_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/common/inc \
	-I$(top_srcdir)/tests/immsv/common \
	-I$(top_srcdir)/tests/unit_test_fw/inc

immomtest_SOURCES = \
	$(top_srcdir)/tests/unit_test_fw/src/utest.c \
	$(top_srcdir)/tests/unit_test_fw/src/util.c \
	$(top_srcdir)/tests/immsv/common/immtest.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmInitialize.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmSelectionObjectGet.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmDispatch.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmFinalize.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmAccessorInitialize.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmAccessorGet_2.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmAccessorFinalize.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmClassCreate_2.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmClassDescriptionGet_2.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmClassDescriptionMemoryFree_2.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmClassDelete.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmSearchInitialize_2.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmSearchNext_2.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmSearchFinalize.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmAdminOwnerInitialize.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmAdminOwnerSet.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmAdminOwnerRelease.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmAdminOwnerFinalize.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmAdminOwnerClear.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmCcbInitialize.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmCcbObjectCreate_2.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmCcbObjectDelete.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmCcbObjectModify_2.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmCcbApply.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmCcbFinalize.c \
	$(top_srcdir)/tests/immsv/management/test_saImmOmAdminOperationContinue.c

immomtest_LDADD = \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOm.la
