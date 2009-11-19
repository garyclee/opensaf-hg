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

SUBDIRS = include

noinst_LTLIBRARIES = libcla.la

libcla_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/avsv/include \
	-I$(top_srcdir)/osaf/libs/agents/saf/cla/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include \
	-I$(top_builddir)/osaf/snmp/mapi/include

libcla_la_LDFLAGS = -static

libcla_la_SOURCES = \
	cla_api.c \
	cla_hdl.c \
	cla_init.c \
	cla_log.c \
	cla_mds.c \
	cla_op.c
