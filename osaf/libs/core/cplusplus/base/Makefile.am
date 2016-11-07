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
	buffer.h \
	file_notify.h \
	getenv.h \
	log_message.h \
	macros.h \
	process.h \
	time.h \
	unix_client_socket.h \
	unix_server_socket.h \
	unix_socket.h

noinst_LTLIBRARIES = libbase.la

libbase_CXXFLAGS =$(AM_CXXFLAGS)

libbase_la_CPPFLAGS = \
	$(AM_CPPFLAGS)

libbase_la_LDFLAGS = -static

libbase_la_SOURCES = \
	file_notify.cc \
	getenv.cc \
	log_message.cc \
	process.cc \
	unix_client_socket.cc \
	unix_server_socket.cc \
	unix_socket.cc
