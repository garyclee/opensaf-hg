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

noinst_LTLIBRARIES = libvds.la

libvds_la_CPPFLAGS = \
	-DNCS_VDS=1 -DNCS_VDS_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/vds/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/common/inc

libvds_la_LDFLAGS = -static

libvds_la_SOURCES = \
	vds_main.c \
	vds_amf.c \
	vds_evt.c \
	vds_log.c \
	vds_mds.c \
	vds_db.c \
	vds_ckpt.c
