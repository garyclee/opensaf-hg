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
# Author(s):  Emerson Network Power
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = scripts

noinst_HEADERS = \
	clms_cb.h \
	clms_mbcsv.h \
	clms_evt.h \
	clms_imm.h \
	clms_ntf.h \
	clms.h

pkglib_PROGRAMS = opensaf_clmd

opensaf_clmd_CPPFLAGS = \
	-DNCS_MAIN_EXTERNAL \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/clmsv/include \
	-I$(top_srcdir)/osaf/libs/common/immsv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

opensaf_clmd_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c \
	$(top_srcdir)/osaf/tools/safimm/src/immutil.c \
	$(top_srcdir)/osaf/libs/common/clmsv/clmsv_enc_dec.c \
	clms_amf.c \
	clms_evt.c \
	clms_imm.c \
	clms_main.c \
	clms_util.c \
	clms_mbcsv.c \
	clms_ntf.c \
	clms_mds.c

if ENABLE_AIS_PLM
opensaf_clmd_SOURCES += clms_plm.c
endif 

opensaf_clmd_LDADD = \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOi.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOm.la \
	$(top_builddir)/osaf/libs/saf/libSaNtf/libSaNtf.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/rda/librda.la

if ENABLE_AIS_PLM
opensaf_clmd_LDADD += \
	$(top_builddir)/osaf/libs/saf/libSaPlm/libSaPlm.la \
	$(top_builddir)/osaf/libs/common/clmsv/libclmsv_common.la \
	$(top_builddir)/osaf/libs/common/plmsv/plms/libplms_common.la \
	$(top_builddir)/osaf/libs/common/plmsv/plmc/libplmc_common.la 
endif
