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

SUBDIRS = include logstr

noinst_LTLIBRARIES = libavm.la

libavm_la_CPPFLAGS = \
	-DNCS_AVM=1 -DNCS_AVM_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/avsv/include \
	-I$(top_srcdir)/osaf/services/saf/avsv/avm/include \
	-I$(top_srcdir)/osaf/services/infrastructure/fm/include \
	-I$(top_srcdir)/osaf/snmp/mapi/include \
	-I$(top_srcdir)/osaf/services/infrastructure/hisv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

libavm_la_CFLAGS = $(AM_CFLAGS) @HPI_CFLAGS@

libavm_la_LDFLAGS = -static

libavm_la_SOURCES = \
	avm_avd.c \
	avm_chkop.c \
	avm_ckpt_dec.c \
	avm_ckpt_edu.c \
	avm_ckpt_enc.c \
	avm_ckpt_updt.c \
	avm_db.c \
	avm_dblog.c \
	avm_dhcp_util.c \
	avm_eda.c \
	avm_ent.c \
	avm_evt.c \
	avm_fsm.c \
	avm_fund.c \
	avm_hpi_util.c \
	avm_init.c \
	avm_mab.c \
	avm_mds.c \
	avm_proc.c \
	avm_rda.c \
	avm_fma.c \
	avm_role_evt.c \
	avm_scalar.c \
	avm_tmr.c \
	avm_traps.c \
	avm_upgd_traps.c \
	avm_util.c

libavm_la_LIBADD = \
	$(top_builddir)/osaf/libs/agents/infrastructure/rda/librda.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/fma/libfma.la \
	$(top_builddir)/osaf/snmp/miblib/avsv/avm/libavm_miblib.la
