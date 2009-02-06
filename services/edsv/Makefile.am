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

CONTROLLER_SUBDIRS = eds logstr

SUBDIRS = $(CONTROLLER_SUBDIRS)

noinst_HEADERS = \
	inc/eda_cb.h \
	inc/eda_dl_api.h \
	inc/eda.h \
	inc/eda_hdl.h \
	inc/eda_log.h \
	inc/eda_mds.h \
	inc/eda_mem.h \
	inc/eds_amf.h \
	inc/eds_cb.h \
	inc/eds_ckpt.h \
	inc/eds_dl_api.h \
	inc/eds_evt.h \
	inc/eds.h \
	inc/eds_log.h \
	inc/eds_mds.h \
	inc/eds_mem.h \
	inc/edsv_defs.h \
	inc/edsv_logstr.h \
	inc/edsv_mem.h \
	inc/edsv_mib.h \
	inc/edsv_msg.h \
	inc/edsv_util.h

pkglib_PROGRAMS = ncs_eds

ncs_eds_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_EDS=1 -DNCS_EDSV_LOG=1 \
	-I $(top_srcdir)/services/edsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/vds/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

ncs_eds_SOURCES = \
	$(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_eds_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/lib/lib_SaClm/libSaClm.la \
	$(top_builddir)/services/mbcsv/src/libmbca.la \
	$(top_builddir)/services/edsv/common/libedsv_common.la \
	$(top_builddir)/services/edsv/eds/libeds.la
