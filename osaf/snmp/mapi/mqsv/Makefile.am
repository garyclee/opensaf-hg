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

MAPI_GENERATED_FILES = \
	$(top_builddir)/osaf/snmp/mapi/include/mqsv_mapi.h

BUILT_SOURCES = $(MAPI_GENERATED_FILES)

$(MAPI_GENERATED_FILES): mapi

mapi:
	@(if [ ! -f mapi.gen ]; then \
		export SMIPATH=$(top_srcdir)/osaf/snmp/mibs:$(MIBDIRS); \
		export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
		smidump -R $(top_srcdir)/OpenSAFCopyRights \
	   	-f ncsmapi\
	   	-d $(top_builddir)/osaf \
	   	-o mqsv SAF-MSG-MIB; \
		touch mapi.gen; \
	fi)

CLEANFILES = \
	$(MAPI_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB \
	mapi.gen
