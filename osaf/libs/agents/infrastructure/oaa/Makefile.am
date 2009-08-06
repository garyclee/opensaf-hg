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
	-I$(top_srcdir)/osaf/services/infrastructure/bam/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

liboaa_la_LDFLAGS = -static

liboaa_la_SOURCES = \
	$(top_srcdir)/osaf/services/infrastructure/masv/mab_log.c \
	$(top_srcdir)/osaf/services/infrastructure/masv/mab_mds.c \
	$(top_srcdir)/osaf/services/infrastructure/masv/mab_def.c \
	$(top_srcdir)/osaf/services/infrastructure/masv/mab_tgt.c \
	oac_api.c \
	oac_lib.c \
	oac_mds.c \
	oac_pvt.c
