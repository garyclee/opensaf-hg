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

CONTROLLER_SUBDIRS = mqd logstr

SUBDIRS = mqnd $(CONTROLLER_SUBDIRS)

noinst_HEADERS = \
	inc/mqa_callbk.h \
	inc/mqa_db.h \
	inc/mqa_def.h \
	inc/mqa_dl_api.h \
	inc/mqa.h \
	inc/mqa_log.h \
	inc/mqa_mem.h \
	inc/mqd_api.h \
	inc/mqd_clm.h \
	inc/mqd_db.h \
	inc/mqd_dl_api.h \
	inc/mqd.h \
	inc/mqd_log.h \
	inc/mqd_logstr.h \
	inc/mqd_mem.h \
	inc/mqd_mib.h \
	inc/mqd_red.h \
	inc/mqd_saf.h \
	inc/mqd_tmr.h \
	inc/mqnd_db.h \
	inc/mqnd_dl_api.h \
	inc/mqnd_evt.h \
	inc/mqnd.h \
	inc/mqnd_log.h \
	inc/mqnd_mds.h \
	inc/mqnd_mem.h \
	inc/mqnd_mib.h \
	inc/mqnd_proc.h \
	inc/mqnd_restart.h \
	inc/mqnd_saf.h \
	inc/mqnd_shm.h \
	inc/mqnd_tmr.h \
	inc/mqsv_asapi.h \
	inc/mqsv_asapi_mem.h \
	inc/mqsv_common.h \
	inc/mqsv_def.h \
	inc/mqsv_edu.h \
	inc/mqsv_evt.h \
	inc/mqsv.h \
	inc/mqsv_init.h \
	inc/mqsv_mbedu.h \
	inc/mqsv_mem.h \
	inc/posix.h

pkglib_PROGRAMS = ncs_mqd ncs_mqnd

ncs_mqd_CPPFLAGS = \
	-DNCS_MQD=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/mqsv/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/vds/inc \
	-I$(top_srcdir)/services/common/inc \
	-I$(top_srcdir)/services/mab/inc

ncs_mqd_SOURCES = \
	$(top_srcdir)/services/common/ncs_main_pvt.c

ncs_mqd_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/lib/lib_SaClm/libSaClm.la \
	$(top_builddir)/services/common/libsaf_common.la \
	$(top_builddir)/services/mbcsv/libmbca.la \
	$(top_builddir)/services/mqsv/mqd/libmqd.la \
	$(top_builddir)/services/mqsv/common/libmqsv_common.la


ncs_mqnd_CPPFLAGS= \
	-DNCS_MQND=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/mqsv/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/vds/inc \
	-I$(top_srcdir)/services/common/inc \
	-I$(top_srcdir)/services/mab/inc

ncs_mqnd_SOURCES = \
	$(top_srcdir)/services/common/ncs_main_pvt.c

ncs_mqnd_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/lib/lib_SaClm/libSaClm.la \
	$(top_builddir)/services/mqsv/mqnd/libmqnd.la \
	$(top_builddir)/services/mqsv/common/libmqsv_common.la
