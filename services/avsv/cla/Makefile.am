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

noinst_LTLIBRARIES = libcla.la

libcla_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_builddir)/services/avsv/inc/avsv_snmp_inc \
	-I$(top_srcdir)/services/avsv/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/common/inc

libcla_la_LDFLAGS = -static

libcla_la_SOURCES = \
	cla_api.c \
	cla_hdl.c \
	cla_init.c \
	cla_log.c \
	cla_mds.c \
	cla_op.c
