#      -*- OpenSAF  -*-
#
# (C) Copyright 2010 The OpenSAF Foundation
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

sbin_PROGRAMS = plmc_d 

plmc_d_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/include

plmc_d_CFLAGS = $(AM_CFLAGS)

plmc_d_SOURCES = \
	plmc_d.c

plmc_d_LDFLAGS = -lpthread

plmc_d_LDADD = \
	$(top_builddir)/lib/utils/libplmc_utils.la
