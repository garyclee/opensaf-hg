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

CONTROLLER_SUBDIRS = ncs-snmp-subagt

SUBDIRS = $(CONTROLLER_SUBDIRS)

MAINTAINERCLEANFILES = Makefile.in

pkglib_PROGRAMS = ncs_snmp_subagt

ncs_snmp_subagt_CPPFLAGS = \
	-DNCS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=1 -DNCS_MAC=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/nms/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/vds/inc \
	-I$(top_srcdir)/services/common/inc

ncs_snmp_subagt_SOURCES = \
	$(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_snmp_subagt_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/lib/lib_SaEvt/libSaEvt.la \
	$(top_builddir)/services/mab/mac/libmaa.la
