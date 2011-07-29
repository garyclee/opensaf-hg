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

noinst_LTLIBRARIES = libeda.la

libeda_la_CPPFLAGS = \
	-DNCS_EDA=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/edsv/include

libeda_la_LDFLAGS = -static

libeda_la_SOURCES = \
	eda_util.c \
	eda_saf_api.c \
	eda_hdl.c \
	eda_mds.c \
	eda_init.c
