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

noinst_LTLIBRARIES = libavm_miblib.la

MIBLIB_GENERATED_FILES = \
	$(top_builddir)/osaf/snmp/miblib/avsv/avm/ncsavmentdeploytableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avm/ncsavmentfaultdomaintableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avm/ncsavmentupgradetableentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/avsv/avm/ncsavmscalars_mib.c

libavm_miblib_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/saf/avsv/avm/include \
	-I$(top_srcdir)/osaf/services/infrastructure/hisv/include \
	-I$(top_builddir)/osaf/snmp/mapi/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

libavm_miblib_la_CFLAGS = @HPI_CFLAGS@

libavm_miblib_la_LDFLAGS = -static

nodist_libavm_miblib_la_SOURCES = $(MIBLIB_GENERATED_FILES)

CLEANFILES = \
	$(MIBLIB_GENERATED_FILES)
