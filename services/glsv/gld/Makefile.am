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

noinst_LTLIBRARIES = libgld.la

MIBLIB_GENERATED_FILES = \
	$(top_builddir)/services/glsv/gld/miblib_gen/saflckscalarobject_mib.c \
	$(top_builddir)/services/glsv/gld/miblib_gen/salckresourceentry_mib.c

libgld_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_GLSV_LOG=1 \
	-I $(top_srcdir)/services/glsv/inc \
	-I $(top_builddir)/services/glsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/cpsv/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

libgld_la_LDFLAGS = -static

libgld_la_SOURCES = \
	gld_amf.c \
	gld_api.c \
	gld_evt.c \
	gld_log.c \
	gld_mbcsv.c \
	gld_mds.c \
	gld_mib.c \
	gld_red.c \
	gld_rsc.c \
	gld_scalar.c \
	gld_standby.c \
	gld_tbl.c \
	gld_tmr.c

nodist_libgld_la_SOURCES = $(MIBLIB_GENERATED_FILES)

$(MIBLIB_GENERATED_FILES): miblib

miblib:
	@for file in $(MIBLIB_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBSDIR); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmiblib \
			-d $(top_builddir)/services \
			SAF-LCK-MIB; \
		fi \
	done

CLEANFILES = \
	$(MIBLIB_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB

clean-local:
	-rm -rf $(top_builddir)/services/glsv/gld/miblib_gen
