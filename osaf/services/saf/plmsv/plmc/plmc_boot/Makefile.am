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
# Author(s): Emerson 


include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in


SUBDIRS =

noinst_HEADERS = 

pkglib_PROGRAMS = plmc_boot

plmc_boot_CPPFLAGS = \
	-DNCS_MAIN_EXTERNAL \
	$(AM_CPPFLAGS) \
	$(HPI_CFLAGS) \
	-I$(top_srcdir)/osaf/libs/saf/include \
	-I$(top_srcdir)/osaf/libs/common/plmsv/include

plmc_boot_SOURCES = \
	plmc_boot.c

plmc_boot_LDADD = \
	$(top_builddir)/osaf/libs/common/plmsv/plmc/libplmc_common.la

