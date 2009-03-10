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

pkglib_PROGRAMS = ncs_rde

ncs_rde_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-I $(top_srcdir)/services/rde/inc \
	-I $(top_srcdir)/include \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/rde/common/inc

ncs_rde_SOURCES = \
	rde_amf.c \
	rde_init.c \
	rde_log.c \
	rde_main.c \
	rde_rda.c \
	rde_rde.c

ncs_rde_LDADD = \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la
