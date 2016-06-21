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

SUBDIRS = include tests

noinst_LTLIBRARIES = libava.la

libava_la_CXXFLAGS = $(AM_CXXFLAGS)

libava_la_CPPFLAGS = \
	-DSA_EXTENDED_NAME_SOURCE \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/amf/include \
	-I$(top_srcdir)/osaf/libs/agents/saf/amfa/include

libava_la_LDFLAGS = -static

libava_la_SOURCES = \
	amf_agent.cc \
	ava_hdl.cc \
	ava_init.cc \
	ava_mds.cc \
	ava_op.cc
