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

noinst_LTLIBRARIES = libeds.la

MIBLIB_GENERATED_FILES = \
	$(top_builddir)/services/edsv/edsv_miblibgen/saevtchannelentry_mib.c \
	$(top_builddir)/services/edsv/edsv_miblibgen/safevtscalarobject_mib.c

MAPI_GENERATED_FILES = \
	$(top_builddir)/services/edsv/inc/edsv_mapi.h

BUILT_SOURCES = $(MAPI_GENERATED_FILES)

libeds_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_EDS=1 -DNCS_EDSV_LOG=1 -DNCS_DTA=1 \
	-I $(top_srcdir)/services/edsv/inc \
	-I $(top_builddir)/services/edsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include \
	-I $(top_srcdir)/services/common/inc

libeds_la_LDFLAGS = -static 

libeds_la_SOURCES = \
	eds_amf.c \
	eds_api.c \
	eds_cb.c \
	eds_ckpt.c \
	eds_debug.c \
	eds_evt.c \
	eds_ll.c \
	eds_log.c \
	eds_mds.c \
	eds_mib.c \
	eds_tmr.c \
	eds_util.c

nodist_libeds_la_SOURCES = $(MIBLIB_GENERATED_FILES)

libeds_la_LIBADD = $(top_builddir)/services/rde/rda/librda.la

$(MIBLIB_GENERATED_FILES): miblib

$(MAPI_GENERATED_FILES): mapi

miblib:
	@for file in $(MIBLIB_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBSDIR); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmiblib \
			-d $(top_builddir)/services \
			SAF-EVT-SVC-v7_5; \
		fi \
	done

mapi:
	@for file in $(MAPI_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBSDIR); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmapi \
			-d $(top_builddir)/services \
			-o edsv SAF-EVT-SVC-v7_5; \
		fi \
	done

CLEANFILES = \
	$(MAPI_GENERATED_FILES) \
	$(MIBLIB_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB

clean-local:
	-rm -rf $(top_builddir)/services/edsv/edsv_miblibgen
