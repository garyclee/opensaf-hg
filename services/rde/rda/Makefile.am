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

lib_LTLIBRARIES = librda.la

noinst_HEADERS = \
	inc/rda.h

librda_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/rde/rda/inc \
	-I$(top_srcdir)/services/leap/inc/ \
	-I$(top_srcdir)/services/common/inc \
	-I$(top_srcdir)/services/rde/common/inc

librda_la_SOURCES = \
	src/rda_main.c \
	src/rda_papi.c

librda_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@
