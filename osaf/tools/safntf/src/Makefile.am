#      -*- OpenSAF  -*-
#
# (C) Copyright 2014 The OpenSAF Foundation
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

noinst_LTLIBRARIES = libntfclient.la

libntfclient_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DSA_EXTENDED_NAME_SOURCE \
	-I$(top_srcdir)/osaf/libs/saf/include \
	-I$(top_srcdir)/osaf/tools/safntf/include \
	-I$(top_srcdir)/osaf/libs/common/ntfsv/include

libntfclient_la_SOURCES = \
	ntfclient.c
