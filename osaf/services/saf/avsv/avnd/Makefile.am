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

noinst_LTLIBRARIES = libavnd_ctrl.la libavnd_pld.la

COMMON_SRC = \
	avnd.c \
	avnd_cam.c \
	avnd_cbq.c \
	avnd_chc.c \
	avnd_clc.c \
	avnd_clm.c \
	avnd_clmdb.c \
	avnd_comp.c \
	avnd_compdb.c \
	avnd_cpm.c \
	avnd_di.c \
	avnd_err.c \
	avnd_evt.c \
	avnd_evtlog.c \
	avnd_hcdb.c \
	avnd_hpi.c \
	avnd_log.c \
	avnd_mab.c \
	avnd_mds.c \
	avnd_pg.c \
	avnd_pgdb.c \
	avnd_proc.c \
	avnd_sidb.c \
	avnd_srm.c \
	avnd_su.c \
	avnd_sudb.c \
	avnd_susm.c \
	avnd_term.c \
	avnd_tmr.c \
	avnd_trap.c \
	avnd_util.c \
	avnd_verify.c \
	avnd_avnd.c \
	avnd_proxy.c \
	avnd_proxydb.c 

COMMON_CPPFLAGS = \
	-DNCS_AVND=1 -DNCS_AVSV_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/avsv/include \
	-I$(top_builddir)/osaf/snmp/mapi/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

libavnd_ctrl_la_CPPFLAGS = \
	-DNCS_AVND_MBCSV_CKPT=1 \
	$(COMMON_CPPFLAGS)

libavnd_ctrl_la_LDFLAGS = -static

libavnd_ctrl_la_SOURCES = \
	$(COMMON_SRC) \
	avnd_mbcsv.c \
	avnd_ckpt_dec.c \
	avnd_ckpt_edu.c \
	avnd_ckpt_enc.c \
	avnd_ckpt_updt.c 

libavnd_ctrl_la_LIBADD = \
	$(top_builddir)/osaf/snmp/miblib/avsv/avnd/libavnd_miblib.la


libavnd_pld_la_CPPFLAGS = $(COMMON_CPPFLAGS)

libavnd_pld_la_LDFLAGS = -static

libavnd_pld_la_SOURCES = $(COMMON_SRC)
