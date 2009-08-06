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

SUBDIRS = include

noinst_LTLIBRARIES = libleap.la

libleap_la_CPPFLAGS = \
	$(AM_CPPFLAGS)

libleap_la_SOURCES = \
	ncsdlib.c \
	hj_regx.c \
	ncs_posix_tmr.c \
	sysf_dl.c \
	sysf_cio.c \
	sysf_def.c \
	patricia.c \
	sysf_fio.c \
	sysf_ipc.c \
	sysf_key.c \
	sysf_lck.c \
	sysf_mem.c \
	hj_cache.c \
	sysf_sem.c \
	sysf_str.c \
	sysf_tmr.c \
	sysf_tsk.c \
	hj_iplib.c \
	hj_mib_edu.c \
	nid_api.c \
	hj_mib_mds.c \
	sysf_file.c \
	nid_ipc.c \
	hj_ipcstats.c \
	sysfpool.c \
	hj_xmtre.c \
	hj_qptrs.c \
	sysfsysm.c \
	hj_dlprm.c \
	hj_dec.c \
	hj_edp.c \
	hj_edu.c \
	hj_enc.c \
	hj_queue.c \
	hj_hdl.c \
	hj_kms.c \
	hj_mib.c \
	hj_tmr.c \
	hjmiblib.c \
	hj_stack.c \
	hj_stree.c \
	hj_ubaid.c \
	hj_ubsar.c \
	ncs_dummy.c \
	hj_hash.c \
	hj_ipv6.c \
	hj_mtree.c \
	sysf_exc_scr.c \
	os_defs.c \
	dl_defs.c

libleap_la_LDFLAGS = -static
