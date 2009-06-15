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

noinst_HEADERS = \
	inc/cmndiagenv.h \
	inc/gl_defs.h \
	inc/mib_mem.h \
	inc/ncs_cache.h \
	inc/ncs_cio.h \
	inc/ncsdlib.h \
	inc/ncs_dlprm.h \
	inc/ncs_dummy.h \
	inc/ncs_edu.h \
	inc/ncsencdec.h \
	inc/ncs_fio.h \
	inc/ncsft.h \
	inc/ncsft_rms.h \
	inc/ncsftrms.h \
	inc/ncs_hash.h \
	inc/ncs_hdl.h \
	inc/ncs_ipcstats.h \
	inc/ncs_ipxs.h \
	inc/ncs_kms.h \
	inc/ncs_leap_version.h \
	inc/ncs_mib.h \
	inc/ncs_mpls.h \
	inc/ncs_opt.h \
	inc/ncs_qptrs.h \
	inc/ncs_regx.h \
	inc/ncs_stack.h \
	inc/ncs_stree.h \
	inc/ncs_sysm.h \
	inc/ncs_tasks.h \
	inc/ncs_ubsar.h \
	inc/ncs_ubsar_pub.h \
	inc/patricia.h \
	inc/sysf_def.h \
	inc/sysf_exc_scr.h \
	inc/sysf_file.h \
	inc/sysf_ipc.h \
	inc/sysf_key.h \
	inc/sysf_lck.h \
	inc/sysf_mem.h \
	inc/sysf_pat.h \
	inc/sysfpool.h \
	inc/sysf_sem.h \
	inc/sysfsysm.h \
	inc/sysf_tmr.h \
	inc/sysf_tsk.h \
	inc/t_suite.h \
	inc/usrbuf.h \
	inc/usr_prof.h

noinst_LTLIBRARIES = libleap.la

libleap_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/services/leap/inc

libleap_la_SOURCES = \
	src/ncsdlib.c \
	src/hj_regx.c \
	src/ncs_posix_tmr.c \
	src/sysf_dl.c \
	src/sysf_cio.c \
	src/sysf_def.c \
	src/patricia.c \
	src/sysf_fio.c \
	src/sysf_ipc.c \
	src/sysf_key.c \
	src/sysf_lck.c \
	src/sysf_mem.c \
	src/hj_cache.c \
	src/sysf_sem.c \
	src/sysf_str.c \
	src/sysf_tmr.c \
	src/sysf_tsk.c \
	src/hj_iplib.c \
	src/hj_mib_edu.c \
	src/nid_api.c \
	src/hj_mib_mds.c \
	src/sysf_file.c \
	src/nid_ipc.c \
	src/hj_ipcstats.c \
	src/sysfpool.c \
	src/hj_xmtre.c \
	src/hj_qptrs.c \
	src/sysfsysm.c \
	src/hj_dlprm.c \
	src/hj_dec.c \
	src/hj_edp.c \
	src/hj_edu.c \
	src/hj_enc.c \
	src/hj_queue.c \
	src/hj_hdl.c \
	src/hj_kms.c \
	src/hj_mib.c \
	src/hj_tmr.c \
	src/hjmiblib.c \
	src/hj_stack.c \
	src/hj_stree.c \
	src/hj_ubaid.c \
	src/hj_ubsar.c \
	src/ncs_dummy.c \
	src/hj_hash.c \
	src/hj_ipv6.c \
	src/hj_mtree.c \
	src/sysf_exc_scr.c \
	src/os_defs.c \
	src/dl_defs.c

libleap_la_LDFLAGS = -static
