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

noinst_LTLIBRARIES = libifd_miblib.la

MIBLIB_GENERATED_FILES = \
	$(top_builddir)/osaf/snmp/miblib/ifsv/ifd/ifmibobjects_mib.c \
	$(top_builddir)/osaf/snmp/miblib/ifsv/ifd/ncsifsvifipentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/ifsv/ifd/ncsifsvifxentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/ifsv/ifd/intf_mib.c \
	$(top_builddir)/osaf/snmp/miblib/ifsv/ifd/ncsifsvifmapentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/ifsv/ifd/ncsifsvipentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/ifsv/ifd/ncsifsvbindifentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/ifsv/ifd/ncsvipentry_mib.c \
	$(top_builddir)/osaf/snmp/miblib/ifsv/ifd/ifentry_mib.c

libifd_miblib_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/ifsv/include \
	-I$(top_builddir)/osaf/snmp/mapi/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

libifd_miblib_la_LDFLAGS = -static

nodist_libifd_miblib_la_SOURCES = $(MIBLIB_GENERATED_FILES)

CLEANFILES = \
	$(MIBLIB_GENERATED_FILES)
