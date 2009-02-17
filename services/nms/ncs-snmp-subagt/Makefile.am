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

CONTROLLER_SUBDIRS = logstr

SUBDIRS = $(CONTROLLER_SUBDIRS)

noinst_HEADERS = \
	inc/subagt_agt.h \
	inc/subagt_amf.h \
	inc/subagt_cb.h \
	inc/subagt_dlapi.h \
	inc/subagt_eda.h \
	inc/subagt_edu.h \
	inc/subagt.h \
	inc/subagt_log.h \
	inc/subagt_mac.h \
	inc/subagt_mbx.h \
	inc/subagt_mem.h

pkglib_LTLIBRARIES = libncs_snmp_subagt.la

libncs_snmp_subagt_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=1 -DNCS_AVA=1 -DNCS_AVSV_LOG=1 -DNCS_EDA=1 \
	-I $(top_srcdir)/services/nms/ncs-snmp-subagt/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/include \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_builddir)/services/avsv/snmp_integ \
	-I $(top_builddir)/services/ifsv/snmp_integ \
	-I $(top_builddir)/services/ifsv/ipxs_snmp_integ \
	-I $(top_builddir)/services/ifsv/entp_snmp_integ \
	-I $(top_builddir)/services/dtsv/snmp_integ \
	-I $(top_builddir)/services/avsv/avm/snmp_integ

libncs_snmp_subagt_la_CFLAGS = $(AM_CFLAGS) @NETSNMP_CFLAGS@

libncs_snmp_subagt_la_SOURCES = \
	src/subagt_oid_db.c \
	src/subagt_agt.c \
	src/subagt_amf.c \
	src/subagt_eda.c \
	src/subagt_edu.c \
	src/subagt_mab.c \
	src/subagt_mac.c \
	src/subagt_mbx.c \
	src/subagt_log.c \
	src/subagt_dlapi.c \
	src/subagt_cb.c

libncs_snmp_subagt_la_LDFLAGS = \
	@NETSNMP_LIBS@ \
	-version-info @OPENSAF_LIB_VERSION@

libncs_snmp_subagt_la_LIBADD = \
   $(top_builddir)/services/avsv/avm/snmp_integ/libavm_subagt.la \
   $(top_builddir)/services/ifsv/entp_snmp_integ/libifsv_entp_subagt.la \
   $(top_builddir)/services/ifsv/ipxs_snmp_integ/libipxs_subagt.la \
   $(top_builddir)/services/ifsv/snmp_integ/libifsv_subagt.la  \
   $(top_builddir)/services/avsv/snmp_integ/libavsv_subagt.la \
   $(top_builddir)/services/dtsv/snmp_integ/libdtsv_subagt.la \
   $(top_builddir)/services/rde/rda/librda.la
