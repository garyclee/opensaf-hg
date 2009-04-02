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
# Author(s): Ericsson AB
#
#

MAINTAINERCLEANFILES = Makefile.in

noinst_HEADERS = \
	immnd_cb.h \
	immnd.h \
	immnd_init.h \
	ImmAttrValue.hh \
	ImmModel.hh \
	ImmSearchOp.hh

pkglib_PROGRAMS = opensaf_immnd

opensaf_immnd_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_MAIN_EXTERNAL \
	-I $(top_srcdir)/services/immsv/inc \
	-I $(top_srcdir)/services/logsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/vds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

opensaf_immnd_SOURCES =	\
	$(top_srcdir)/services/common/src/ncs_main_pvt.c \
	immnd_amf.c \
	immnd_db.c \
	immnd_evt.c \
	immnd_main.c \
	immnd_mds.c \
	immnd_proc.c \
	ImmAttrValue.cc \
	ImmSearchOp.cc \
	ImmModel.cc

opensaf_immnd_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/lib/lib_SaClm/libSaClm.la \
	$(top_builddir)/services/immsv/common/libimmsv_common.la
