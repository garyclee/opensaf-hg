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

noinst_LTLIBRARIES = libmds.la

libmds_la_CPPFLAGS = \
	@MDS_WORD_ARCH_FLAGS@ \
	$(AM_CPPFLAGS)

libmds_la_LDFLAGS = -static

libmds_la_SOURCES = \
	mds_dt_tcp.c \
	mds_dt_disc.c \
	mds_dt_trans.c \
	mds_dt_common.c \
	mds_c_db.c \
	mds_main.c \
	mds_papi.c \
	mds_c_sndrcv.c \
	mds_c_api.c \
	mds_log.c \
	ncs_ada.c \
	ncs_mda.c \
	ncs_vda.c


if ENABLE_TIPC_TRANSPORT
 
libmds_la_SOURCES +=  mds_dt_tipc.c 

endif
