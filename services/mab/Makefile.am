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

CONTROLLER_SUBDIRS = mas psr psrcli logstr

SUBDIRS = $(CONTROLLER_SUBDIRS)

noinst_HEADERS = \
	inc/app_amf.h \
	inc/mab_def.h \
	inc/mab_env.h \
	inc/mab.h \
	inc/mab_log_const.h \
	inc/mab_log.h \
	inc/mab_mem.h \
	inc/mab_msg.h \
	inc/mab_mtbl.h \
	inc/mab_opt.h \
	inc/mab_tgt.h \
	inc/mac_api.h \
	inc/mac.h \
	inc/mac_pvt.h \
	inc/mas_amf.h \
	inc/mas_api.h \
	inc/mas_mbcsv.h \
	inc/mas_pvt.h \
	inc/oac_dl_api.h \
	inc/oac_pvt.h \
	inc/psr_amf.h \
	inc/psr_api.h \
	inc/psrcli.h \
	inc/psr_def.h \
	inc/psr_dl_api.h \
	inc/psr.h \
	inc/psr_log.h \
	inc/psr_mapi.h \
	inc/psr_pvt.h \
	inc/psr_rfmt.h \
	inc/psr_version.h \
	inc/pss_mbcsv.h

bin_PROGRAMS = ncs_cli_maa

pkglib_PROGRAMS = ncs_mas ncs_psr

ncs_mas_CPPFLAGS = \
	-DNCS_MAS=1 -DNCS_MAB=1 -DMAB_LOG=1 -DNCS_AVA=1 -DNCS_AVSV_LOG=1 \
	-DNCS_MAS_RED=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/avsv/inc \
	-I$(top_srcdir)/services/common/inc \
	-I$(top_srcdir)/services/vds/inc

ncs_mas_SOURCES = \
	$(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_mas_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/services/mbcsv/src/libmbca.la \
	$(top_builddir)/services/common/libsaf_common.la \
	$(top_builddir)/services/mab/mas/libmas.la

ncs_psr_CPPFLAGS = \
	-DNCS_MAC=1 -DNCS_PSR=1 -DNCS_AVA=1 -DNCS_AVSV_LOG=1 -DPSR_LOG=1 \
	-DNCS_PSS_RED=1 -DNCS_MBCSV=1 -DMBCSV_DEBUG=1 -DMBCSV_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/avsv/inc \
	-I$(top_srcdir)/services/mbcsv/inc \
	-I$(top_srcdir)/services/bam/inc \
	-I$(top_srcdir)/services/vds/inc \
	-I$(top_srcdir)/services/common/inc

ncs_psr_SOURCES = \
	$(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_psr_LDFLAGS = -Wl,--export-dynamic 

ncs_psr_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_builddir)/services/mbcsv/src/libmbca.la \
	$(top_builddir)/services/mab/mac/libmaa.la \
	$(top_builddir)/services/mab/psr/libpsr.la

ncs_cli_maa_CPPFLAGS = \
	-DNCS_MAC=1 -DNCS_CLI_MAA -DNCS_CLI=1 -DNCSCLI_DEBUG=1 -DNCSCLI_FILE=1 \
	-DNCSCLI_LOG=1 -DMAB_LOG=1 -DNCS_AVA=1 -DNCS_AVSV_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/mab/inc \
	-I$(top_srcdir)/services/leap/inc \
	-I$(top_srcdir)/services/mds/inc \
	-I$(top_srcdir)/services/avsv/inc \
	-I$(top_srcdir)/services/vds/inc \
	-I$(top_srcdir)/services/common/inc

ncs_cli_maa_SOURCES = \
	$(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_cli_maa_LDFLAGS = -Wl,--export-dynamic

ncs_cli_maa_LDADD = \
	$(top_builddir)/lib/lib_ncs_core/libncs_core.la \
	$(top_builddir)/services/cli/libcli.la \
	$(top_builddir)/services/hisv/hpl/libhpl.la \
	$(top_builddir)/lib/lib_SaEvt/libSaEvt.la \
	$(top_builddir)/services/mab/mac/libmaa.la
