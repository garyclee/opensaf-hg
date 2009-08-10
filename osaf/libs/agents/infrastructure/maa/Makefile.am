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

pkglib_LTLIBRARIES = libmaa.la

libmaa_la_CPPFLAGS = \
	-DNCS_MAC=1 -DMAB_LOG=1 -DNCS_MAB=1 -DNCS_CLI_MAA=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

libmaa_la_SOURCES = \
	mac_api.c \
	mac_lib.c \
	mac_mds.c \
	mac_pvt.c

libmaa_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@
