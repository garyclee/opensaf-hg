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

pkglib_PROGRAMS = plmc_d 

plmc_d_CPPFLAGS = \
	-DNCS_MAIN_EXTERNAL \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/saf/include \
	-I$(top_srcdir)/osaf/libs/common/plmsv/include

plmc_d_CFLAGS = $(AM_CFLAGS) @HPI_CFLAGS@

plmc_d_SOURCES = \
	plmc_d.c

plmc_d_LDADD = \
	$(top_builddir)/osaf/libs/common/plmsv/plmc/libplmc_common.la

