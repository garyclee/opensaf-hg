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

CONTROLLER_SUBDIRS = logstr

SUBDIRS = srmnd $(CONTROLLER_SUBDIRS)

noinst_HEADERS = \
	inc/srma_cb.h \
	inc/srma_cbk.h \
	inc/srma_db.h \
	inc/srma_dl_api.h \
	inc/srma_func.h \
	inc/srma.h \
	inc/srma_log.h \
	inc/srma_logstr.h \
	inc/srma_mds.h \
	inc/srma_mem.h \
	inc/srmnd_amf.h \
	inc/srmnd_cb.h \
	inc/srmnd_db.h \
	inc/srmnd_dl_api.h \
	inc/srmnd_evt.h \
	inc/srmnd_func.h \
	inc/srmnd.h \
	inc/srmnd_log.h \
	inc/srmnd_logstr.h \
	inc/srmnd_mds.h \
	inc/srmnd_mem.h \
	inc/srmst_defs.h \
	inc/srmsv_db.h \
	inc/srmsv_defs.h \
	inc/srmsv.h \
	inc/srmsv_log.h \
	inc/srmsv_logstr.h \
	inc/srmsv_mem.h \
	inc/srmsv_msg.h

pkglib_PROGRAMS = ncs_srmnd

ncs_srmnd_CPPFLAGS = \
	-DNCS_SRMND=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/srmsv/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/common/inc \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/vds/inc

ncs_srmnd_SOURCES = \
	$(top_srcdir)/services/common/ncs_main_pvt.c

ncs_srmnd_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/services/srmsv/srmnd/libsrmnd.la
