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

lib_LTLIBRARIES = libsrma.la

libsrma_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_SRMA=1 -DNCS_SRMA_LOG=1 \
	-I $(top_srcdir)/services/srmsv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/include

libsrma_la_SOURCES =	\
	srma_api.c \
	srma_cbk.c \
	srma_db.c \
	srma_init.c \
	srma_log.c \
	srma_mds.c \
	srma_msg.c \
	srma_proc.c \
	srma_tmr.c \
	$(top_srcdir)/services/srmsv/common/srmsv_edumsg.c \
	$(top_srcdir)/services/srmsv/srmst/srmst_stat.c \
	$(top_srcdir)/services/srmsv/srmst/srmst_util.c

libsrma_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@
