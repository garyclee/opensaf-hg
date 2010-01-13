#      -*- OpenSAF  -*-
#
# (C) Copyright 2009 The OpenSAF Foundation
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

noinst_LTLIBRARIES = libavd_miblib.la

MIBLIB_GENERATED_FILES = \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/avsvscalars_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/ncsndtableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/ncssgtableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/ncssitableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/ncssutableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/saamfcompcstypesupportedtableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/saamfcomptableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/saamfcsinamevaluetableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/saamfcsitableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/saamfcstypeparamentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/saamfhealthchecktableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/saamfnodetableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/saamfscalars_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/saamfsgsirankentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/saamfsgsurankentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/saamfsgtableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/saamfsisideptableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/saamfsitableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/saamfsusitableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/saamfsuspersirankentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/saamfsutableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/saclmnodetableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avd/safclmscalarobject_mib.c

libavd_miblib_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/avsv/include \
	-I$(top_builddir)/osaf/snmp/mapi/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

libavd_miblib_la_CFLAGS = @HPI_CFLAGS@

libavd_miblib_la_LDFLAGS = -static

nodist_libavd_miblib_la_SOURCES = $(MIBLIB_GENERATED_FILES)

CLEANFILES = \
	$(MIBLIB_GENERATED_FILES)
