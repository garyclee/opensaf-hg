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

pkglib_PROGRAMS = ncs_srmnd

ncs_srmnd_CPPFLAGS = \
	-DNCS_SRMND=1 -DNCS_SRMND_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/srmsv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/vds/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

ncs_srmnd_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c \
	$(top_srcdir)/osaf/services/infrastructure/srmsv/srmsv_edumsg.c \
	$(top_srcdir)/osaf/services/infrastructure/srmsv/srmst/srmst_stat.c \
	$(top_srcdir)/osaf/services/infrastructure/srmsv/srmst/srmst_util.c \
	srmnd_amf.c \
	srmnd_db.c \
	srmnd_evt.c \
	srmnd_init.c \
	srmnd_log.c \
	srmnd_mds.c \
	srmnd_mon.c \
	srmnd_msg.c \
	srmnd_proc.c \
	srmnd_stat.c \
	srmnd_tmr.c

ncs_srmnd_LDADD = \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la
