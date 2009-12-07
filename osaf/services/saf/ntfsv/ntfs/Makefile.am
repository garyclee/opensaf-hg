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
# Author(s): Ericsson AB
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = scripts

noinst_HEADERS = \
	ntfs_com.h \
	ntfs_cb.h \
	ntfs_evt.h \
	ntfs.h \
	ntfs_mbcsv.h \
	NtfAdmin.hh \
	NtfClient.hh \
	NtfFilter.hh \
	NtfLogger.hh \
	NtfNotification.hh \
	NtfReader.hh \
	NtfSubscription.hh

pkglib_PROGRAMS = opensaf_ntfd

opensaf_ntfd_CPPFLAGS = \
	-DNCS_MAIN_EXTERNAL \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/ntfsv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

opensaf_ntfd_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c \
	ntfs_amf.c \
	ntfs_evt.c \
	ntfs_mbcsv.c \
	ntfs_main.c \
	ntfs_mds.c \
	ntfs_com.c \
	NtfNotification.cc \
	NtfFilter.cc \
	NtfSubscription.cc \
	NtfLogger.cc \
	NtfReader.cc \
	NtfClient.cc \
	NtfAdmin.cc

opensaf_ntfd_LDADD = \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/common/avsv/libavsv_common.la \
	$(top_builddir)/osaf/libs/common/immsv/libimmsv_common.la \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la \
	$(top_builddir)/osaf/libs/saf/libSaLog/libSaLog.la \
	$(top_builddir)/osaf/libs/common/ntfsv/libntfsv_common.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/rda/librda.la 
