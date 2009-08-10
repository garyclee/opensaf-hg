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

if ENABLE_HPI

SUBDIRS = include scripts hcd hpl logstr

pkglib_PROGRAMS = ncs_hisv

else

SUBDIRS = include scripts hpl

endif

ncs_hisv_CPPFLAGS= \
	-DNCS_HISV=1 -DNCS_HISV_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/hisv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

ncs_hisv_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c

ncs_hisv_LDADD = \
	@HPI_LIBS@ \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la \
	$(top_builddir)/osaf/libs/saf/libSaEvt/libSaEvt.la \
	$(top_builddir)/osaf/services/infrastructure/hisv/hcd/libhcd.la \
	$(top_builddir)/osaf/services/infrastructure/hisv/hpl/libhpl.la
