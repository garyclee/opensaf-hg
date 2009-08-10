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

SUBDIRS = scripts

pkglib_PROGRAMS = ncs_pcap

ncs_pcap_CPPFLAGS = \
	-DNCS_AVND=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/avsv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

ncs_pcap_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c

ncs_pcap_LDADD = \
	$(top_builddir)/osaf/libs/saf/libSaEvt/libSaEvt.la \
	$(top_builddir)/osaf/libs/common/avsv/libavsv_common.la \
	$(top_builddir)/osaf/services/saf/avsv/avnd/libavnd_pld.la
