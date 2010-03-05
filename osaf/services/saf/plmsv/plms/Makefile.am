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
# Author(s): Emerson 


include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in


SUBDIRS = scripts

noinst_HEADERS = 

pkglib_PROGRAMS = opensaf_plms

opensaf_plms_CPPFLAGS = \
	-DNCS_MAIN_EXTERNAL \
	$(AM_CPPFLAGS) \
	$(HPI_CFLAGS) \
	-I$(top_srcdir)/osaf/libs/saf/include \
	-I$(top_srcdir)/osaf/libs/common/plmsv/include \
	-I$(top_srcdir)/osaf/libs/common/immsv/include \
        -I$(top_srcdir)/osaf/services/infrastructure/masv/include

opensaf_plms_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c \
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

opensaf_plms_LDADD = \
        @HPI_LIBS@ \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la \
	$(top_builddir)/osaf/libs/common/plmsv/plms/libplms_common.la \
	$(top_builddir)/osaf/services/saf/plmsv/plmc/plmc_lib/libplmc.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/rda/librda.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOi.la \
        $(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOm.la \
        $(top_builddir)/osaf/libs/saf/libSaNtf/libSaNtf.la

