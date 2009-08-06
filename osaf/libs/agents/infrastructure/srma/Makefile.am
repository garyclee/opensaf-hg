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

lib_LTLIBRARIES = libsrma.la

libsrma_la_CPPFLAGS = \
	-DNCS_SRMA=1 -DNCS_SRMA_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/srmsv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

libsrma_la_SOURCES = \
	$(top_srcdir)/osaf/services/infrastructure/srmsv/srmsv_edumsg.c \
	$(top_srcdir)/osaf/services/infrastructure/srmsv/srmst/srmst_stat.c \
	$(top_srcdir)/osaf/services/infrastructure/srmsv/srmst/srmst_util.c \
	srma_api.c \
	srma_cbk.c \
	srma_db.c \
	srma_init.c \
	srma_log.c \
	srma_mds.c \
	srma_msg.c \
	srma_proc.c \
	srma_tmr.c

libsrma_la_LDFLAGS = -version-info @OPENSAF_LIB_VERSION@
