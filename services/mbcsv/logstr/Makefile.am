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

MAINTAINERCLEANFILES = Makefile.in

pkglib_LTLIBRARIES = libmbcsv_logstr.la

libmbcsv_logstr_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_MBCSV=1 -DMBCSV_DEBUG=1 -DMBCSV_LOG=1 -DNCS_DTS=1 \
	-I $(top_srcdir)/services/mbcsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/include \
	-I $(top_srcdir)/services/common/inc

libmbcsv_logstr_la_SOURCES = \
	mbcsv_logstr.c

libmbcsv_logstr_la_LDFLAGS = -version-number 2
