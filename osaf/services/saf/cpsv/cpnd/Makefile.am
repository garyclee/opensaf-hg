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

pkglib_PROGRAMS = ncs_cpnd

ncs_cpnd_CPPFLAGS = \
	-DNCS_CPND=1 -DNCS_CPSV_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/cpsv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/vds/include \
	-I$(top_builddir)/osaf/snmp/mapi/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

ncs_cpnd_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c \
	cpnd_amf.c \
	cpnd_db.c \
	cpnd_evt.c \
	cpnd_init.c \
	cpnd_log.c \
	cpnd_mds.c \
	cpnd_proc.c \
	cpnd_res.c \
	cpnd_tmr.c

ncs_cpnd_LDADD = \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la \
	$(top_builddir)/osaf/libs/saf/libSaClm/libSaClm.la \
	$(top_builddir)/osaf/libs/common/cpsv/libcpsv_common.la

clean-local:
	-rm -rf $(top_builddir)/services/cpsv/inc/snmp_inc
