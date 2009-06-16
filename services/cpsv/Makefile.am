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

CONTROLLER_SUBDIRS = cpd logstr

SUBDIRS = cpnd $(CONTROLLER_SUBDIRS)

noinst_HEADERS = \
	inc/cpa_cb.h \
	inc/cpa_def.h \
	inc/cpa_dl_api.h \
	inc/cpa.h \
	inc/cpa_log.h \
	inc/cpa_mds.h \
	inc/cpa_mem.h \
	inc/cpa_proc.h \
	inc/cpa_tmr.h \
	inc/cpd_cb.h \
	inc/cpd_dl_api.h \
	inc/cpd.h \
	inc/cpd_init.h \
	inc/cpd_log.h \
	inc/cpd_mds.h \
	inc/cpd_mem.h \
	inc/cpd_mib.h \
	inc/cpd_proc.h \
	inc/cpd_red.h \
	inc/cpd_sbedu.h \
	inc/cpd_tmr.h \
	inc/cpnd_cb.h \
	inc/cpnd_dl_api.h \
	inc/cpnd.h \
	inc/cpnd_init.h \
	inc/cpnd_log.h \
	inc/cpnd_mem.h \
	inc/cpnd_mib.h \
	inc/cpnd_tmr.h \
	inc/cpsv_evt.h \
	inc/cpsv.h \
	inc/cpsv_mem.h \
	inc/cpsv_shm.h

pkglib_PROGRAMS = ncs_cpd ncs_cpnd

ncs_cpd_CPPFLAGS = \
	-DNCS_CPD=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/cpsv/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/vds/inc \
	-I$(top_srcdir)/services/common/inc \
	-I$(top_srcdir)/services/mab/inc

ncs_cpd_SOURCES = \
	$(top_srcdir)/services/common/ncs_main_pvt.c

ncs_cpd_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/lib/lib_SaClm/libSaClm.la \
	$(top_builddir)/services/mbcsv/libmbca.la \
	$(top_builddir)/services/cpsv/cpd/libcpd.la \
	$(top_builddir)/services/cpsv/common/libcpsv_common.la

ncs_cpnd_CPPFLAGS = \
	-DNCS_CPND=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/cpsv/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/vds/inc \
	-I$(top_srcdir)/services/common/inc \
	-I$(top_srcdir)/services/mab/inc

ncs_cpnd_SOURCES = \
	$(top_srcdir)/services/common/ncs_main_pvt.c

ncs_cpnd_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/lib/lib_SaClm/libSaClm.la \
	$(top_builddir)/services/cpsv/cpnd/libcpnd.la \
	$(top_builddir)/services/cpsv/common/libcpsv_common.la
