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

SUBDIRS = scripts

pkglib_PROGRAMS = opensaf_fmsd

noinst_HEADERS = \
	fm_amf.h \
	fm_cb.h \
	fm_evt.h \
	fm.h \
	fm_mds.h \
	fm_mem.h

opensaf_fmsd_CPPFLAGS= \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/fm/include

opensaf_fmsd_CFLAGS = $(AM_CFLAGS) @HPI_CFLAGS@

opensaf_fmsd_SOURCES = \
	$(top_srcdir)/osaf/services/infrastructure/fm/fma_fm_intf.c \
	fm_main.c \
	fm_mds.c \
	fm_rda.c \
	fm_amf.c

opensaf_fmsd_LDADD = \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/rda/librda.la \
	$(top_builddir)/osaf/services/infrastructure/hisv/hpl/libhpl.la
