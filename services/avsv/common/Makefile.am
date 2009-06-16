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

lib_LTLIBRARIES = libavsv_common.la

libavsv_common_la_CPPFLAGS = \
	-DNCS_EDU=1 \
	$(AM_CPPFLAGS) \
	-I$(top_builddir)/services/avsv/inc/avsv_snmp_inc \
	-I$(top_srcdir)/services/avsv/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/common/inc

libavsv_common_la_SOURCES = \
	$(top_srcdir)/services/common/saf_edu.c \
	avsv_d2nedu.c \
	avsv_nd2ndedu.c \
	avsv_d2nmsg.c \
	avsv_n2avamsg.c \
	avsv_n2clamsg.c \
	avsv_n2avaedu.c \
	avsv_n2claedu.c \
	avsv_util.c \
	avsv_eduutil.c

libavsv_common_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@
