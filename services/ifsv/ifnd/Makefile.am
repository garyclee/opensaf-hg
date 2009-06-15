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

noinst_LTLIBRARIES = libifnd.la

libifnd_la_CPPFLAGS = \
	-DNCS_IFND=1 -DNCS_IFSV_LOG=1 -DNCS_VIP=1 -DNCS_MAB=1 -DVIP_HALS_SUPPORT=1 \
	-DNCS_IFSV_BOND_INTF=1 -DNCS_IFSV_IPXS=1 \
	$(AM_CPPFLAGS) \
	-I$(top_builddir)/services/ifsv/inc/ifsv_snmp_inc \
	-I$(top_srcdir)/services/ifsv/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/common/inc

libifnd_la_LDFLAGS = -static

libifnd_la_SOURCES = \
	ifnd_ipxs.c \
	ifnd_api.c \
	ifnd_evt.c \
	ifnd_mds.c \
	ifnd_mib.c \
	ifnd_red.c \
	ifnd_saf.c \
	ifnddevt.c \
	ifnddprc.c \
	ifnd_vipproc.c \
	ifndproc.c
