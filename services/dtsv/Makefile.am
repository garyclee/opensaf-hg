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

CONTROLLER_SUBDIRS = clicmds dts logstr pssv_integ

SUBDIRS = $(CONTROLLER_SUBDIRS)

noinst_HEADERS = \
	inc/dta_api.h \
	inc/dta_def.h \
	inc/dta_dl_api.h \
	inc/dta.h \
	inc/dta_mem.h \
	inc/dta_opt.h \
	inc/dta_pvt.h \
	inc/dta_tgt.h \
	inc/dts_amf.h \
	inc/dts_api.h \
	inc/dts_ckp.h \
	inc/dts_ckpt_edu.h \
	inc/dts_ckpt_msg.h \
	inc/dts_ckpt_updt.h \
	inc/dts_def.h \
	inc/dts_dl_api.h \
	inc/dts.h \
	inc/dts_log.h \
	inc/dts_mapi.h \
	inc/dts_mem.h \
	inc/dts_opt.h \
	inc/dts_pvt.h \
	inc/dts_tgt.h \
	inc/dtsvcli.h \
	inc/dtsv_mem.h \
	inc/dtsv_msg.h \
	inc/sysf_log.h

pkglib_PROGRAMS = ncs_dts

ncs_dts_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_DTS=1 -DDTA_FLOW=1 -DNCS_DTA=1 \
	-I $(top_srcdir)/services/dtsv/inc \
	-I $(top_srcdir)/include \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/vds/inc \
	-I $(top_srcdir)/services/mab/inc

ncs_dts_SOURCES = \
	$(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_dts_LDFLAGS = -Wl,--export-dynamic

ncs_dts_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/services/mbcsv/src/libmbca.la \
	$(top_builddir)/services/dtsv/dts/libdts.la
