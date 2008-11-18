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

pkglib_LTLIBRARIES = libavsv_logstr.la

libavsv_logstr_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_DTS=1 \
	-I $(top_srcdir)/services/avsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

libavsv_logstr_la_SOURCES = \
	ava_logstr.c \
	avd_logstr.c \
	avnd_logstr.c \
	avsv_logstr.c \
	cla_logstr.c

libavsv_logstr_la_LDFLAGS = -version-number 2
