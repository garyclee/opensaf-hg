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
	imm_dumper.hh

bin_PROGRAMS = immdump

immdump_CXXFLAGS = $(AM_CXXFLAGS) @XML2_CFLAGS@ @SQLITE3_CFLAGS@

immdump_SOURCES = \
	$(top_srcdir)/osaf/tools/safimm/src/immutil.c \
	imm_dumper.cc \
	imm_pbe_dump.cc \
	imm_pbe_daemon.cc \
	imm_xmlw_dump.cc

immdump_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/immsv/include

immdump_LDFLAGS = @XML2_LIBS@ @SQLITE3_LIBS@

immdump_LDADD = \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOm.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOi.la
