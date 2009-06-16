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
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libmas.la

libmas_la_CPPFLAGS = \
	-DNCS_MAS=1 -DNCS_MAB=1 -DMAB_LOG=1 -DNCS_AVA=1 -DNCS_AVSV_LOG=1 \
	-DNCS_MAS_RED=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/common/inc

libmas_la_LDFLAGS = -static

libmas_la_SOURCES = \
	$(top_srcdir)/services/mab/mas_pss_common/app_amf.c \
	mas_amf.c \
	mas_api.c \
	mas_lib.c \
	mas_mbcsv.c \
	mas_mds.c \
	mas_pvt.c

libmas_la_LIBADD = $(top_builddir)/services/rde/rda/librda.la
