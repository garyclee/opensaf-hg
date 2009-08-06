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
# Author(s): Ericsson AB
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = immadm

immadm_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/tools/safimm/include \
	-I$(top_srcdir)/osaf/libs/common/immsv/include

immadm_SOURCES = \
	$(top_srcdir)/osaf/tools/safimm/src/immutil.c \
	$(top_srcdir)/osaf/tools/safimm/src/saf_error.c \
	imm_admin.c

immadm_LDADD = \
	$(top_srcdir)/osaf/libs/saf/libSaImm/libSaImmOm.la \
	$(top_srcdir)/osaf/libs/saf/libSaImm/libSaImmOi.la
