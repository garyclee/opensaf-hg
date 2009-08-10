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

pkglib_LTLIBRARIES = libhpl.la

libhpl_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/hisv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

libhpl_la_CFLAGS = $(AM_CFLAGS) @HPI_CFLAGS@

libhpl_la_SOURCES = \
	hpl_init.c \
	hpl_api.c \
	hpl_mds.c

libhpl_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@
