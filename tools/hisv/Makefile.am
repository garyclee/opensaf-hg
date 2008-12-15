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

bin_PROGRAMS = hisv_events_demo hisv_power_set_util

hisv_events_demo_CPPFLAGS= \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_SAF=1 \
	-I $(top_srcdir)/services/hisv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/vds/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/include

hisv_events_demo_CFLAGS = @HPI_CFLAGS@

noinst_HEADERS = \
	hisv_events_demo_app.h 

hisv_events_demo_SOURCES = \
	hisv_events_demo_app.c \
	hisv_events_demo_main.c

hisv_events_demo_LDADD = \
	@HPI_LIBS@ \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/lib/lib_SaEvt/libSaEvt.la \
	$(top_builddir)/services/edsv/common/libedsv_common.la \
	$(top_builddir)/services/hisv/hcd/libhcd.la \
	$(top_builddir)/services/hisv/hpl/libhpl.la


hisv_power_set_util_CPPFLAGS= \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_SAF=1 \
	-I $(top_srcdir)/services/hisv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/vds/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/include

hisv_power_set_util_CFLAGS = @HPI_CFLAGS@

hisv_power_set_util_SOURCES = \
	hisv_power_set_util.c

hisv_power_set_util_LDADD = \
	@HPI_LIBS@ \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/lib/lib_SaEvt/libSaEvt.la \
	$(top_builddir)/services/edsv/common/libedsv_common.la \
	$(top_builddir)/services/hisv/hcd/libhcd.la \
	$(top_builddir)/services/hisv/hpl/libhpl.la

