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

bin_PROGRAMS = saflogger

saflogger_CPPFLAGS = \
	-I $(top_srcdir)/include

saflogger_SOURCES = \
	./saf_logger.c \
	./saf_error.c

saflogger_LDADD = $(top_srcdir)/lib/lib_SaLog/libSaLog.la

