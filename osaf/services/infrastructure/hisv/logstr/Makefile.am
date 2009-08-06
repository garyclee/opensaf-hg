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

pkglib_LTLIBRARIES = libhisv_logstr.la

libhisv_logstr_la_CPPFLAGS = \
	-DNCS_HISV_LOG=1 -DNCS_DTS=1 -DDTS_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/hisv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/dtsv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

libhisv_logstr_la_SOURCES = \
	hcd_logstr.c

libhisv_logstr_la_LDFLAGS = -version-number 2
