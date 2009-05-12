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
#

MAINTAINERCLEANFILES = Makefile.in

noinst_HEADERS = \
	inc/ntfs_com.h \
	inc/ntfs_cb.h \
	inc/ntfs_evt.h \
	inc/ntfs.h \
	inc/ntfs_mbcsv.h \
	inc/NtfAdmin.hh \
	inc/NtfClient.hh \
	inc/NtfFilter.hh \
	inc/NtfLogger.hh \
	inc/NtfNotification.hh \
	inc/NtfReader.hh \
	inc/NtfSubscription.hh

pkglib_PROGRAMS = opensaf_ntfd

opensaf_ntfd_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_MAIN_EXTERNAL \
	-I $(top_srcdir)/services/ntfsv/inc/ \
	-I $(top_srcdir)/services/leap/inc/ \
	-I $(top_srcdir)/services/mds/inc/ \
	-I $(top_srcdir)/services/mab/inc/ \
	-I $(top_srcdir)/services/vds/inc/ \
	-I $(top_srcdir)/services/common/inc/ \
	-I $(top_srcdir)/include/ \
	-I $(top_srcdir)/services/ntfsv/ntfs/inc

opensaf_ntfd_SOURCES = \
	$(top_srcdir)/services/common/src/ncs_main_pvt.c \
	ntfs_amf.c \
	ntfs_evt.c \
	ntfs_mbcsv.c \
	ntfs_main.c \
	$(top_srcdir)/services/ntfsv/common/ntfsv_enc_dec.c \
	$(top_srcdir)/services/ntfsv/common/ntfsv_mem.c \
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
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/lib/lib_SaLog/libSaLog.la \
	$(top_builddir)/services/mbcsv/src/libmbca.la
