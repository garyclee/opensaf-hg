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

pkglib_PROGRAMS = immload

bin_PROGRAMS = immdump immadm immfind immlist immcfg

noinst_HEADERS = \
	saf_error.h

immload_CXXFLAGS = @XML2_CFLAGS@

immload_CPPFLAGS = \
	-I $(top_srcdir)/include \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/immsv/inc

immload_SOURCES = \
	./imm_loader.cc

immload_LDFLAGS = @XML2_LIBS@

immload_LDADD = \
	$(top_srcdir)/lib/lib_SaImm/libSaImmOm.la

immdump_CXXFLAGS = @XML2_CFLAGS@

immdump_CPPFLAGS = \
	-I $(top_srcdir)/include \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/immsv/inc

immdump_SOURCES = \
	./imm_dumper.cc

immdump_LDFLAGS = @XML2_LIBS@

immdump_LDADD = \
	$(top_srcdir)/lib/lib_SaImm/libSaImmOm.la

immadm_CPPFLAGS = \
	-I $(top_srcdir)/include \
	-I $(top_srcdir)/services/immsv/inc

immadm_SOURCES = \
	imm_admin.c \
	immutil.c \
	saf_error.c

immadm_LDADD = \
	$(top_srcdir)/lib/lib_SaImm/libSaImmOm.la \
	$(top_srcdir)/lib/lib_SaImm/libSaImmOi.la

immfind_CPPFLAGS = -I $(top_srcdir)/include

immfind_SOURCES = \
	./imm_find.c \
	./saf_error.c

immfind_LDADD = $(top_srcdir)/lib/lib_SaImm/libSaImmOm.la

immlist_CPPFLAGS = -I $(top_srcdir)/include

immlist_SOURCES = \
	./imm_list.c \
	./saf_error.c

immlist_LDADD = $(top_srcdir)/lib/lib_SaImm/libSaImmOm.la

immcfg_CPPFLAGS = \
	-I $(top_srcdir)/include \
	-I $(top_srcdir)/services/immsv/inc

immcfg_SOURCES = \
	imm_cfg.c \
	immutil.c \
	saf_error.c

immcfg_LDADD = \
	$(top_srcdir)/lib/lib_SaImm/libSaImmOm.la \
	$(top_srcdir)/lib/lib_SaImm/libSaImmOi.la


