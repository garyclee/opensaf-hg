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

CONTROLLER_SUBDIRS = gld logstr

SUBDIRS = glnd $(CONTROLLER_SUBDIRS)

noinst_HEADERS = \
	inc/gla_callbk.h \
	inc/gla_cb.h \
	inc/gla_dl_api.h \
	inc/gla_evt.h \
	inc/gla.h \
	inc/gla_log.h \
	inc/gla_mds.h \
	inc/gla_mem.h \
	inc/gld_amf.h \
	inc/gld_api.h \
	inc/gld_cb.h \
	inc/gld_dl_api.h \
	inc/gld_edu.h \
	inc/gld_evt.h \
	inc/gld.h \
	inc/gld_log.h \
	inc/gld_mbcsv.h \
	inc/gld_mds.h \
	inc/gld_mem.h \
	inc/gld_mib.h \
	inc/gld_tmr.h \
	inc/glnd_api.h \
	inc/glnd_cb.h \
	inc/glnd_client.h \
	inc/glnd_dl_api.h \
	inc/glnd_edu.h \
	inc/glnd_evt.h \
	inc/glnd.h \
	inc/glnd_log.h \
	inc/glnd_mds.h \
	inc/glnd_mem.h \
	inc/glnd_res.h \
	inc/glnd_restart.h \
	inc/glnd_tmr.h \
	inc/glsv_defs.h \
	inc/glsv_lck.h \
	inc/glsv_logstr.h \
	inc/glsv_mem.h

bin_PROGRAMS = ncs_gld ncs_glnd

ncs_gld_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_GLD=1 \
	-I $(top_srcdir)/services/glsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/vds/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

ncs_gld_SOURCES = \
	$(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_gld_LDADD =  \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/lib/lib_SaClm/libSaClm.la \
	$(top_builddir)/services/mbcsv/src/libmbca.la \
	$(top_builddir)/services/glsv/gld/libgld.la \
	$(top_builddir)/services/glsv/common/libglsv_common.la

ncs_glnd_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_GLND=1 \
	-I $(top_srcdir)/services/glsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/vds/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

ncs_glnd_SOURCES = \
	$(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_glnd_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/lib/lib_SaCkpt/libSaCkpt.la \
	$(top_builddir)/services/glsv/glnd/libglnd.la \
	$(top_builddir)/services/glsv/common/libglsv_common.la
