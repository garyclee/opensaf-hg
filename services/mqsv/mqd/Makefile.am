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

noinst_LTLIBRARIES = libmqd.la

MIBLIB_GENERATED_FILES = \
	$(top_builddir)/services/mqsv/mqd/miblib_gen/safmsgscalarobject_mib.c \
	$(top_builddir)/services/mqsv/mqd/miblib_gen/samsgqueuegroupmembersentry_mib.c \
	$(top_builddir)/services/mqsv/mqd/miblib_gen/samsgqueuegroupentry_mib.c

libmqd_la_CPPFLAGS =	\
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_MQD=1 -DASAPi_DEBUG=1 -DNCS_MQSV_LOG=1 -DNCS_DTA=1 \
	-I $(top_srcdir)/services/mqsv/inc \
	-I $(top_builddir)/services/mqsv/inc/snmp_inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

libmqd_la_LDFLAGS = -static

libmqd_la_SOURCES = \
	mqd_sbevt.c \
	mqd_mbcsv.c \
	mqd_mbedu.c \
	mqd_util.c \
	mqd_asapi.c \
	mqd_api.c \
	mqd_clm.c \
	mqd_evt.c \
	mqd_log.c \
	mqd_mds.c \
	mqd_mib.c \
	mqd_red.c \
	mqd_saf.c \
	mqd_tmr.c \
	mqd_qgrpmemsentry.c \
	mqd_grpentry.c \
	mqd_scalarentry.c

nodist_libmqd_la_SOURCES = $(MIBLIB_GENERATED_FILES)

$(MIBLIB_GENERATED_FILES): miblib

miblib:
	@for file in $(MIBLIB_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBSDIR); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
			-f ncsmiblib \
			-d $(top_builddir)/services \
			SAF-MSG-SVC-v7_5; \
		fi \
	done

CLEANFILES = \
	$(MIBLIB_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB

clean-local:
	-rm -rf $(top_builddir)/services/mqsv/mqd/miblib_gen
