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

SUBDIRS = config logstr services

noinst_HEADERS = \
	cliconst.h \
	clictree.h \
	cli_def.h \
	cli.h \
	cliio.h \
	clilog.h \
	cli_mem.h \
	cli_opt.h \
	clipar.h \
	clistruc.h \
	clitrav.h

bin_PROGRAMS = ncs_cli_maa

ncs_cli_maa_CPPFLAGS = \
	-DNCS_MAC=1 -DNCS_CLI_MAA -DNCS_CLI=1 -DNCSCLI_DEBUG=1 -DNCSCLI_FILE=1 \
	-DNCSCLI_LOG=1 -DMAB_LOG=1 -DNCS_AVA=1 -DNCS_AVSV_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include \
	-I$(top_srcdir)/osaf/libs/common/avsv/include \
	-I$(top_srcdir)/osaf/libs/agents/saf/ava/include

ncs_cli_maa_LDFLAGS = -Wl,--export-dynamic

ncs_cli_maa_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c \
	prulegen/clilex.c \
	prulegen/clipar.c \
	cliapi.c \
	cli.c \
	clictree.c \
	cliio.c \
	clilog.c \
	clitrav.c \
	sysfcli.c

ncs_cli_maa_LDADD = \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/common/edsv/libedsv_common.la \
	$(top_builddir)/osaf/libs/saf/libSaEvt/libSaEvt.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/maa/libmaa.la \
	$(top_builddir)/osaf/services/infrastructure/hisv/hpl/libhpl.la
