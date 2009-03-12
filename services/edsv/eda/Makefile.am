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

MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libeda.la

libeda_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_EDA=1 -DNCS_DTA=1 \
	-I $(top_srcdir)/services/edsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

libeda_la_LDFLAGS = -static

libeda_la_SOURCES = \
	eda_util.c \
	eda_saf_api.c \
	eda_hdl.c \
	eda_log.c \
	eda_mds.c \
	eda_init.c
