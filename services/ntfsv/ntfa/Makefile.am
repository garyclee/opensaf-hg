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
#

MAINTAINERCLEANFILES = Makefile.in

noinst_HEADERS = \
	inc/ntfa.h

noinst_LTLIBRARIES = libntfa.la

libntfa_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-I $(top_srcdir)/services/ntfsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include \
	-I $(top_srcdir)/services/ntfsv/ntfa/inc

libntfa_la_SOURCES = \
	ntfa_api.c \
	ntfa_util.c \
	$(top_srcdir)/services/ntfsv/common/ntfsv_enc_dec.c \
	$(top_srcdir)/services/ntfsv/common/ntfsv_mem.c \
	ntfa_mds.c

libntfa_la_LDFLAGS = -static
