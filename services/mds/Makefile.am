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

noinst_HEADERS= \
	inc/mds_adm.h \
	inc/mds_core.h \
	inc/mds_dl_api.h \
	inc/mds_dt2c.h \
	inc/mds_dt.h \
	inc/mds_dt_tipc.h \
	inc/mds_log.h \
	inc/mds_pvt.h

noinst_LTLIBRARIES = libmds.la

libmds_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	@MDS_WORD_ARCH_FLAGS@ \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/include \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/mab/inc

libmds_la_LDFLAGS = -static

libmds_la_SOURCES =	\
	src/mds_dt_tipc.c \
	src/mds_c_db.c \
	src/mds_main.c \
	src/mds_papi.c \
	src/mds_c_sndrcv.c \
	src/mds_dt.c \
	src/mds_c_api.c \
	src/mds_log.c
