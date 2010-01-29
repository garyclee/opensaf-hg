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

SUBDIRS = scripts

pkglib_PROGRAMS = ncs_dts

ncs_dts_CPPFLAGS = \
	-DNCS_DTS=1 -DDTS_LOG=1 -DNCS_DTA=1 -DDTS_FLOW=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/dtsv/include \
	-I$(top_srcdir)/osaf/libs/common/immsv/include

ncs_dts_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c \
	$(top_srcdir)/osaf/tools/safimm/src/immutil.c \
	dts_amf.c \
	dts_api.c \
	dts_cbop.c \
	dts_chkop.c \
	dts_ckpt_dec.c \
	dts_ckpt_edu.c \
	dts_ckpt_enc.c \
	dts_ckpt_updt.c \
	dts_def.c \
	dts_dl_api.c \
	dts_log.c \
	dts_mds.c \
	dts_logpolc.c \
	dts_pvt.c \
	dts_seq.c \
	dts_tgt.c \
	dts_imm.c \
	dts_sysf_log.c

ncs_dts_LDFLAGS = -Wl,--export-dynamic

ncs_dts_LDADD = \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/common/avsv/libavsv_common.la \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/rda/librda.la \
	$(top_builddir)/osaf/libs/common/immsv/libimmsv_common.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOi.la \
        $(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOm.la
