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

SUBDIRS = include

noinst_LTLIBRARIES = libmbca.la

libmbca_la_LDFLAGS = -static

libmbca_la_CPPFLAGS = \
	-DNCS_MBCSV=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/core/mbcsv/include

libmbca_la_SOURCES = \
	mbcsv_act.c \
	mbcsv_api.c \
	mbcsv_dl_api.c \
	mbcsv_fsm.c \
	mbcsv_mbx.c \
	mbcsv_mds.c \
	mbcsv_peer.c \
	mbcsv_pr_evts.c \
	mbcsv_pwe_anc.c \
	mbcsv_queue.c \
	mbcsv_tmr.c \
	mbcsv_util.c
