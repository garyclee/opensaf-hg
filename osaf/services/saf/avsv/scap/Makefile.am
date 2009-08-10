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

pkglib_PROGRAMS = ncs_scap

ncs_scap_CPPFLAGS = \
	-DNCS_MAC=1 -DNCS_BAM=1 -DNCS_AVD=1 -DNCS_AVND=1 -DNCS_AVM=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/avsv/include \
	-I$(top_srcdir)/osaf/services/saf/avsv/avm/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/bam/include

ncs_scap_CFLAGS = $(AM_CFLAGS) @HPI_CFLAGS@

ncs_scap_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c

ncs_scap_LDFLAGS = @XERCESC_LIB@

ncs_scap_LDADD = \
	$(top_builddir)/osaf/libs/saf/libSaEvt/libSaEvt.la \
	$(top_builddir)/osaf/libs/common/avsv/libavsv_common.la \
	$(top_builddir)/osaf/libs/agents/infrastructure/maa/libmaa.la \
	$(top_builddir)/osaf/services/saf/avsv/avd/libavd.la \
	$(top_builddir)/osaf/services/saf/avsv/avnd/libavnd_ctrl.la \
	$(top_builddir)/osaf/services/saf/avsv/avm/libavm.la \
	$(top_builddir)/osaf/services/infrastructure/hisv/hpl/libhpl.la \
	$(top_builddir)/osaf/services/infrastructure/bam/libbam.la
