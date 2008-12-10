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

noinst_LTLIBRARIES = libglnd.la

libglnd_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_GLND=1 -DNCS_GLSV_LOG=1 \
	-I $(top_srcdir)/services/glsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/include

libglnd_la_LDFLAGS = -static

libglnd_la_SOURCES = \
	glnd_agent.c \
	glnd_amf.c \
	glnd_api.c \
	glnd_cb.c \
	glnd_ckpt.c \
	glnd_client.c \
	glnd_evt.c \
	glnd_log.c \
	glnd_mds.c \
	glnd_queue.c \
	glnd_res.c \
	glnd_res_req.c \
	glnd_restart.c \
	glnd_shm.c \
	glnd_tmr.c
