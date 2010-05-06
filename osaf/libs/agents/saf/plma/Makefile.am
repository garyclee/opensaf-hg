#      -*- OpenSAF  -*-
#
# (C) Copyright 2010 The OpenSAF Foundation
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
# Author(s): Emerson
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libplma.la

libplma_la_CPPFLAGS = \
	-DNCS_PLMA=1 -DNCS_PLMSV_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/plmsv/include

libplma_la_CFLAGS = \
	$(AM_CFLAGS) \
	@HPI_CFLAGS@

libplma_la_LDFLAGS = -static

libplma_la_SOURCES = \
	plma_init.c \
	plma_api.c \
	plma_comm.c \
	plma_mds.c
