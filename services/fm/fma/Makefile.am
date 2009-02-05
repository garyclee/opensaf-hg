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

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = logstr

lib_LTLIBRARIES = libfma.la

noinst_HEADERS = \
	$(top_srcdir)/services/fm/fma/inc/fma_cb.h \
	$(top_srcdir)/services/fm/fma/inc/fma_hdl.h \
	$(top_srcdir)/services/fm/fma/inc/fma_mem.h \
	$(top_srcdir)/services/fm/fma/inc/fma_log.h \
	$(top_srcdir)/services/fm/fma/inc/fma.h \
	$(top_srcdir)/services/fm/fma/inc/fma_mds.h \
	$(top_srcdir)/services/fm/common/inc/fma_fm_intf.h

libfma_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-I $(top_srcdir)/services/fm/fma/inc \
	-I $(top_srcdir)/include \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/fm/common/inc \
	-I $(top_srcdir)/services/mab/inc

libfma_la_CFLAGS = $(AM_CFLAGS) @HPI_CFLAGS@

libfma_la_SOURCES = \
	$(top_srcdir)/services/fm/common/src/fma_fm_intf.c \
	src/fma_api.c \
	src/fma_hdl.c \
	src/fma_init.c \
	src/fma_log.c \
	src/fma_mds.c

libfma_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@
