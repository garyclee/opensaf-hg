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
	-D_GNU_SOURCE \
	-DUSE_LITTLE_ENDIAN=1 -DNCSL_ENV_INIT_HM=1 \
	-DNCSL_ENV_INIT_MTM=1 -DNCSL_ENV_INIT_TMR=1 \
	$(AM_CPPFLAGS)

libleap_la_SOURCES = \
	ncsdlib.c \
	sysf_def.c \
	patricia.c \
	sysf_ipc.c \
	sysf_mem.c \
	sysf_sem.c \
	sysf_tmr.c \
	sysf_tsk.c \
	nid_api.c \
	nid_ipc.c \
	hj_dec.c \
	hj_edp.c \
	hj_edu.c \
	hj_enc.c \
	hj_queue.c \
	hj_hdl.c \
	hj_tmr.c \
	hj_stack.c \
	hj_ubaid.c \
	sysf_exc_scr.c \
	os_defs.c

libleap_la_LDFLAGS = -static
