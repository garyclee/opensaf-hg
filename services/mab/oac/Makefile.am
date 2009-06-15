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

noinst_LTLIBRARIES = liboaa.la

liboaa_la_CPPFLAGS = \
	-DMAB_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/common/inc \
	-I$(top_srcdir)/services/vds/inc \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/bam/inc

liboaa_la_LDFLAGS = -static

liboaa_la_SOURCES = \
	$(top_srcdir)/services/mab/common/mab_log.c \
	$(top_srcdir)/services/mab/common/mab_mds.c \
	$(top_srcdir)/services/mab/src/mab_def.c \
	$(top_srcdir)/services/mab/src/mab_tgt.c \
	oac_api.c \
	oac_lib.c \
	oac_mds.c \
	oac_pvt.c
