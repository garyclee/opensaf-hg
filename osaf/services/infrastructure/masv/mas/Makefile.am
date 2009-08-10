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

SUBDIRS = scripts

pkglib_PROGRAMS = ncs_mas

ncs_mas_CPPFLAGS = \
	-DNCS_MAS=1 -DNCS_MAB=1 -DMAB_LOG=1 -DNCS_AVA=1 -DNCS_AVSV_LOG=1 \
	-DNCS_MAS_RED=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include \
	-I$(top_srcdir)/osaf/libs/common/avsv/include

ncs_mas_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c \
	$(top_srcdir)/osaf/services/infrastructure/masv/app_amf.c \
	mas_amf.c \
	mas_api.c \
	mas_lib.c \
	mas_mbcsv.c \
	mas_mds.c \
	mas_pvt.c

ncs_mas_LDADD = \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/rda/librda.la
