#      -*- OpenSAF  -*-
#
# (C) Copyright 2016 The OpenSAF Foundation
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

DEFAULT_INCLUDES =

SUBDIRS = tests

noinst_HEADERS = \
	log_server.h \
	log_writer.h \
	transport_monitor.h

osaf_execbindir = $(pkglibdir)
osaf_execbin_PROGRAMS = osaftransportd

osaftransportd_CXXFLAGS =$(AM_CXXFLAGS)

osaftransportd_CPPFLAGS = \
	$(AM_CPPFLAGS)

osaftransportd_SOURCES = \
	log_server.cc \
	log_writer.cc \
	transport_monitor.cc \
	main.cc

osaftransportd_LDADD = \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la
