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

SUBDIRS = include

noinst_LTLIBRARIES = libmqsv_common.la

libmqsv_common_la_CPPFLAGS = \
	-DNCS_MQA=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/mqsv/include

libmqsv_common_la_SOURCES = \
	mqsv_common.c \
	mqsv_asapi_dec.c \
	mqsv_asapi_enc.c \
	mqsv_asapi.c \
	mqsv_edu.c \
	posix.c
