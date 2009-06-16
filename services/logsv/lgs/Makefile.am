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
#            Ericsson AB
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

noinst_HEADERS = \
	inc/lgs_cb.h \
	inc/lgs_fmt.h \
	inc/lgs_mbcsv.h \
	inc/lgs_util.h \
	inc/lgs_evt.h \
	inc/lgs.h \
	inc/lgs_stream.h

pkglib_PROGRAMS = opensaf_saflogd

opensaf_saflogd_CPPFLAGS = \
	-DNCS_MAIN_EXTERNAL \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/logsv/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/vds/inc \
	-I$(top_srcdir)/services/common/inc \
	-I$(top_srcdir)/services/logsv/lgs/inc \
	-I$(top_srcdir)/services/immsv/inc

opensaf_saflogd_SOURCES = \
	$(top_srcdir)/services/common/ncs_main_pvt.c \
	$(top_srcdir)/services/immsv/utils/immutil.c \
	lgs_amf.c \
	lgs_evt.c \
	lgs_mbcsv.c \
	lgs_fmt.c \
	lgs_imm.c \
	lgs_main.c \
	lgs_mds.c \
	lgs_util.c \
	lgs_stream.c

opensaf_saflogd_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/lib/lib_SaImm/libSaImmOi.la \
	$(top_builddir)/lib/lib_SaImm/libSaImmOm.la \
	$(top_builddir)/services/mbcsv/libmbca.la \
	$(top_builddir)/services/rde/rda/librda.la \
	$(top_builddir)/services/common/libncs_common.la
