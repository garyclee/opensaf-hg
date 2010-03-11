#      -*- OpenSAF  -*-
#
# (C) Copyright 2009-2010 The OpenSAF Foundation
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
# Author(s): Emerson Network Power
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = scripts

pkglib_PROGRAMS = osafplmd

osafplmd_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/saf/include \
	-I$(top_srcdir)/osaf/libs/common/plmsv/include \
	-I$(top_srcdir)/osaf/libs/common/immsv/include

osafplmd_CFLAGS = $(AM_CFLAGS) @HPI_CFLAGS@

osafplmd_SOURCES = \
	$(top_srcdir)/osaf/tools/safimm/src/immutil.c \
	plms_main.c \
	plms_amf.c \
	plms_mds.c \
	plms_notifications.c \
	plms_mbcsv.c \
	plms_hrb_mds.c \
	plms_epath_util.c \
	plms_hrb.c \
	plms_hsm.c \
	plms_imm.c \
	plms_he_pres_fsm.c \
	plms_adm_fsm.c \
	plms_utils.c \
	plms_plmc.c \
	plms_proc.c \
	plms_stdby.c \
	plms_dbg_utils.c

osafplmd_LDADD = \
        @HPI_LIBS@ \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/common/avsv/libavsv_common.la \
	$(top_builddir)/osaf/libs/common/immsv/libimmsv_common.la \
	$(top_builddir)/osaf/libs/common/ntfsv/libntfsv_common.la \
	$(top_builddir)/osaf/libs/common/plmsv/plms/libplms_common.la \
	$(top_builddir)/osaf/services/saf/plmsv/plmc/plmc_lib/libplmc.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/rda/librda.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOi.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOm.la \
	$(top_builddir)/osaf/libs/saf/libSaNtf/libSaNtf.la
