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

SUBDIRS = logstr scripts

pkglib_PROGRAMS = ncs_psr

ncs_psr_CPPFLAGS = \
	-DNCS_PSR=1 -DNCS_MAB=1 -DMAB_LOG=1 -DPSR_LOG=1 -DNCS_PSS_RED=1 \
	-DNCS_MAC=1 -DNCS_AVA=1 -DNCS_AVSV_LOG=1 \
	-DNCS_MBCSV=1 -DMBCSV_DEBUG=1 -DMBCSV_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/bam/include \
	-I$(top_srcdir)/osaf/libs/common/avsv/include \
	-I$(top_srcdir)/osaf/libs/core/mbcsv/include \
	-I$(top_srcdir)/osaf/libs/agents/saf/ava/include

ncs_psr_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c \
	$(top_srcdir)/osaf/services/infrastructure/masv/app_amf.c \
	psr_amf.c \
	psr_api.c \
	psr_ext.c \
	psr_init.c \
	psr_log.c \
	psr_mds.c \
	psr_mib.c \
	psr_pvt.c \
	pss_mbcsv.c \
	psr_rfmt.c

ncs_psr_LDFLAGS = -Wl,--export-dynamic

ncs_psr_LDADD = \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/common/avsv/libavsv_common.la \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la \
 	$(top_builddir)/osaf/libs/agents/infrastructure/maa/libmaa.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/rda/librda.la \
	$(top_builddir)/osaf/snmp/miblib/pssv/pss/libpss_miblib.la
