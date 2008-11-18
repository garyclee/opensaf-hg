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

CONTROLLER_SUBDIRS = src logstr

SUBDIRS = $(CONTROLLER_SUBDIRS)

noinst_HEADERS = \
	inc/pdrbd_amf.h \
	inc/pdrbd_cb.h \
	inc/pdrbd_dl_api.h \
	inc/pdrbd.h \
	inc/pdrbd_log.h \
	inc/pdrbd_mds.h \
	inc/pdrbd_mem.h \
	inc/pdrbd_util.h

bin_PROGRAMS = ncs_pdrbd

ncs_pdrbd_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_PDRBD=1 -DPDRBD_LOG=1 \
	-I $(top_srcdir)/tools/pdrbd/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/vds/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/include

ncs_pdrbd_SOURCES = \
	$(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_pdrbd_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/tools/pdrbd/src/libpdrbd.la
