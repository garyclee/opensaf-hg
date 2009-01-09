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

noinst_LTLIBRARIES = libmqnd.la

MIBLIB_GENERATED_FILES = \
	$(top_builddir)/services/mqsv/mqnd/miblib_gen/samsgqueueentry_mib.c \
	$(top_builddir)/services/mqsv/mqnd/miblib_gen/samsgqueuepriorityentry_mib.c

MAPI_GENERATED_FILES = \
	$(top_builddir)/services/mqsv/inc/snmp_inc/mqsv_mapi.h

BUILT_SOURCES = $(MAPI_GENERATED_FILES)

libmqnd_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_MQND=1 -DASAPi_DEBUG=1 -DNCS_MQSV_LOG=1 -DNCS_DTA=1 \
	-I $(top_srcdir)/services/mqsv/inc \
	-I $(top_builddir)/services/mqsv/inc/snmp_inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

libmqnd_la_LDFLAGS =	-static

libmqnd_la_SOURCES =	\
	mqnd_qentry.c \
	mqnd_proc.c \
	mqnd_util.c \
	mqnd_db.c \
	mqnd_mq.c \
	mqnd_namedb.c \
	mqnd_qprentry.c \
	mqnd_amf.c \
	mqnd_evt.c \
	mqnd_log.c \
	mqnd_mds.c \
	mqnd_mib.c \
	mqnd_shm.c \
	mqnd_tmr.c \
	mqnd_restart.c \
	mqnd_init.c

nodist_libmqnd_la_SOURCES = $(MIBLIB_GENERATED_FILES)

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
	   	SAF-MSG-SVC-v7_5; \
		fi \
	done

mapi:
	@for file in $(MAPI_GENERATED_FILES); do \
		if [ ! -f $$file ]; then \
			export SMIPATH=$(top_srcdir)/mibs:$(MIBSDIR); \
			export PATH=$(top_builddir)/tools/smidump/bin:$$PATH; \
			smidump -R $(top_srcdir)/OpenSAFCopyRights \
	   	-f ncsmapi\
	   	-d $(top_builddir)/services \
	   	-o mqsv SAF-MSG-SVC-v7_5; \
		fi \
	done

CLEANFILES = \
	$(MIBLIB_GENERATED_FILES) \
	$(MAPI_GENERATED_FILES) \
	TEMP_ESMI_DUMP.MIB

clean-local:
	rm -rf $(top_builddir)/services/mqsv/mqnd/miblib_gen
