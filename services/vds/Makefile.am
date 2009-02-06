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

CONTROLLER_SUBDIRS = vds logstr

SUBDIRS = $(CONTROLLER_SUBDIRS)

pkglib_PROGRAMS = ncs_vds

noinst_HEADERS = \
	inc/mda_dl_api.h \
	inc/mda_mem.h \
	inc/mda_pvt_api.h \
	inc/vds_amf.h \
	inc/vds_cb.h \
	inc/vds_ckpt.h \
	inc/vds_def.h \
	inc/vds_dl_api.h \
	inc/vds_evt.h \
	inc/vds.h \
	inc/vds_log.h \
	inc/vds_logstr.h \
	inc/vds_mem.h

ncs_vds_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_VDS=1 -DNCS_VDS_LOG=1 \
	-I $(top_srcdir)/services/vds/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

ncs_vds_SOURCES = \
	$(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_vds_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/lib/lib_SaCkpt/libSaCkpt.la \
	$(top_builddir)/services/vds/vds/libvds.la
