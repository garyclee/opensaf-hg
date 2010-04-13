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
# Author(s): Emerson Network Power
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = logstr

pkglib_LTLIBRARIES = libfma.la

noinst_HEADERS = \
	fma_cb.h \
	fma_hdl.h \
	fma_mem.h \
	fma_log.h \
	fma.h \
	fma_mds.h

libfma_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/fm/include

libfma_la_CFLAGS = $(AM_CFLAGS)

libfma_la_SOURCES = \
	$(top_srcdir)/osaf/services/infrastructure/fm/fma_fm_intf.c \
	fma_api.c \
	fma_hdl.c \
	fma_init.c \
	fma_log.c \
	fma_mds.c

libfma_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@
