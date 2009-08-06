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

bin_PROGRAMS = hisv_power_set_util

hisv_power_set_util_CPPFLAGS= \
	-DNCS_SAF=1 \
	$(AM_CPPFLAGS)

hisv_power_set_util_CFLAGS = $(AM_CFLAGS) @HPI_CFLAGS@

hisv_power_set_util_SOURCES = \
	hisv_power_set_util.c

hisv_power_set_util_LDADD = \
	$(top_builddir)/osaf/libs/saf/libSaEvt/libSaEvt.la \
	$(top_builddir)/osaf/services/infrastructure/hisv/hpl/libhpl.la
