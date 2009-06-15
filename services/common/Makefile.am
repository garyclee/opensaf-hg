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

lib_LTLIBRARIES = libsaf_common.la 

noinst_LTLIBRARIES = libncs_common.la 

noinst_HEADERS = \
	inc/avm_avd.h \
	inc/logtrace.h \
	inc/ncs_avsv_bam_msg.h \
	inc/ncs_bam_avm.h \
	inc/ncs_main_pvt.h \
	inc/ncs_mda_pvt.h \
	inc/ncs_saf_edu.h \
	inc/ncs_util.h \
	inc/oac_api.h \
	inc/saf_def.h \
	inc/saf_mem.h \
	inc/sprr_dl_api.h \
	inc/nid_start_util.h

libncs_common_la_CPPFLAGS = \
	-DNCS_DTA=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/common/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/dtsv/inc \
	-I$(top_srcdir)/services/common/inc \
	-I$(top_srcdir)/services/vds/inc \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/avsv/inc \
	-I$(top_srcdir)/services/bam/inc \
	-I$(top_srcdir)/services/srmsv/inc \
	-I$(top_srcdir)/services/avsv/avm/inc

libncs_common_la_LDFLAGS = -static

libncs_common_la_SOURCES = \
	src/ncs_main_pub.c \
	src/ncs_sprr.c \
	src/logtrace.c \
	src/nid_start_util.c

libsaf_common_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/common/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/common/inc

libsaf_common_la_SOURCES = \
	src/avm_avd_mem.c \
	src/saf_edu.c

libsaf_common_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@
