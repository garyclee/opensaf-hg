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

noinst_LTLIBRARIES = libpdrbd.la

libpdrbd_la_CPPFLAGS = \
	-DNCS_PDRBD=1 -DPDRBD_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/tools/pdrbd/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/common/inc

libpdrbd_la_LDFLAGS = -static

libpdrbd_la_SOURCES = \
	pdrbd_amf.c \
	pdrbd_log.c \
	pdrbd_mds.c \
	pdrbd_proc.c \
	pdrbd_proxied_amf.c \
	pdrbd_util.c
