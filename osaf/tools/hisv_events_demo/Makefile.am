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

bin_PROGRAMS = hisv_events_demo

hisv_events_demo_CPPFLAGS= \
	-DNCS_SAF=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/hisv/include

hisv_events_demo_CFLAGS = $(AM_CFLAGS) @HPI_CFLAGS@

noinst_HEADERS = \
	hisv_events_demo_app.h

hisv_events_demo_SOURCES = \
	hisv_events_demo_app.c \
	hisv_events_demo_main.c

hisv_events_demo_LDADD = \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/common/avsv/libavsv_common.la \
	$(top_builddir)/osaf/libs/common/edsv/libedsv_common.la \
	$(top_builddir)/osaf/libs/saf/libSaEvt/libSaEvt.la \
	$(top_builddir)/osaf/services/infrastructure/hisv/hpl/libhpl.la
