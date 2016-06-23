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
#            Ericsson AB
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = scripts

noinst_HEADERS = \
	lgs_cb.h \
	lgs_fmt.h \
	lgs_util.h \
	lgs_evt.h \
	lgs.h \
	lgs_stream.h \
	lgs_file.h \
	lgs_filehdl.h \
	lgs_config.h \
	lgs_mbcsv.h \
	lgs_mbcsv_v1.h \
	lgs_mbcsv_v2.h \
	lgs_mbcsv_v3.h \
	lgs_mbcsv_v5.h \
	lgs_recov.h \
	lgs_imm_gcfg.h

osaf_execbindir = $(pkglibdir)
osaf_execbin_PROGRAMS = osaflogd

osaflogd_CXXFLAGS = $(AM_CXXFLAGS)

osaflogd_CPPFLAGS = \
	-DSA_EXTENDED_NAME_SOURCE \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/logsv/include \
	-I$(top_srcdir)/osaf/libs/common/immsv/include

osaflogd_SOURCES = \
	lgs_amf.cc \
	lgs_evt.cc \
	lgs_fmt.cc \
	lgs_imm.cc \
	lgs_main.cc \
	lgs_config.cc \
	lgs_mds.cc \
	lgs_stream.cc \
	lgs_util.cc \
	lgs_file.cc \
	lgs_filehdl.cc \
	lgs_mbcsv.cc \
	lgs_mbcsv_v1.cc \
	lgs_mbcsv_v2.cc \
	lgs_mbcsv_v3.cc \
	lgs_mbcsv_v5.cc \
	lgs_recov.cc \
	lgs_imm_gcfg.cc

osaflogd_LDADD = \
	$(top_builddir)/osaf/tools/safimm/src/libimmutil.la \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOi.la \
	$(top_builddir)/osaf/libs/saf/libSaImm/libSaImmOm.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/rda/librda.la
