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

SUBDIRS = config scripts logstr

noinst_HEADERS = \
	pdrbd_amf.h \
	pdrbd_cb.h \
	pdrbd_dl_api.h \
	pdrbd.h \
	pdrbd_log.h \
	pdrbd_mds.h \
	pdrbd_mem.h \
	pdrbd_util.h

pkglib_PROGRAMS = ncs_pdrbd

ncs_pdrbd_CPPFLAGS = \
	-DNCS_PDRBD=1 -DPDRBD_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

ncs_pdrbd_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c \
	pdrbd_amf.c \
	pdrbd_log.c \
	pdrbd_mds.c \
	pdrbd_proc.c \
	pdrbd_proxied_amf.c \
	pdrbd_util.c

ncs_pdrbd_LDADD = \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la
