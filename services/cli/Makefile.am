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

CONTROLLER_SUBDIRS = logstr

SUBDIRS = $(CONTROLLER_SUBDIRS)

noinst_HEADERS = \
	inc/cliconst.h \
	inc/clictree.h \
	inc/cli_def.h \
	inc/cli.h \
	inc/cliio.h \
	inc/clilog.h \
	inc/cli_mem.h \
	inc/cli_opt.h \
	inc/clipar.h \
	inc/clistruc.h \
	inc/clitrav.h \
	inc/shimmab.h

noinst_LTLIBRARIES = libcli.la

libcli_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_CLI=1 -DNCSCLI_DEBUG=1 -DNCSCLI_FILE=1 -DNCSCLI_LOG=1 \
	-I $(top_srcdir)/services/cli/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

libcli_la_LDFLAGS = -static 

libcli_la_SOURCES =	\
	src/cliapi.c \
	src/cli.c \
	src/clictree.c \
	src/cliio.c \
	src/clilog.c \
	src/clitrav.c \
	src/sysfcli.c \
	src/prulegen/clilex.c \
	src/prulegen/clipar.c
