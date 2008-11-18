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

noinst_LTLIBRARIES = libsrmnd.la

libsrmnd_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_SRMND=1 -DNCS_SRMND_LOG=1 \
	-I $(top_srcdir)/services/srmsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

libsrmnd_la_LDFLAGS = -static

libsrmnd_la_SOURCES = \
	srmnd_amf.c \
	srmnd_db.c \
	srmnd_evt.c \
	srmnd_init.c \
	srmnd_log.c \
	srmnd_mds.c \
	srmnd_mon.c \
	srmnd_msg.c \
	srmnd_proc.c \
	srmnd_stat.c \
	srmnd_tmr.c \
	$(top_srcdir)/services/srmsv/common/srmsv_edumsg.c \
	$(top_srcdir)/services/srmsv/srmst/srmst_stat.c \
	$(top_srcdir)/services/srmsv/srmst/srmst_util.c
