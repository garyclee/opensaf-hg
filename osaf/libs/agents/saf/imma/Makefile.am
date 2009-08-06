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

noinst_HEADERS = \
	imma_cb.h \
	imma_def.h \
	imma.h \
	imma_mds.h \
	imma_proc.h

noinst_LTLIBRARIES = libimmaOm.la libimmaOi.la

libimmaOm_la_CPPFLAGS = \
	-DIMMA_OM \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/immsv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

libimmaOm_la_LDFLAGS = -static

libimmaOm_la_SOURCES = \
	imma_om_api.c \
	imma_db.c \
	imma_init.c \
	imma_mds.c \
	imma_proc.c

libimmaOi_la_CPPFLAGS = \
	-DIMMA_OI \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/immsv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

libimmaOi_la_LDFLAGS = -static

libimmaOi_la_SOURCES = \
	imma_oi_api.c \
	imma_db.c \
	imma_init.c \
	imma_mds.c \
	imma_proc.c
