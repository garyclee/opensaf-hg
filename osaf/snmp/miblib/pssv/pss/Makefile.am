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

noinst_LTLIBRARIES = libpss_miblib.la

MIBLIB_GENERATED_FILES = \
	$(top_builddir)/osaf/snmp/miblib/pssv/pss/ncspssvprofiletableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/pssv/pss/ncspssvscalars_mib.c

libpss_miblib_la_CPPFLAGS = \
        $(AM_CPPFLAGS) \
        -I$(top_srcdir)/osaf/services/infrastructure/bam/include \
        -I$(top_srcdir)/osaf/services/infrastructure/masv/include

libpss_miblib_la_LDFLAGS = -static

nodist_libpss_miblib_la_SOURCES = $(MIBLIB_GENERATED_FILES)

CLEANFILES = \
	$(MIBLIB_GENERATED_FILES)
