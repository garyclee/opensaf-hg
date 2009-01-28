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

lib_LTLIBRARIES = libglsv_common.la

MAPI_GENERATED_FILES = \
	$(top_builddir)/services/glsv/inc/glsv_mapi.h

BUILT_SOURCES = $(MAPI_GENERATED_FILES)

libglsv_common_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_GLSV_LOG=1 \
	-I $(top_srcdir)/services/glsv/inc \
	-I $(top_builddir)/services/glsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/cpsv/inc \
	-I $(top_srcdir)/include

libglsv_common_la_SOURCES = \
	glsv_edu.c

libglsv_common_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@

$(MAPI_GENERATED_FILES): mapi

mapi:
	@for file in $(MAPI_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBSDIR); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmapi \
			-d $(top_builddir)/services \
			-o glsv SAF-LCK-MIB; \
		fi \
	done

CLEANFILES = \
	$(MAPI_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB
