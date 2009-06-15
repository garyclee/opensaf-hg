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

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = saflogger

noinst_HEADERS = \
	inc/saf_error.h

saflogger_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/logsv/utils/inc

saflogger_SOURCES = \
	saf_logger.c \
	saf_error.c

saflogger_LDADD = $(top_builddir)/lib/lib_SaLog/libSaLog.la
