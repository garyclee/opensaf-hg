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

SUBDIRS = config logstr services

noinst_HEADERS = \
	subagt_agt.h \
	subagt_amf.h \
	subagt_cb.h \
	subagt_dlapi.h \
	subagt_eda.h \
	subagt_edu.h \
	subagt.h \
	subagt_log.h \
	subagt_mac.h \
	subagt_mbx.h \
	subagt_mem.h

pkglib_LTLIBRARIES = libopensaf_snmp_subagt.la

libopensaf_snmp_subagt_la_CPPFLAGS = \
	-DNCS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=1 -DNCS_AVA=1 -DNCS_AVSV_LOG=1 \
	-DNCS_EDA=1 \
	$(AM_CPPFLAGS) \
	-I$(top_builddir)/osaf/snmp/subagt/services/avsv \
	-I$(top_builddir)/osaf/snmp/subagt/services/ifsv \
	-I$(top_builddir)/osaf/snmp/subagt/services/ifsv/ipxs \
	-I$(top_builddir)/osaf/snmp/subagt/services/ifsv/entp \
	-I$(top_builddir)/osaf/snmp/subagt/services/dtsv \
	-I$(top_builddir)/osaf/snmp/subagt/services/avsv/avm \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

libopensaf_snmp_subagt_la_CFLAGS = $(AM_CFLAGS) @NETSNMP_CFLAGS@

libopensaf_snmp_subagt_la_SOURCES = \
	subagt_oid_db.c \
	subagt_agt.c \
	subagt_amf.c \
	subagt_eda.c \
	subagt_edu.c \
	subagt_mab.c \
	subagt_mac.c \
	subagt_mbx.c \
	subagt_log.c \
	subagt_dlapi.c \
	subagt_cb.c

libopensaf_snmp_subagt_la_LDFLAGS = \
	@NETSNMP_LIBS@ \
	-version-info @OPENSAF_LIB_VERSION@

libopensaf_snmp_subagt_la_LIBADD = \
   $(top_builddir)/osaf/snmp/subagt/services/avsv/avm/libavm_subagt.la \
   $(top_builddir)/osaf/snmp/subagt/services/ifsv/entp/libifsv_entp_subagt.la \
   $(top_builddir)/osaf/snmp/subagt/services/ifsv/ipxs/libipxs_subagt.la \
   $(top_builddir)/osaf/snmp/subagt/services/ifsv/libifsv_subagt.la  \
   $(top_builddir)/osaf/snmp/subagt/services/avsv/libavsv_subagt.la \
   $(top_builddir)/osaf/snmp/subagt/services/dtsv/libdtsv_subagt.la \
   $(top_builddir)/osaf/libs/agents/infrastructure/rda/librda.la
