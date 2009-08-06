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

pkglib_PROGRAMS = ncs_cpd

ncs_cpd_CPPFLAGS = \
	-DNCS_CPD=1 -DNCS_CPSV_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/cpsv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include \
	-I$(top_builddir)/osaf/snmp/mapi/include

ncs_cpd_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c \
	cpd_amf.c \
	cpd_ckptentry.c \
	cpd_ckptscalar.c \
	cpd_db.c \
	cpd_evt.c \
	cpd_init.c \
	cpd_log.c \
	cpd_mbcsv.c \
	cpd_mds.c \
	cpd_mib.c \
	cpd_proc.c \
	cpd_red.c \
	cpd_reploc.c \
	cpd_sbevt.c \
	cpd_tmr.c

ncs_cpd_LDADD = \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la \
	$(top_builddir)/osaf/libs/saf/libSaClm/libSaClm.la \
	$(top_builddir)/osaf/libs/common/cpsv/libcpsv_common.la \
	$(top_builddir)/osaf/snmp/miblib/cpsv/cpd/libcpd_miblib.la
