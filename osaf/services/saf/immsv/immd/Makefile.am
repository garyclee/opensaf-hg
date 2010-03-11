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
	immd_cb.h \
	immd.h \
	immd_mds.h \
	immd_proc.h \
	immd_red.h \
	immd_sbedu.h

pkglib_PROGRAMS = osafimmd

osafimmd_CPPFLAGS = \
	-DSA_CLM_B01=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/immsv/include

osafimmd_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c \
	immd_amf.c \
	immd_db.c \
	immd_evt.c \
	immd_main.c \
	immd_mbcsv.c \
	immd_mds.c \
	immd_proc.c \
	immd_sbevt.c

osafimmd_LDADD = \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/common/avsv/libavsv_common.la \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la \
	$(top_builddir)/osaf/libs/common/immsv/libimmsv_common.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/rda/librda.la
